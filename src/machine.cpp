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
int8_t Machine::next_byte() {
    set_pc(get_pc() + 1);
    return *reinterpret_cast<uint8_t*>(memory + get_pc());
}
void Machine::byte_to_word(int16_t *immediate){
    *immediate = (*immediate) | (next_byte() << 8);
}

// CPU
Machine::Machine(char *buffer, int size) {
    memory = buffer;
    memorySize = size;
    programCounter = 0;
}
int16_t Machine::get_pc() const {
    return programCounter;
}
void Machine::set_pc(int16_t new_PC) {
    programCounter = new_PC;
}
int16_t Machine::get_xreg(int which) const {
    which &= 0x1f; // Make sure the register number is 0 - 31
    return registers[which];
}
void Machine::set_xreg(int which, int16_t value) {
    which &= 0x1f;
    registers[which] = value;
}
int16_t Machine::reg_to_register(uint16_t* reg){
    switch (*reg){
        case 0:     // al
        case 4:     // ah
        case 8:     // ax
            return 0;
        case 1:     // cl
        case 5:     // ch
        case 9:     // cx
            return 1;
        case 2:     // dl
        case 6:     // dh
        case 10:    // dx
            return 2;
        case 3:     // bl
        case 7:     // bh
        case 11:    // bx
            return 3;
    }
    return 0;
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
    // std::cout << std::dec << '<' << get_pc() << "> ";
    fetchObj.opcode = memory_read<uint8_t>(programCounter);
}
Machine::Fetch &Machine::debug_fetch_out() { 
    return fetchObj; 
}

// DECODE
void Machine::decode() {
    if (fetchObj.opcode == 0x04) {                  // add al, imm8
        decodeObj.instruction = "add";
        decodeObj.rightOperand = decodeObj.immediate = next_byte();
        decodeObj.reg = 0; // AL
        decodeObj.regi = 0; // AX - registers[0]
        decodeObj.leftOperand = (get_xreg(0) & 0xff); 
    }
    else if (fetchObj.opcode == 0x81) {             // add rw, imm16
        decodeObj.instruction = "add";
        decodeObj.reg = next_byte() - 0xc0 + 8;
        decodeObj.regi = reg_to_register(&decodeObj.reg);
        decodeObj.immediate = next_byte();
        byte_to_word(&decodeObj.immediate);
        decodeObj.leftOperand = get_xreg(decodeObj.regi);
        decodeObj.rightOperand = decodeObj.immediate;
    }
    else if (fetchObj.opcode == 0x3c) {             // cmp al, imm16
        decodeObj.instruction = "cmp";
        decodeObj.reg = 0;
        decodeObj.regi = reg_to_register(&decodeObj.reg);
        decodeObj.immediate = next_byte();
        decodeObj.leftOperand = (get_xreg(decodeObj.regi) & 0x00ff);   
        decodeObj.rightOperand = decodeObj.immediate;
    }
    else if (fetchObj.opcode == 0xfe) {             // inc r8
        decodeObj.instruction = "inc";
        decodeObj.immediate = next_byte();
        decodeObj.reg = decodeObj.immediate - 0xc0;
        decodeObj.regi = reg_to_register(&decodeObj.reg);
        switch (decodeObj.reg){
            case 0x00: // Lower-Byte Registers
            case 0x01:
            case 0x02:
            case 0x03:
                decodeObj.leftOperand = get_xreg(decodeObj.regi) & 0xff; 
                break;
            case 0x04: // Upper-Byte Registers
            case 0x05:
            case 0x06:
            case 0x07:
                decodeObj.leftOperand = (get_xreg(decodeObj.regi) >> 8) & 0xff; 
                break;
        }
        decodeObj.rightOperand = 1;
    }
    else if (fetchObj.opcode== 0xcd) {              // int imm8
        decodeObj.instruction = "int";
        decodeObj.reg = 16;
        decodeObj.immediate = next_byte();      
    }
    else if (fetchObj.opcode == 0xeb) {             // jmp rel8
        decodeObj.instruction = "jmp";
        decodeObj.immediate = next_byte();
        decodeObj.leftOperand = get_pc();
        decodeObj.rightOperand = decodeObj.immediate;
    }
    else if (fetchObj.opcode == 0x74) {             // je imm8
        decodeObj.instruction = "je";
        decodeObj.reg = 16;
        decodeObj.immediate = next_byte();
        decodeObj.leftOperand = get_pc(); 
        decodeObj.rightOperand = decodeObj.immediate;
    }
    else if (fetchObj.opcode == 0x8a){
        decodeObj.instruction = "mov";              // mov rb, [m8]
        decodeObj.immediate = next_byte();
        switch (decodeObj.immediate % 8){
            // [bx]
            case 7:
                decodeObj.reg = decodeObj.immediate % 7; 
                decodeObj.immediate = memory_read<uint8_t>(get_xreg(3));
                break;
            // [si]
            case 4:
                decodeObj.reg = (decodeObj.immediate - 4) / 4 - 1;
                break;
        }
        decodeObj.regi = reg_to_register(&decodeObj.reg);
        decodeObj.leftOperand = get_xreg(decodeObj.regi) & 0x00ff;
        decodeObj.rightOperand = decodeObj.immediate;
    }
    else if ( (fetchObj.opcode & 0x40) == 0x40) {   // inc rw
        decodeObj.instruction = "inc";
        decodeObj.immediate = fetchObj.opcode - 0x40;
        fetchObj.opcode &= 0x40;
        decodeObj.regi = decodeObj.immediate;
        decodeObj.reg = decodeObj.regi + 8;
        decodeObj.leftOperand = get_xreg(decodeObj.regi);
        decodeObj.rightOperand = 1;
    }
    else if ((fetchObj.opcode & 0xb0) == 0xb0) {   // mov rb, imm16
        decodeObj.reg = fetchObj.opcode - 0xb0;     
        fetchObj.opcode &= 0xb0;
        decodeObj.instruction = "mov";
        decodeObj.immediate = next_byte();
        if (decodeObj.reg > 7){                    // mov rw, rw
            byte_to_word(&decodeObj.immediate);
        }
        decodeObj.regi = reg_to_register(&decodeObj.reg);
        decodeObj.leftOperand = (get_xreg(decodeObj.regi) >> 8);
        decodeObj.rightOperand = decodeObj.immediate;
    }
}
Machine::Decode &Machine::debug_decode_out() { 
    return decodeObj; 
}

// EXECUTE
void Machine::execute() {
    switch (fetchObj.opcode){
        case 0x81:      // add rw, imm16
            executeObj.result = decodeObj.leftOperand + decodeObj.rightOperand;
            break;
        case 0xfe:      // inc rb
            executeObj.result = decodeObj.leftOperand + decodeObj.rightOperand;
            if (executeObj.result > SHRT_MAX) {
                set_carry_flag();
            }
            break;
        case 0x40:
            executeObj.result = decodeObj.leftOperand + decodeObj.rightOperand;
            if (executeObj.result > SHRT_MAX) {
                set_carry_flag();
            }
            break;
        case 0x3c:      // cmp al, imm16
            executeObj.result = decodeObj.leftOperand - decodeObj.rightOperand;
            break;
        case 0xeb:      // jmp rel8
            executeObj.result = decodeObj.leftOperand + decodeObj.rightOperand;
            break;
        case 0x74:      // je imm8
            executeObj.result = decodeObj.leftOperand + decodeObj.rightOperand;
            break;
        case 0xb0:      // mov rb, imm8
            executeObj.result = decodeObj.immediate;
            break;
        case 0x8a:
            executeObj.result = decodeObj.immediate;
            break;
    }
}
Machine::Execute &Machine::debug_execute_out() { 
    return executeObj; 
}

// // MEMORY
// void Machine::memory_access() {
// }
// Machine::Memory &Machine::debug_memory_out() { 
//     return memoryObj; 
// }

// WRITEBACK
void Machine::write_back() {
    switch (fetchObj.opcode) {
        case 0x81:      // add rw, imm16
            set_xreg(decodeObj.reg, executeObj.result);
            break;
        case 0x3c:      // cmp al, imm16
            if (executeObj.result == 0) set_zero_flag();
            else if (executeObj.result < 0) set_sign_flag();
            else set_carry_flag();
            break;
        case 0xfe:      // inc rb
            set_xreg(decodeObj.reg, executeObj.result); 
            break;
        case 0x40:      // inc rw
            set_xreg(decodeObj.regi, executeObj.result);
            break;
        case 0xcd:      // int imm8
            if (decodeObj.immediate == 0x10) { 
                if ( (get_xreg(decodeObj.regi) >> 8) == 0x0e ) {
                    putchar( (get_xreg(decodeObj.regi) & 0xff) );
                }
            }
            break;
        case 0xeb:      // jmp rel8
            set_pc(executeObj.result);
            break;
        case 0x74:      // je imm8
            if (check_zero_flag()) set_pc(executeObj.result);
            break;
        case 0x8a:      // mov rb, m8
            set_xreg(decodeObj.regi, 
                            (get_xreg(decodeObj.regi) & 0xff00)
                            | decodeObj.immediate);
            break;
        case 0xb0:      // mov rb, imm8
            if (decodeObj.reg > 7){ // 16-Bit Immediate
                set_xreg(decodeObj.regi, executeObj.result);
            }
            else{
                switch (decodeObj.reg){
                case 0: // Lower 8-Bit Registers
                case 1:
                case 2:
                case 3:
                    set_xreg(decodeObj.regi, 
                            (get_xreg(decodeObj.regi) & 0xff00)
                            | executeObj.result);
                    break;
                case 4: // Upper 8-Bit Registers
                case 5:
                case 6:
                case 7:
                    set_xreg(decodeObj.regi, 
                            (get_xreg(decodeObj.regi) & 0x00ff)
                            | (executeObj.result << 8));
                    break;
                }
            }
            break;
    }
}