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
    // uint8_t funct3 = (instr >> 12) & 0x07; // Will be used for more detailed control signals
    id_ex_reg.rs1 = (instr >> 15) & 0x1F;
    id_ex_reg.rs2 = (instr >> 20) & 0x1F;
    // uint8_t funct7 = (instr >> 25) & 0x7F; // Will be used for more detailed control signals

    id_ex_reg.reg_val1 = regs[id_ex_reg.rs1];
    id_ex_reg.reg_val2 = regs[id_ex_reg.rs2];

    // Default all controls to off
    id_ex_reg.controls = {};

    // TODO: This is where a large part of the control logic will go
    // For now, we'll just decode a few instruction types naively.
    switch (opcode) {
        case 0x33: // R-type
            id_ex_reg.controls.reg_write = true;
            break;
        case 0x13: // I-type
            id_ex_reg.controls.reg_write = true;
            id_ex_reg.controls.alu_src = true; // Use immediate
            id_ex_reg.imm = sign_extend((instr >> 20) & 0xFFF, 12);
            break;
        case 0x03: // Load
            id_ex_reg.controls.reg_write = true;
            id_ex_reg.controls.mem_read = true;
            id_ex_reg.controls.alu_src = true; // Use immediate for address calculation
            id_ex_reg.imm = sign_extend((instr >> 20) & 0xFFF, 12);
            break;
        // Other opcodes will be added here
    }
}

void CPU::ex_stage() {
    uint32_t operand1 = id_ex_reg.reg_val1;
    uint32_t operand2 = id_ex_reg.controls.alu_src ? id_ex_reg.imm : id_ex_reg.reg_val2;
    
    // Simple ALU for now, just does addition
    ex_mem_reg.alu_result = operand1 + operand2;
    
    ex_mem_reg.reg_val2 = id_ex_reg.reg_val2; // Forward value for stores
    ex_mem_reg.rd = id_ex_reg.rd;
    ex_mem_reg.controls = id_ex_reg.controls;
}

void CPU::mem_stage() {
    mem_wb_reg.controls = ex_mem_reg.controls;
    mem_wb_reg.rd = ex_mem_reg.rd;
    mem_wb_reg.alu_result = ex_mem_reg.alu_result;

    if (ex_mem_reg.controls.mem_read) {
        mem_wb_reg.mem_data = mem.read32(ex_mem_reg.alu_result);
    }
    if (ex_mem_reg.controls.mem_write) {
        mem.write32(ex_mem_reg.alu_result, ex_mem_reg.reg_val2);
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
