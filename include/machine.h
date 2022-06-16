#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <unordered_map>

#ifndef MACHINE_H
#define MACHINE_H

const int MEM_SIZE = 1 << 18; // 262144
const int NUM_REGS = 16;

const std::string reg[] {
    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", // 8-Bit Registers
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di"  // 16-Bit Registers
};

class Machine {
    char* memory; // Memory
    int memorySize; // Size of Memory (Should be MEM_SIZE)
    int16_t programCounter; // Program Counter
    int16_t registers[NUM_REGS];

    // INSTRUCTION CYCLE 

    struct Fetch{
        uint16_t xInstruction;

        friend std::ostream& operator<<(std::ostream &out, const Fetch &fo){
            std::ostringstream sout;
            sout << "0x" << std::hex << std::setw(2) << std::setfill('0') << fo.xInstruction;
            return out << sout.str();
        }
    };

    struct Decode{
        std::string instruction;
        int16_t immediate;
        uint16_t reg1;
        uint16_t reg2;

        friend std::ostream &operator<<(std::ostream &out, const Decode &dec) {
            std::ostringstream sout;
            sout << "Instruction: " << std::hex << dec.instruction << '\n';
            sout << "Register 1: " << reg[dec.reg1] << '\n';
            sout << "Immediate: 0x" << std::hex << std::setw(2) << std::setfill('0') << dec.immediate << '\n';
            return out << sout.str();
        }
    };

    Fetch fetchObj;
    Decode decodeObj;

    template<typename T>
    T memory_read(int16_t) const;
    template<typename T>
    void memory_write(int16_t, T);
    template<typename T>
    T next_byte();

    public: 
        Machine(char *, int);
        int64_t get_pc() const;
        void set_pc(int16_t);
        int64_t get_xreg(int) const;
        void set_xreg(int, int16_t);

        void fetch();
        void decode();
        Fetch &debug_fetch_out();
        Decode &debug_decode_out();
};

#endif