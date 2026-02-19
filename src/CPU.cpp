#include "CPU.hpp"
#include "Memory.hpp"
#include <iostream>
#include <iomanip>

CPU::CPU(Memory& memory) : mem(memory) {
    reset();
}

void CPU::reset() {
    regs.fill(0);
    pc = 0;
}

uint32_t CPU::fetch() {
    uint32_t instr = mem.read32(pc);
    pc += 4;
    return instr;
}

void CPU::step() {
    uint32_t instr = fetch();
    execute(instr);
}

void CPU::execute(uint32_t instr) {
    uint8_t opcode = instr & 0x7F;
    uint8_t rd = (instr >> 7) & 0x1F;
    uint8_t funct3 = (instr >> 12) & 0x07;
    uint8_t rs1 = (instr >> 15) & 0x1F;
    uint8_t rs2 = (instr >> 20) & 0x1F;
    uint8_t funct7 = (instr >> 25) & 0x7F;

    // I-type immediate (for ADDI, etc.)
    int32_t imm = sign_extend((instr >> 20) & 0xFFF, 12);
    // S-type immediate (for STORE)
    int32_t s_imm = sign_extend(((instr >> 25) << 5) | ((instr >> 7) & 0x1F), 12);
    // U-type immediate (for LUI, AUIPC)
    int32_t u_imm = (instr & 0xFFFFF000);
    // Shift amount for immediate shifts (SLLI, SRLI, SRAI)
    uint8_t shamt = (instr >> 20) & 0x1F;

    if (opcode == 0x37) { // LUI
        if (rd != 0) regs[rd] = u_imm;
    } else if (opcode == 0x13) { // OP-IMM
        switch (funct3) {
            case 0x0: // ADDI
                if (rd != 0) regs[rd] = regs[rs1] + imm;
                break;
            case 0x1: // SLLI
                if (funct7 == 0x00) {
                    if (rd != 0) regs[rd] = regs[rs1] << shamt;
                } else {
                    std::cerr << "Invalid funct7 for SLLI: 0x" << std::hex << (int)funct7 << std::endl;
                }
                break;
            case 0x2: // SLTI
                if (rd != 0) regs[rd] = ((int32_t)regs[rs1] < imm) ? 1 : 0;
                break;
            case 0x3: // SLTIU
                if (rd != 0) regs[rd] = (regs[rs1] < (uint32_t)imm) ? 1 : 0;
                break;
            case 0x4: // XORI
                if (rd != 0) regs[rd] = regs[rs1] ^ imm;
                break;
            case 0x5: // SRLI / SRAI
                if (funct7 == 0x00) { // SRLI
                    if (rd != 0) regs[rd] = regs[rs1] >> shamt;
                } else if (funct7 == 0x20) { // SRAI
                    if (rd != 0) regs[rd] = (int32_t)regs[rs1] >> shamt;
                } else {
                    std::cerr << "Invalid funct7 for SRLI/SRAI: 0x" << std::hex << (int)funct7 << std::endl;
                }
                break;
            case 0x6: // ORI
                if (rd != 0) regs[rd] = regs[rs1] | imm;
                break;
            case 0x7: // ANDI
                if (rd != 0) regs[rd] = regs[rs1] & imm;
                break;
            default:
                std::cerr << "Unknown funct3 for OP-IMM: 0x" << std::hex << (int)funct3 << std::endl;
                break;
        }
    } else if (opcode == 0x03) { // LOAD
        uint32_t addr = regs[rs1] + imm;
        switch (funct3) {
            case 0x0: // LB
                if (rd != 0) regs[rd] = sign_extend(mem.read8(addr), 8);
                break;
            case 0x1: // LH
                if (rd != 0) regs[rd] = sign_extend(mem.read16(addr), 16);
                break;
            case 0x2: // LW
                if (rd != 0) regs[rd] = mem.read32(addr);
                break;
            case 0x4: // LBU
                if (rd != 0) regs[rd] = mem.read8(addr);
                break;
            case 0x5: // LHU
                if (rd != 0) regs[rd] = mem.read16(addr);
                break;
            default:
                std::cerr << "Unknown funct3 for LOAD: 0x" << std::hex << (int)funct3 << std::endl;
                break;
        }
    } else if (opcode == 0x23) { // STORE
        uint32_t addr = regs[rs1] + s_imm;
        switch (funct3) {
            case 0x0: // SB
                mem.write8(addr, regs[rs2] & 0xFF);
                break;
            case 0x1: // SH
                mem.write16(addr, regs[rs2] & 0xFFFF);
                break;
            case 0x2: // SW
                mem.write32(addr, regs[rs2]);
                break;
            default:
                std::cerr << "Unknown funct3 for STORE: 0x" << std::hex << (int)funct3 << std::endl;
                break;
        }
    } else if (opcode == 0x33) { // OP (R-type)
        switch (funct3) {
            case 0x0: 
                if (funct7 == 0x00) { // ADD
                    if (rd != 0) regs[rd] = regs[rs1] + regs[rs2];
                } else if (funct7 == 0x20) { // SUB
                    if (rd != 0) regs[rd] = regs[rs1] - regs[rs2];
                }
                break;
            case 0x1: // SLL (Shift Left Logical)
                if (rd != 0) regs[rd] = regs[rs1] << (regs[rs2] & 0x1F);
                break;
            case 0x2: // SLT (Set Less Than)
                if (rd != 0) regs[rd] = ((int32_t)regs[rs1] < (int32_t)regs[rs2]) ? 1 : 0;
                break;
            case 0x3: // SLTU (Set Less Than Unsigned)
                if (rd != 0) regs[rd] = (regs[rs1] < regs[rs2]) ? 1 : 0;
                break;
            case 0x4: // XOR
                if (rd != 0) regs[rd] = regs[rs1] ^ regs[rs2];
                break;
            case 0x5:
                if (funct7 == 0x00) { // SRL (Shift Right Logical)
                    if (rd != 0) regs[rd] = regs[rs1] >> (regs[rs2] & 0x1F);
                } else if (funct7 == 0x20) { // SRA (Shift Right Arithmetic)
                    if (rd != 0) regs[rd] = (int32_t)regs[rs1] >> (regs[rs2] & 0x1F);
                }
                break;
            case 0x6: // OR
                if (rd != 0) regs[rd] = regs[rs1] | regs[rs2];
                break;
            case 0x7: // AND
                if (rd != 0) regs[rd] = regs[rs1] & regs[rs2];
                break;
            default:
                std::cerr << "Unknown funct3 for OP: 0x" << std::hex << (int)funct3 << std::endl;
                break;
        }
    } else {
        std::cerr << "Unknown opcode: 0x" << std::hex << (int)opcode << std::endl;
    }

    // Ensure x0 is always 0
    regs[0] = 0;
}

void CPU::dump_registers() const {
    std::cout << "--- CPU State ---" << std::endl;
    std::cout << "PC: 0x" << std::hex << std::setw(8) << std::setfill('0') << pc << std::endl;
    for (int i = 0; i < 32; ++i) {
        std::cout << "x" << std::dec << std::setw(2) << std::setfill(' ') << i << ": 0x" 
                  << std::hex << std::setw(8) << std::setfill('0') << regs[i] << "  ";
        if ((i + 1) % 4 == 0) std::cout << std::endl;
    }
    std::cout << "-----------------" << std::endl;
}
