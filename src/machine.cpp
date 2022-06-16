#include "machine.h"

// MEMORY
template<typename T>
T Machine::memory_read(int16_t address) const {
    return *reinterpret_cast<T*>(memory + address);
}
template<typename T>
void Machine::memory_write(int16_t address, T value) {
    *reinterpret_cast<T*>(memory + address) = value;
}  
template<typename T>
uint8_t Machine::next_byte() {
    set_pc(get_pc() + 1);
    return *reinterpret_cast<uint8_t*>(memory + get_pc());
}

// CPU
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
    fetchObj.opcode = memory_read<uint8_t>(programCounter);
}
Machine::Fetch &Machine::debug_fetch_out() { 
    return fetchObj; 
}

// DECODE
void Machine::decode() {
    if ( (fetchObj.opcode & 0xfe) == 0xfe) {      // inc rb
        decodeObj.instruction = "inc";
        decodeObj.immediate = next_byte<int8_t>();
        decodeObj.reg1 = decodeObj.immediate - 0xc0;
    }
    else if ( (fetchObj.opcode & 0xb0) == 0xb0) { // mov rb, imm16
        decodeObj.instruction = "mov";
        decodeObj.reg1 = fetchObj.opcode - 0xb0;
        decodeObj.immediate = next_byte<int8_t>();
        if (decodeObj.reg1 > 7){                        // mov rw, rw
            decodeObj.immediate = (decodeObj.immediate << 8) | next_byte<uint8_t>();
        }
    }
    else if ( (fetchObj.opcode & 0x3c) == 0x3c) { // cmp al, 
        decodeObj.instruction = "cmp";
        decodeObj.reg1 = 0;
        decodeObj.immediate = next_byte<int8_t>();      
    }
    else if ( (fetchObj.opcode & 0xeb) == 0xeb) { // jmp rel8
        decodeObj.instruction = "jmp";
        decodeObj.immediate = next_byte<int8_t>();
    }
    else if ( (fetchObj.opcode & 0xcd) == 0xcd) { // int imm8
        decodeObj.instruction = "int";
        decodeObj.reg1 = 16;
        decodeObj.immediate = next_byte<int8_t>();      
    }
    else if ( (fetchObj.opcode & 0x74) == 0x74) { // je imm8
        decodeObj.instruction = "je";
        decodeObj.reg1 = 16;
        decodeObj.immediate = next_byte<int8_t>();      
    }
}
Machine::Decode &Machine::debug_decode_out() { 
    return decodeObj; 
}

// EXECUTE
void Machine::execute() {
    
}
Machine::Execute &Machine::debug_execute_out() { 
    return executeObj; 
}
