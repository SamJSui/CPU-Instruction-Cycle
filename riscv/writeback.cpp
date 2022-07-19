#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

const int MEM_SIZE = 1 << 18; // CONST GLOBALS
const int NUM_REGS = 32;

int64_t sign_extend(int64_t value, int8_t index);

enum OpcodeCategories {
   LOAD, STORE, BRANCH, JALR,
   JAL, OP_IMM, OP, AUIPC, LUI,
   OP_IMM_32, OP_32, SYSTEM,
   UNIMPL
};

const OpcodeCategories OPCODE_MAP[4][8] = {
   // First row (inst[6:5] = 0b00)
   { LOAD, UNIMPL, UNIMPL, UNIMPL, OP_IMM, AUIPC, OP_IMM_32, UNIMPL }, 
   // Second row (inst[6:5] = 0b01)
   { STORE, UNIMPL, UNIMPL, UNIMPL, OP, LUI, OP_32, UNIMPL },
   // Third row (inst[6:5] = 0b10)
   { UNIMPL, UNIMPL, UNIMPL, UNIMPL, UNIMPL, UNIMPL, UNIMPL, UNIMPL },
   // Fourth row (inst[6:5] = 0b11)
   { BRANCH, JALR, UNIMPL, JAL, SYSTEM, UNIMPL, UNIMPL, UNIMPL }
};

enum AluCommands {
   ALU_ADD,
   ALU_SUB,
   ALU_MUL,
   ALU_DIV,
   ALU_REM,
   ALU_SLL,
   ALU_SRL,
   ALU_SRA,
   ALU_AND,
   ALU_OR,
   ALU_XOR,
   ALU_NOT
};

class Machine {

    // Structs
    struct FetchOut {
        uint32_t instruction;

        // The code below allows us to cout a FetchOut structure.
        // We can use this to debug our code.
        // FetchOut fo = { 0xdeadbeef };
        // cout << fo << '\n';
        friend std::ostream &operator<<(std::ostream &out, const FetchOut &fo) {
            std::ostringstream sout;
            sout << "0x" << std::hex << std::setfill('0') << std::right << std::setw(8) << fo.instruction;
            return out << sout.str();
        }
    };
    struct DecodeOut {
        OpcodeCategories op;
        uint8_t rd;
        uint8_t funct3;
        uint8_t funct7;
        int64_t offset;    // Offsets for BRANCH and STORE
        int64_t left_val;  // typically the value of rs1
        int64_t right_val; // typically the value of rs2 or immediate

        friend ostream &operator<<(ostream &out, const DecodeOut &dec) {
            ostringstream sout;
            sout << "Operation: ";
            switch (dec.op) {
                case LUI:
                    sout << "LUI";
                    break;
                case AUIPC:
                    sout << "AUIPC";
                    break;
                case LOAD:
                    sout << "LOAD";
                    break;
                case STORE:
                    sout << "STORE";
                    break;
                case OP_IMM:
                    sout << "OPIMM";
                    break;
                case OP_IMM_32:
                    sout << "OPIMM32";
                    break;
                case OP:
                    sout << "OP";
                    break;
                case OP_32:
                    sout << "OP32";
                    break;
                case BRANCH:
                    sout << "BRANCH";
                    break;
                case JALR:
                    sout << "JALR";
                    break;
                case JAL:
                    sout << "JAL";
                    break;
                case SYSTEM:
                    sout << "SYSTEM";
                    break;
                case UNIMPL:
                    sout << "NOT-IMPLEMENTED";
                    break;
            }
            sout << '\n';
            sout << "RD       : " << (uint32_t)dec.rd << '\n';
            sout << "funct3   : " << (uint32_t)dec.funct3 << '\n';
            sout << "funct7   : " << (uint32_t)dec.funct7 << '\n';
            sout << "offset   : " << dec.offset << '\n';
            sout << "left     : " << dec.left_val << '\n';
            sout << "right    : " << dec.right_val;
            return out << sout.str();
        }
    };
    struct ExecuteOut {
        int64_t result;
        uint8_t n, z, c, v;
        friend ostream &operator<<(ostream &out, const ExecuteOut &eo) {
            ostringstream sout;
            sout << "Result: " << eo.result << " [NZCV]: " 
                << (uint32_t)eo.n 
                << (uint32_t)eo.z 
                << (uint32_t)eo.c
                << (uint32_t)eo.v;
            return out << sout.str();
        }
    };
    struct MemoryOut {
        int64_t value;

        friend ostream &operator<<(ostream &out, const MemoryOut &mo) {
            ostringstream sout;
            sout << "0x" << hex << right << setfill('0') << setw(16) << mo.value;
            return out << sout.str();
        }
    };    
    
    char *mMemory;   // The memory.
    int mMemorySize; // The size of the memory (should be MEM_SIZE)
    int64_t mPC;     // The program counter
    int64_t mRegs[NUM_REGS]; // The register file

    // Objects
    FetchOut mFO;   
    DecodeOut mDO;
    ExecuteOut mEO;
    MemoryOut mMO;

    // Read from the internal memory
    // Usage:
    // int myintval = memory_read<int>(0); // Read the first 4 bytes
    // char mycharval = memory_read<char>(8); // Read byte index 8
    template<typename T>
    T memory_read(int64_t address) const {
        return *reinterpret_cast<T*>(mMemory + address);
    }
    // Write to the internal memory
    // Usage:
    // memory_write<int>(0, 0xdeadbeef); // Set bytes 0, 1, 2, 3 to 0xdeadbeef
    // memory_write<char>(8, 0xff);      // Set byte index 8 to 0xff
    template<typename T>
    void memory_write(int64_t address, T value) {
        *reinterpret_cast<T*>(mMemory + address) = value;
    }

    // Decode
    void decode_b() {
        mDO.rd        = (mFO.instruction >> 7) & 0x1f;
        mDO.funct3    = (mFO.instruction >> 12) & 7;
        mDO.left_val  = get_xreg((mFO.instruction >> 15)); // get_xreg truncates for us
        mDO.right_val = get_xreg((mFO.instruction >> 20));
        mDO.offset    = sign_extend((((mFO.instruction >> 31) & 1) << 12) |
                                    (((mFO.instruction >> 25) & 0x3f) << 5) |
                                    (((mFO.instruction >> 8) & 0xf) << 1) | 
                                    (((mFO.instruction >> 7) & 1) << 11), 12);
    }
    void decode_r() {
        mDO.rd        = (mFO.instruction >> 7) & 0x1f;
        mDO.funct3    = (mFO.instruction >> 12) & 7;
        mDO.left_val  = get_xreg(mFO.instruction >> 15); // get_xreg truncates for us
        mDO.right_val = get_xreg(mFO.instruction >> 20);
        mDO.funct7    = (mFO.instruction >> 25) & 0x7f;
    }
    void decode_i() {
        mDO.rd        = (mFO.instruction >> 7) & 0x1f;
        mDO.funct3    = (mFO.instruction >> 12) & 7;
        mDO.left_val  = get_xreg(mFO.instruction >> 15); // RS1
        mDO.right_val = sign_extend((mFO.instruction >> 20), 11); // Immediate
    }
    void decode_j() {
        mDO.rd        = (mFO.instruction >> 7) & 0x1f;
        // imm[20|10:1|11|19:12]
        mDO.right_val = sign_extend((((mFO.instruction >> 31) & 1) << 20 | // imm[20]
                                    (((mFO.instruction >> 21) & 0x3ff) << 1) | // imm[10:1]
                                    (((mFO.instruction >> 20) & 1) << 11) | // imm[11]
                                    (((mFO.instruction >> 12) & 0xff) << 12)), 20); // imm[19:12]
    }
    void decode_u() {
        mDO.rd        = (mFO.instruction >> 7) & 0x1f;
        mDO.right_val = sign_extend((((mFO.instruction >> 12) & 0xfffff) << 12), 31); 
    }
    void decode_s() {
        mDO.funct3    = (mFO.instruction >> 12) & 7;
        mDO.left_val  = get_xreg(mFO.instruction >> 15); // RS1
        mDO.right_val = get_xreg(mFO.instruction >> 20); // RS2
        mDO.offset    = sign_extend(((mFO.instruction >> 7) & 0x1f) | // Offset
                                   (((mFO.instruction >> 25) & 0x7f) << 5), 11);
    }

    // ALU Operations
    ExecuteOut alu(AluCommands cmd, int64_t left, int64_t right) {
        ExecuteOut ret;
        switch (cmd) {
            case ALU_ADD:
                ret.result = left + right;
            break;
            case ALU_SUB:
                ret.result = left - right;
            break;
            case ALU_MUL:
                ret.result = left * right;
            break;
            case ALU_DIV:
                ret.result = left / right;
            break;
            case ALU_REM:
                ret.result = left % right;
            break;
            case ALU_SRL:
                ret.result = static_cast<uint64_t>(left) >> right;
            break;
            // Finish the commands here.
            case ALU_SLL:
                ret.result = static_cast<uint64_t>(left) << right;
                break;
            case ALU_SRA:
                ret.result = static_cast<int64_t>(left) >> right;
                break;
            case ALU_AND:
                ret.result = left & right;
                break;
            case ALU_OR:
                ret.result = left | right;
                break;
            case ALU_XOR:
                ret.result = left ^ right;
                break;
            case ALU_NOT:
                ret.result = ~left;
                break;
        }
        // Now that we have the result, determine the flags.
        uint8_t sign_left = (left >> 63) & 1;
        uint8_t sign_right = (right >> 63) & 1;
        uint8_t sign_result = (ret.result >> 63) & 1;
        ret.z = !ret.result;
        ret.n = sign_result;
        ret.v = (~sign_left & ~sign_right & sign_result) |
                    (sign_left & sign_right & ~sign_result);
        ret.c = (ret.result > left) || (ret.result > right);
        return ret;
    }

public:
    Machine(char *mem, int size) {
        mMemory = mem;
        mMemorySize = size;
        mPC = 0;
        set_xreg(2, mMemorySize);
    }

    int64_t get_pc() const {
        return mPC;
    }
    void set_pc(int64_t to) {
        mPC = to;
    }

    int64_t get_xreg(int reg) const {
        reg &= 0x1f; // Make sure the register number is 0 - 31
        return mRegs[reg];
    }
    void set_xreg(int reg, int64_t value) {
        reg &= 0x1f;
        mRegs[reg] = value;
    }

    void fetch() {
        mFO.instruction = memory_read<uint32_t>(mPC);
    }
    void decode() {
        uint8_t opcode_map_row = (mFO.instruction >> 5) & 3;
        uint8_t opcode_map_col = (mFO.instruction >> 2) & 7;
        uint8_t inst_size      = mFO.instruction & 3;
        if (inst_size != 3) {
            cerr << "[DECODE] Invalid instruction (not a 32-bit instruction).\n";
            return;
        }
        mDO.op = OPCODE_MAP[opcode_map_row][opcode_map_col];
        // Decode the rest of mDO based on the instruction type
        switch (mDO.op) {
            case LOAD:
            case JALR:
            case OP_IMM:
            case OP_IMM_32:
            case SYSTEM:
                decode_i();
                break;
            case STORE:
                decode_s();
                break;
            case BRANCH:
                decode_b();
                break;
            case JAL:
                decode_j();
                break;
            case AUIPC:
            case LUI:
                decode_u();
                break;
            case OP:
            case OP_32:
                decode_r();
                break;
            default:
                cerr << "Invalid op type: " << mDO.op << '\n';
                break;
        }
    }
    void execute() {
        AluCommands cmd;
        // Most instructions will follow left/right
        // but some won't, so we need these:
        int64_t op_left = mDO.left_val;
        int64_t op_right = mDO.right_val; 

        if (mDO.op == BRANCH) { // 11000
            // A branch needs to subtract the operands
            cmd = ALU_SUB;
        }
        else if (mDO.op == LOAD || mDO.op == STORE) { // 00000 | 01000
            // For loads and stores, we need to add the
            // offset with the base register.
            cmd = ALU_ADD; // LB/LH/LW/LD or SB/SH/SW/SD
        }
        else if (mDO.op == OP) { // 01100
            // We can't tell which ALU command to use until
            // we read the funct3 and funct7
            switch (mDO.funct3) {
                case 0b000: // ADD or SUB
                    if (mDO.funct7 == 0) cmd = ALU_ADD; // ADD
                    else if (mDO.funct7 == 32) cmd = ALU_SUB; // SUB
                    else if (mDO.funct7 == 1) cmd = ALU_MUL; // MUL
                break;
                // Finish the rest of the OP functions here.
                case 0b001:
                    cmd = ALU_SLL; // SLL
                    break;
                case 0b100:
                    if (mDO.funct7 == 0) cmd = ALU_XOR; // XOR
                    else if (mDO.funct7 == 1) cmd = ALU_DIV; // DIV
                    break;
                case 0b101:
                    if (mDO.funct7 == 0) cmd = ALU_SRL; // SRL
                    else if (mDO.funct7 == 32) cmd = ALU_SRA; // SRA
                    break;
                case 0b110:
                    if (mDO.funct3 == 0) cmd = ALU_OR; // OR
                    else if (mDO.funct7 == 1) cmd = ALU_REM; // REM
                    break;
                case 0b111:
                    cmd = ALU_AND; // AND
                    break;
            }
        }
        else if (mDO.op == OP_32) { // 01110
            op_left = sign_extend(op_left, 31);
            op_right = sign_extend(op_right, 31);
            // You still need to determine the ALU command
            switch (mDO.funct3) {
                case 0b000:
                    if (mDO.funct7 == 0) cmd = ALU_ADD; // ADDW
                    else if (mDO.funct7 == 32) cmd = ALU_SUB; // SUBW
                    else if (mDO.funct7 == 1) cmd = ALU_MUL; // MULW
                    break;
                case 0b101: // DIVUW
                    if (mDO.funct7 == 0) cmd = ALU_SRL; // SRLW
                    else if (mDO.funct7 == 32) cmd = ALU_SRA; // SRAW
                    else if (mDO.funct7 == 1) cmd = ALU_DIV; // DIVUW
                    break;
                case 0b001:
                    cmd = ALU_SLL;
                    break;
                case 0b100:
                    if (mDO.funct7 == 1) cmd = ALU_DIV; // DIVW
                    break;
                case 0b110:
                    if (mDO.funct7 == 1) cmd = ALU_REM; // REMW
                    break;
                case 0b111:
                    if (mDO.funct7 == 1) cmd = ALU_REM; // REMUW
                    break;
            }
        }
        else if (mDO.op == OP_IMM) { // 00100
            // Look and see which ALU op needs to be executed.
            switch (mDO.funct3){
                case 0b000:
                    cmd = ALU_ADD; // ADDI
                    break;
                case 0b100: // XORI
                    cmd = ALU_XOR;
                    break;
                case 0b110:
                    cmd = ALU_OR; // ORI
                    break;
                case 0b111:
                    cmd = ALU_AND; // ANDI
                    break;
                case 0b001:
                    cmd = ALU_SLL; // SLLI
                    break;
                case 0b101:
                    if (mDO.funct7 == 0) cmd = ALU_SRL; // SRLI
                    else if (mDO.funct7 == 32) cmd = ALU_SRA; // SRAI
                    break;
            }
        }
        else if (mDO.op == OP_IMM_32) { // 00110
            op_left = sign_extend(op_left, 31);
            op_right = sign_extend(op_right, 31);
            // This is just like OP_IMM except we truncated
            // the left and right ops.
            switch (mDO.funct3){
                case 0b000:
                    cmd = ALU_ADD; // ADDIW
                    break;
                case 0b001:
                    cmd = ALU_SLL; // SLLIW
                    break;
                case 0b101:
                    if (mDO.funct7 == 0) cmd = ALU_SRL; // SRLIW
                    else if (mDO.funct7 == 32) cmd = ALU_SRA; // SRAIW
                    break;
            }
        }
        else if (mDO.op == JALR) { // 11001
            // JALR has an offset and a register value that
            // need to be added together.
            cmd = ALU_ADD; // JALR
        }
        else if (mDO.op == LUI) { // 01101
            cmd = ALU_ADD;
            
        } 
        else if (mDO.op == AUIPC) { // 00101
            cmd = ALU_ADD; 
            op_left = get_pc();
        }
        else if (mDO.op == JAL) { // 11011
            cmd = ALU_ADD; // JAL
            op_left = get_pc();
            // op_right = OFFSET
        }
        else if (mDO.op == SYSTEM){ // 11100
            cmd = ALU_ADD; // ECALL
        }
        // No-op does not assign a command
        mEO = alu(cmd, op_left, op_right);
    }    
    void memory() {
        if (mDO.op == STORE) {
            switch (mDO.funct3) {
                case 0b000: // SB
                    memory_write<uint8_t>(mEO.result, mDO.right_val);
                    break;
                // Finish here
                case 0b001: // SH
                    memory_write<uint32_t>(mEO.result, mDO.right_val);
                    break;
                case 0b010: // SW
                    memory_write<uint8_t>(mEO.result, mDO.right_val);
                    break;
                case 0b011: // SD
                    memory_write<uint64_t>(mEO.result, mDO.right_val);
                    break;
                default:
                    cerr << "[MEMORY: STORE]: Invalid funct3: " << mDO.funct3 << '\n';
                break;
            }
        }
        else if (mDO.op == LOAD) {
            switch (mDO.funct3) {
                case 0b000: // LB
                    mMO.value = memory_read<int8_t>(mEO.result);
                    break;
                // Finish here
                case 0b001: // LH
                    mMO.value = memory_read<int16_t>(mEO.result);
                    break;
                case 0b010: // LW
                    mMO.value = memory_read<int32_t>(mEO.result);
                    break;
                case 0b100: // LBU
                    mMO.value = memory_read<uint8_t>(mEO.result);
                    break;
                case 0b101: // LHU
                    mMO.value = memory_read<uint16_t>(mEO.result);
                    break;
                case 0b110: // LWU
                    mMO.value = memory_read<uint32_t>(mEO.result);
                    break;
                case 0b011: // LD
                    mMO.value = memory_read<int64_t>(mEO.result);
                    break;
                default:
                    cerr << "[MEMORY: LOAD]: Invalid funct3: " << mDO.funct3 << '\n';
                break;
            }
        }
        else {
            // If this is not a LOAD or STORE, then this stage just copies
            // the ALU result.
            mMO.value = mEO.result;
        }
    }
    void writeback(){
        int64_t system_number = get_xreg(17); // get system number from a7
        switch(mDO.op){
            case SYSTEM: // ECALL Instruction (SYSTEM Opcode) - System call
                switch (system_number){
                    case 0: 
                        exit(0);
                        break;
                    case 1:
                        set_xreg( 10, (getchar() & 0xff) ); // Get char from a0
                        break;
                    case 2:
                        putchar( (char) get_xreg(10) ); // Prints char to the screen
                        break;
                }
                set_pc(get_pc() + 4); 
                break;
            case BRANCH: 
                switch(mDO.funct3){
                    case 0b000: // BEQ 
                        if (mEO.z) set_pc(get_pc() + mDO.offset); // Takes the PC and adds the offset if condition is true
                        else set_pc(get_pc() + 4);  
                        break;
                    case 0b001: // BNE
                        if (!(mEO.z)) set_pc(get_pc() + mDO.offset); // Takes the PC and adds the offset if condition is true
                        else set_pc(get_pc() + 4);  
                        break;
                    case 0b100: // BLT
                        if (mEO.n) set_pc(get_pc() + mDO.offset);
                        else set_pc(get_pc() + 4);  
                        break;
                    case 0b101: // BGE
                        if (mEO.c) set_pc(get_pc() + mDO.offset);
                        else set_pc(get_pc() + 4);  
                        break;
                }
                break;
            case JAL:
                set_xreg(mDO.rd, get_pc()+4); // x[rd] = pc+4
                set_pc(mMO.value); // pc += sext(offset)
                break;
            case JALR:
                set_xreg(mDO.rd, get_pc()+4); // x[rd]=pc+4
                set_pc( mMO.value ); // pc=(x[rs1]+sext(offset))&∼1
                break;
            default:
                set_xreg(mDO.rd, mMO.value);
                set_pc(get_pc() + 4);
                break;
        }
        set_xreg(0, 0);     
    }

    FetchOut &debug_fetch_out() { 
        return mFO; 
    }
    DecodeOut &debug_decode_out() { 
        return mDO; 
    }
    ExecuteOut &debug_execute_out(){
        return mEO;
    }
    MemoryOut &debug_memory_out(){
        return mMO; 
    }
};

int main(int argc, char *argv[]) {

    // .... Code that error checks and reads the file ....
    if(argc < 2){
        std::cerr << "include file\n";
        return -1; 
    }

    char* bin_file = argv[1];
    std::ifstream ifs (bin_file, std::ios::binary);
    if (!(ifs.is_open())){
        std::cerr << "invalid file type\n";
        return -1;
    }

    // Seek to end of file, get length, then return back to top
    ifs.seekg (0, ifs.end);
    int size = ifs.tellg();
    if (size % 4 != 0){
        std::cerr << "invalid file size\n";
        return -1;
    }
    ifs.clear();
    ifs.seekg (0, ifs.beg);
    
    // Allocate char* size of file
    char* mem = new char[MEM_SIZE];
    ifs.read(mem, size);

    Machine mach(mem, MEM_SIZE);
    while (mach.get_pc() < size) {
        mach.fetch();
        cout << mach.debug_fetch_out() << '\n';
        mach.decode();
        //cout << mach.debug_decode_out() << '\n';
        mach.execute();
        //cout << mach.debug_execute_out() << '\n';
        mach.memory();
        //cout << mach.debug_memory_out() << '\n';
        mach.writeback();
    }
    delete[] mem;
    ifs.close();
    return 0;
}

int64_t sign_extend(int64_t value, int8_t index) {
    if ((value >> index) & 1) {
        // Sign bit is 1
        return value | (-1UL << index);
    }
    else {
        // Sign bit is 0
        return value & ~(-1UL << index);
    }
}
