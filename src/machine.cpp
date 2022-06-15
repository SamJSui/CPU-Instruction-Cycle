#include "machine.h"

// Read from the internal memory
// Usage:
// int myintval = memory_read<int>(0); // Read the first 4 bytes
// char mycharval = memory_read<char>(8); // Read byte index 8
template<typename T>
T Machine::memory_read(int64_t address) const {
    return *reinterpret_cast<T*>(memory + address);
}
// Write to the internal memory
// Usage:
// memory_write<int>(0, 0xdeadbeef); // Set bytes 0, 1, 2, 3 to 0xdeadbeef
// memory_write<char>(8, 0xff);      // Set byte index 8 to 0xff
template<typename T>
void Machine::memory_write(int64_t address, T value) {
    *reinterpret_cast<T*>(memory + address) = value;
}  

Machine::Machine(char *buffer, int size){
    memory = buffer;
    memorySize = size;
    programCounter = 0;
}
int64_t Machine::get_pc() const {
    return programCounter;
}
void Machine::set_pc(int64_t new_PC) {
    programCounter = new_PC;
}
int64_t Machine::get_xreg(int reg) const {
    reg &= 0x1f; // Make sure the register number is 0 - 31
    return registers[reg];
}
void Machine::set_xreg(int reg, int64_t value) {
    reg &= 0x1f;
    registers[reg] = value;
}

// INSTRUCTION CYCLE

// FETCH
void Machine::fetch() {
    fetchObj.instruction = memory_read<uint8_t>(programCounter);
}
Machine::Fetch &Machine::debug_fetch_out() { 
    return fetchObj; 
}

// DECODE

void Machine::decode() {
    
}
Machine::Decode &Machine::debug_decode_out() { 
    return decodeObj; 
}
