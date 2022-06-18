#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <climits>
#include <stdlib.h>

#ifndef MACHINE_H
#define MACHINE_H

const int MEM_SIZE = 1 << 18; // 262144
const int NUM_REGS = 16;
const int EFLAGS_REG = 14;

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
        uint16_t leftOperand;
        uint16_t rightOperand;

        friend std::ostream &operator<<(std::ostream &out, const Decode &dec) {
            std::ostringstream sout;
            sout << "Instruction: " << std::hex << dec.instruction << '\n';
            sout << "Register 1: " << reg[dec.reg1] << '\n';
            sout << "Immediate: 0x" << std::hex << std::setw(2) << std::setfill('0') << dec.immediate << '\n';
            sout << "Left Operand: " << std::dec << dec.leftOperand << '\n';
            sout << "Right Operand: " << std::dec << dec.rightOperand;
            return out << sout.str();
        }
    };

    struct Execute {
        int16_t result;
        friend std::ostream &operator<<(std::ostream &out, const Execute &exe) {
            std::ostringstream sout;
            // sout << "Operation: " << std::hex << exe.result << '\n';
            sout << "Result: " << std::hex << exe.result << '\n';
            return out << sout.str();
        }
    };

    struct Memory {
        int16_t value;
    };

    // Objects
    Fetch fetchObj;
    Decode decodeObj;
    Execute executeObj;
    Memory memoryObj;

    // Memory
    template<typename T>
    T memory_read(int16_t) const;
    template<typename T>
    void memory_write(int16_t, T);
    template<typename T>
    uint8_t next_byte();
    
    // EFLAGS
    void set_carry_flag();
    void set_zero_flag();
    void set_sign_flag();
    void unset_carry_flag();
    void unset_zero_flag();
    void unset_sign_flag();
    bool check_carry_flag();
    bool check_zero_flag();
    bool check_sign_flag();

    public: 
        // CPU
        Machine(char *, int);
        int16_t get_pc() const;
        void set_pc(int16_t);
        int16_t get_xreg(int) const;
        void set_xreg(int, int16_t);
        
        // INSTRUCTION CYCLE
        void fetch();
        void decode();
        void execute();
        void memory_access();
        void write_back();
        Fetch &debug_fetch_out();
        Decode &debug_decode_out();
        Execute &debug_execute_out();
        Memory &debug_memory_out();
};

#endif