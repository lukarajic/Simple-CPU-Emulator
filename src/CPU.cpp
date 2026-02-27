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
    csrs.clear();
    if_id_reg = {};
    id_ex_reg = {};
    ex_mem_reg = {};
    mem_wb_reg = {};
}

void CPU::clock() {
    // Execute pipeline stages in reverse order to simulate data flow
    wb_stage();
    mem_stage();
    ex_stage();
    id_stage();
    if_stage();
}

void CPU::if_stage() {
    if_id_reg.instruction = mem.read32(pc);
    if_id_reg.pc = pc;
    pc += 4;
}

void CPU::id_stage() {
    uint32_t instr = if_id_reg.instruction;
    id_ex_reg.pc = if_id_reg.pc;

    uint8_t opcode = instr & 0x7F;
    id_ex_reg.rd = (instr >> 7) & 0x1F;
    uint8_t funct3 = (instr >> 12) & 0x07;
    id_ex_reg.rs1 = (instr >> 15) & 0x1F;
    id_ex_reg.rs2 = (instr >> 20) & 0x1F;
    uint8_t funct7 = (instr >> 25) & 0x7F;

    id_ex_reg.reg_val1 = regs[id_ex_reg.rs1];
    id_ex_reg.reg_val2 = regs[id_ex_reg.rs2];

    id_ex_reg.controls = {}; // Default: all false/0
    id_ex_reg.controls.funct3 = funct3;
    id_ex_reg.controls.funct7 = funct7;

    switch (opcode) {
        case 0x37: // LUI
            id_ex_reg.controls.reg_write = true;
            id_ex_reg.imm = (instr & 0xFFFFF000);
            id_ex_reg.controls.alu_op = 0;
            break;
        case 0x17: // AUIPC
            id_ex_reg.controls.reg_write = true;
            id_ex_reg.imm = (instr & 0xFFFFF000);
            id_ex_reg.controls.alu_op = 1;
            break;
        case 0x6F: // JAL
            id_ex_reg.controls.reg_write = true;
            id_ex_reg.controls.jump = true;
            id_ex_reg.imm = sign_extend(((instr >> 31) << 20) | (((instr >> 12) & 0xFF) << 12) | (((instr >> 20) & 0x1) << 11) | (((instr >> 21) & 0x3FF) << 1), 21);
            id_ex_reg.controls.alu_op = 2;
            break;
        case 0x67: // JALR
            id_ex_reg.controls.reg_write = true;
            id_ex_reg.controls.jump = true;
            id_ex_reg.controls.alu_src = true;
            id_ex_reg.imm = sign_extend((instr >> 20) & 0xFFF, 12);
            id_ex_reg.controls.alu_op = 3;
            break;
        case 0x63: // BRANCH
            id_ex_reg.controls.branch = true;
            id_ex_reg.imm = sign_extend(((instr >> 31) << 12) | (((instr >> 7) & 0x1) << 11) | (((instr >> 25) & 0x3F) << 5) | (((instr >> 8) & 0xF) << 1), 13);
            id_ex_reg.controls.alu_op = 4;
            break;
        case 0x03: // LOAD
            id_ex_reg.controls.reg_write = true;
            id_ex_reg.controls.mem_read = true;
            id_ex_reg.controls.alu_src = true;
            id_ex_reg.imm = sign_extend((instr >> 20) & 0xFFF, 12);
            id_ex_reg.controls.alu_op = 5;
            break;
        case 0x23: // STORE
            id_ex_reg.controls.mem_write = true;
            id_ex_reg.controls.alu_src = true;
            id_ex_reg.imm = sign_extend(((instr >> 25) << 5) | ((instr >> 7) & 0x1F), 12);
            id_ex_reg.controls.alu_op = 6;
            break;
        case 0x13: // OP-IMM
            id_ex_reg.controls.reg_write = true;
            id_ex_reg.controls.alu_src = true;
            id_ex_reg.imm = sign_extend((instr >> 20) & 0xFFF, 12);
            id_ex_reg.controls.alu_op = 7;
            break;
        case 0x33: // OP
            id_ex_reg.controls.reg_write = true;
            id_ex_reg.controls.alu_op = 8;
            break;
        default:
            // Placeholder for unknown or system opcodes
            break;
    }
}

void CPU::ex_stage() {
    uint32_t op1 = id_ex_reg.reg_val1;
    uint32_t op2 = id_ex_reg.controls.alu_src ? id_ex_reg.imm : id_ex_reg.reg_val2;
    uint32_t result = 0;

    uint8_t alu_op = id_ex_reg.controls.alu_op;
    uint8_t funct3 = id_ex_reg.controls.funct3;
    uint8_t funct7 = id_ex_reg.controls.funct7;

    if (alu_op == 0) { // LUI
        result = id_ex_reg.imm;
    } else if (alu_op == 1) { // AUIPC
        result = id_ex_reg.pc + id_ex_reg.imm;
    } else if (alu_op == 2 || alu_op == 3) { // JAL, JALR
        result = id_ex_reg.pc + 4; // Return address
        // Hazard handling will be needed here for PC updates
    } else if (alu_op == 7 || alu_op == 8) { // OP-IMM or OP
        switch (funct3) {
            case 0x0: // ADD / SUB
                if (alu_op == 8 && funct7 == 0x20) result = op1 - op2;
                else result = op1 + op2;
                break;
            case 0x1: result = op1 << (op2 & 0x1F); break; // SLL
            case 0x2: result = ((int32_t)op1 < (int32_t)op2) ? 1 : 0; break; // SLT
            case 0x3: result = (op1 < op2) ? 1 : 0; break; // SLTU
            case 0x4: result = op1 ^ op2; break; // XOR
            case 0x5: // SRL / SRA
                if (funct7 == 0x20) result = (int32_t)op1 >> (op2 & 0x1F);
                else result = op1 >> (op2 & 0x1F);
                break;
            case 0x6: result = op1 | op2; break; // OR
            case 0x7: result = op1 & op2; break; // AND
        }
    } else if (alu_op == 5 || alu_op == 6) { // LOAD or STORE
        result = op1 + op2; // Address calculation
    }

    ex_mem_reg.alu_result = result;
    ex_mem_reg.reg_val2 = id_ex_reg.reg_val2;
    ex_mem_reg.rd = id_ex_reg.rd;
    ex_mem_reg.controls = id_ex_reg.controls;
}

void CPU::mem_stage() {
    mem_wb_reg.controls = ex_mem_reg.controls;
    mem_wb_reg.rd = ex_mem_reg.rd;
    mem_wb_reg.alu_result = ex_mem_reg.alu_result;

    uint8_t funct3 = ex_mem_reg.controls.funct3;
    uint32_t addr = ex_mem_reg.alu_result;

    if (ex_mem_reg.controls.mem_read) {
        switch (funct3) {
            case 0x0: mem_wb_reg.mem_data = sign_extend(mem.read8(addr), 8); break;   // LB
            case 0x1: mem_wb_reg.mem_data = sign_extend(mem.read16(addr), 16); break; // LH
            case 0x2: mem_wb_reg.mem_data = mem.read32(addr); break;                 // LW
            case 0x4: mem_wb_reg.mem_data = mem.read8(addr); break;                  // LBU
            case 0x5: mem_wb_reg.mem_data = mem.read16(addr); break;                 // LHU
        }
    }
    if (ex_mem_reg.controls.mem_write) {
        switch (funct3) {
            case 0x0: mem.write8(addr, ex_mem_reg.reg_val2 & 0xFF); break;    // SB
            case 0x1: mem.write16(addr, ex_mem_reg.reg_val2 & 0xFFFF); break; // SH
            case 0x2: mem.write32(addr, ex_mem_reg.reg_val2); break;          // SW
        }
    }
}

void CPU::wb_stage() {
    if (mem_wb_reg.controls.reg_write && mem_wb_reg.rd != 0) {
        uint32_t result = mem_wb_reg.controls.mem_read ? mem_wb_reg.mem_data : mem_wb_reg.alu_result;
        regs[mem_wb_reg.rd] = result;
    }
}

// --- Other Methods (unchanged for now, but execute_* are gone) ---

void CPU::trap(uint32_t cause, uint32_t tval) {
    csrs[CSR_MCAUSE] = cause;
    csrs[CSR_MEPC] = pc - 4;
    csrs[CSR_MTVAL] = tval;
    pc = csrs.count(CSR_MTVEC) ? csrs[CSR_MTVEC] : 0;
}

int32_t CPU::sign_extend(uint32_t value, int bits) {
    if (value & (1 << (bits - 1))) {
        return (int32_t)(value | (0xFFFFFFFF << bits));
    }
    return (int32_t)value;
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

uint32_t CPU::get_reg(int reg_num) const {
    if (reg_num < 0 || reg_num > 31) return 0;
    return regs[reg_num];
}

uint32_t CPU::get_csr(uint32_t csr_addr) const {
    auto it = csrs.find(csr_addr);
    if (it != csrs.end()) return it->second;
    return 0;
}
