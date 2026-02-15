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
    int32_t imm = sign_extend((instr >> 20) & 0xFFF, 12);

    if (opcode == 0x13) { // OP-IMM
        switch (funct3) {
            case 0x0: // ADDI
                if (rd != 0) {
                    regs[rd] = regs[rs1] + imm;
                }
                break;
            default:
                std::cerr << "Unknown funct3 for OP-IMM: 0x" << std::hex << (int)funct3 << std::endl;
                break;
        }
    } else {
        std::cerr << "Unknown opcode: 0x" << std::hex << (int)opcode << std::endl;
    }

    // Ensure x0 is always 0 (redundant here but good practice)
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
