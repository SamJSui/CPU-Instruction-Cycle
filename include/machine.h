#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

#ifndef MACHINE_H
#define MACHINE_H

const int MEM_SIZE = 1 << 18; // 262144
const int NUM_REGS = 16;

class Machine {
    char* memory; // Memory
    int memorySize; // Size of Memory (Should be MEM_SIZE)
    int16_t programCounter; // Program Counter
    int16_t registers[NUM_REGS];

    // INSTRUCTION CYCLE 

    struct Fetch{
        uint32_t instruction;
        friend std::ostream& operator<<(std::ostream &out, const Fetch &fo){
            std::ostringstream sout;
            sout << "0x" << std::hex << std::setw(2) << std::setfill('0') << fo.instruction;
            return out << sout.str();
        }
    };

    struct Decode{
        uint32_t opcode;
        int32_t immediate;
        uint8_t reg1;
        uint8_t reg2;
        friend std::ostream &operator<<(std::ostream &out, const Decode &dec) {
            std::ostringstream sout;
            // sout << "Operation: ";
            // switch (dec.op) {

            // }
            // sout << '\n';
            // sout << "RD       : " << (uint32_t)dec.rd << '\n';
            // sout << "funct3   : " << (uint32_t)dec.funct3 << '\n';
            // sout << "funct7   : " << (uint32_t)dec.funct7 << '\n';
            // sout << "offset   : " << dec.offset << '\n';
            // sout << "left     : " << dec.left_val << '\n';
            // sout << "right    : " << dec.right_val;
            return out << sout.str();
        }
    };

    Fetch fetchObj;
    Decode decodeObj;

    template<typename T>
    T memory_read(int64_t address) const;
    template<typename T>
    void memory_write(int64_t address, T value);

    public: 
        Machine(char *, int);
        int64_t get_pc() const;
        void set_pc(int64_t);
        int64_t get_xreg(int) const;
        void set_xreg(int, int64_t);

        void fetch();
        void decode();
        Fetch &debug_fetch_out();
        Decode &debug_decode_out();
};

#endif