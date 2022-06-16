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

const std::string reg[] {
    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", // 8-Bit Registers
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di", // 16-Bit Registers
    "NO REG"
};

class Machine {
    char* memory;           // Memory
    int memorySize;         // Size of Memory (Should be MEM_SIZE)
    int16_t programCounter; // Program Counter
    int16_t registers[NUM_REGS]; // Registers:
                                 // 0 - AX
                                 // 1 - CX
                                 // 2 - DX
                                 // 3 - BX 
                                 // 4 - SP
                                 // 5 - BP
                                 // 6 - SI
                                 // 7 - DI
                                 // 8 - SS
                                 // 9 - CS
                                 // 10 - DS
                                 // 11 - ES
                                 // 12 - FS
                                 // 13 - GS
                                 // 14 - EFLAGS
                                 // 15 - IP


    // INSTRUCTION CYCLE 

    struct Fetch {
        uint16_t opcode;

        friend std::ostream& operator<<(std::ostream &out, const Fetch &fo){
            std::ostringstream sout;
            sout << "0x" << std::hex << std::setw(2) << std::setfill('0') << fo.opcode;
            return out << sout.str();
        }
    };

    struct Decode {
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

    struct Execute {
        std::string operation;
    };

    // Objects
    Fetch fetchObj;
    Decode decodeObj;
    Execute executeObj;

    // Memory
    template<typename T>
    T memory_read(int16_t) const;
    template<typename T>
    void memory_write(int16_t, T);
    template<typename T>
    uint8_t next_byte();

    public: 
        // CPU
        Machine(char *, int);
        int64_t get_pc() const;
        void set_pc(int16_t);
        int64_t get_xreg(int) const;
        void set_xreg(int, int16_t);
        
        // INSTRUCTION CYCLE
        void fetch();
        void decode();
        void execute();
        Fetch &debug_fetch_out();
        Decode &debug_decode_out();
        Execute &debug_execute_out();
};

#endif