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
    int64_t programCounter; // Program Counter
    int64_t registers[NUM_REGS];

    struct Fetch{
        uint32_t instruction;
        friend std::ostream& operator<<(std::ostream &out, const Fetch &fo){
            std::ostringstream sout;
            sout << "0x" << std::hex << std::setfill('0') << std::right << std::setw(8) << fo.instruction;
            return out << sout.str();
        }
    };

    Fetch fetchObj;

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
        Fetch &debug_fetch_out();
};

#endif