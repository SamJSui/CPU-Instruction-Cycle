#include "machine.h"

// Read from the internal memory
// Usage:
// int myintval = memory_read<int>(0); // Read the first 4 bytes
// char mycharval = memory_read<char>(8); // Read byte index 8
template<typename T>
T Machine::memory_read(int16_t address) const {
    return *reinterpret_cast<T*>(memory + address);
}
// Write to the internal memory
// Usage:
// memory_write<int>(0, 0xdeadbeef); // Set bytes 0, 1, 2, 3 to 0xdeadbeef
// memory_write<char>(8, 0xff);      // Set byte index 8 to 0xff
template<typename T>
void Machine::memory_write(int16_t address, T value) {
    *reinterpret_cast<T*>(memory + address) = value;
}  
template<typename T>
T Machine::next_byte() {
    set_pc(get_pc() + 1);
    return *reinterpret_cast<T*>(memory + get_pc());
}

Machine::Machine(char *buffer, int size) {
    memory = buffer;
    memorySize = size;
    programCounter = 0;
}
int64_t Machine::get_pc() const {
    return programCounter;
}
void Machine::set_pc(int16_t new_PC) {
    programCounter = new_PC;
}
int64_t Machine::get_xreg(int which) const {
    which &= 0x1f; // Make sure the register number is 0 - 31
    return registers[which];
}
void Machine::set_xreg(int which, int16_t value) {
    which &= 0x1f;
    registers[which] = value;
}

// INSTRUCTION CYCLE

// FETCH
void Machine::fetch() {
    fetchObj.xInstruction = memory_read<uint8_t>(programCounter);
}
Machine::Fetch &Machine::debug_fetch_out() { 
    return fetchObj; 
}

// DECODE

void Machine::decode() {
    if ( (fetchObj.xInstruction & 0xb0) == 0xb0 ){ // mov rh, i16
        decodeObj.instruction = "mov";
        decodeObj.reg1 = fetchObj.xInstruction - 0xb0;
        decodeObj.immediate = next_byte<uint8_t>();
        if (decodeObj.reg1 > 7){ // mov rw, rw
            decodeObj.immediate = (decodeObj.immediate << 8) | next_byte<uint8_t>();
        }
    }
}
Machine::Decode &Machine::debug_decode_out() { 
    return decodeObj; 
}
