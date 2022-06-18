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

// EFLAGS
void Machine::set_carry_flag(){
    registers[EFLAGS_REG] |= (1 << 0);
}
void Machine::set_zero_flag(){
    registers[EFLAGS_REG] |= (1 << 6);
}
void Machine::set_sign_flag(){
    registers[EFLAGS_REG] |= (1 << 7);
}
void Machine::unset_carry_flag(){
    registers[EFLAGS_REG] &= ~(1 << 0);
}
void Machine::unset_zero_flag(){
    registers[EFLAGS_REG] &= ~(1 << 6);
}
void Machine::unset_sign_flag(){
    registers[EFLAGS_REG] &= ~(1 << 7);
}
bool Machine::check_carry_flag(){
    return (registers[EFLAGS_REG] >> 0);
}
bool Machine::check_zero_flag(){
    return (registers[EFLAGS_REG] >> 6);
}
bool Machine::check_sign_flag(){
    return (registers[EFLAGS_REG] >> 7);
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
    if ( (fetchObj.opcode & 0xfe) == 0xfe) {            // inc r8
        fetchObj.opcode &= 0xfe;
        decodeObj.instruction = "inc";
        decodeObj.immediate = next_byte<int8_t>();
        decodeObj.reg1 = decodeObj.immediate - 0xc0;
        switch (decodeObj.reg1){
            case 0x00: // Lower-Byte Registers
            case 0x01:
            case 0x02:
            case 0x03:
                decodeObj.leftOperand = registers[decodeObj.reg1] & 0xff; 
                break;
            case 0x04: // Upper-Byte Registers
            case 0x05:
            case 0x06:
            case 0x07:
                decodeObj.leftOperand = (registers[decodeObj.reg1] >> 8) & 0xff; 
                break;
        }
        decodeObj.rightOperand = 1;
    }
    else if ( (fetchObj.opcode & 0x40) == 0x40) {            // inc r16
        decodeObj.instruction = "inc";
        decodeObj.immediate = fetchObj.opcode - 0x40;
        fetchObj.opcode &= 0x40;
        decodeObj.reg1 = decodeObj.immediate;
        decodeObj.leftOperand = registers[decodeObj.reg1];
        decodeObj.rightOperand = 1;
    }
    else if ( (fetchObj.opcode & 0xb0) == 0xb0) {           // mov rb, imm16
        decodeObj.reg1 = fetchObj.opcode - 0xb0;
        fetchObj.opcode &= 0xb0;
        decodeObj.instruction = "mov";
        decodeObj.immediate = next_byte<int8_t>();
        if (decodeObj.reg1 > 7){                            // mov rw, rw
            decodeObj.immediate = ((decodeObj.immediate << 8) | next_byte<uint8_t>());
        }
    }
    else if ( (fetchObj.opcode & 0x3c) == 0x3c) {           // cmp al, imm16
        fetchObj.opcode &= 0x3c;
        decodeObj.instruction = "cmp";
        decodeObj.reg1 = 0;
        decodeObj.immediate = next_byte<int8_t>();
        decodeObj.leftOperand = (registers[decodeObj.reg1] & 0xff); 
        decodeObj.rightOperand = decodeObj.immediate;     
    }
    else if ( (fetchObj.opcode & 0xeb) == 0xeb) {           // jmp rel8
        fetchObj.opcode &= 0xeb;
        decodeObj.instruction = "jmp";
        decodeObj.immediate = next_byte<int8_t>();
        decodeObj.leftOperand = get_pc();
        decodeObj.rightOperand = decodeObj.immediate;
    }
    else if ( (fetchObj.opcode & 0xcd) == 0xcd) {           // int imm8
        fetchObj.opcode &= 0xcd;
        decodeObj.instruction = "int";
        decodeObj.reg1 = 16;
        decodeObj.immediate = next_byte<int8_t>();      
    }
    else if ( (fetchObj.opcode & 0x74) == 0x74) {           // je imm8
        fetchObj.opcode &= 0x74;
        decodeObj.instruction = "je";
        decodeObj.reg1 = 16;
        decodeObj.immediate = next_byte<int8_t>();
        decodeObj.leftOperand = get_pc(); 
        decodeObj.rightOperand = decodeObj.immediate;
    }
    else if ( (fetchObj.opcode & 0x81) == 0x81) {           // add rw, imm16
        fetchObj.opcode &= 0x81;
        decodeObj.instruction = "add";
        decodeObj.reg1 = next_byte<int8_t>() - 0xc0;
        decodeObj.immediate = next_byte<int8_t>();
        decodeObj.immediate = ((decodeObj.immediate << 8) | next_byte<uint8_t>());
        switch (decodeObj.reg1) {
            case 0:
            case 4:
                decodeObj.reg1 = 8;
                decodeObj.leftOperand = registers[8];
                break;
            case 1:
            case 5:
                decodeObj.reg1 = 9;
                decodeObj.leftOperand = registers[9];
                break;
            case 2:
            case 6:
                decodeObj.reg1 = 10;
                decodeObj.leftOperand = registers[10];
                break;
            case 3:
            case 7:
                decodeObj.reg1 = 11;
                decodeObj.leftOperand = registers[11];
                break;
        } 
        decodeObj.rightOperand = decodeObj.immediate;
    }
    else if ( (fetchObj.opcode & 0x04) == 0x04) {           // add al, imm8
        fetchObj.opcode &= 0x04;
        decodeObj.instruction = "add";
        decodeObj.immediate = next_byte<int8_t>();
        decodeObj.reg1 = 0;
        decodeObj.leftOperand = (registers[decodeObj.reg1] & 0xff); 
        decodeObj.rightOperand = decodeObj.immediate;
    }
}
Machine::Decode &Machine::debug_decode_out() { 
    return decodeObj; 
}

// EXECUTE
void Machine::execute() {
    switch (fetchObj.opcode){
        case 0xfe:      // inc rb
            executeObj.result = decodeObj.leftOperand + decodeObj.rightOperand;
            if (executeObj.result > SHRT_MAX) {
                set_carry_flag();
            }
            break;
        case 0xb0:      // mov rb
            break;
        case 0x3c:      // cmp al, imm16
            executeObj.result = decodeObj.leftOperand - decodeObj.rightOperand;
            if (executeObj.result == 0) set_zero_flag();
            else if (executeObj.result < 0) set_sign_flag();
            else set_carry_flag();
            break;
        case 0xeb:      // jmp rel8
            executeObj.result = decodeObj.leftOperand - decodeObj.rightOperand;
            break;
        case 0xcd:      // int imm8
            break;
        case 0x74:      // je imm8
            if (check_zero_flag()) executeObj.result = decodeObj.leftOperand - decodeObj.rightOperand;
            break;
        case 0x81:      // add rw, imm16
            executeObj.result = decodeObj.leftOperand + decodeObj.rightOperand;
    }
}
Machine::Execute &Machine::debug_execute_out() { 
    return executeObj; 
}

// MEMORY
void Machine::memory_access() {

}
Machine::Memory &Machine::debug_memory_out() { 
    return memoryObj; 
}

// WRITEBACK
void Machine::write_back() {

}