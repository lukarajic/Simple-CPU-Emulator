#include "CPU.hpp"
#include "Memory.hpp"
#include <iostream>
#include <iomanip>

CPU::CPU(Memory& memory) : mem(memory), dcache(64, 64) {
    reset();
}

void CPU::reset() {
    regs.fill(0);
    pc = 0;
    csrs.clear();
    stall = false;
    halted = false;
    if_id_reg = {};
    id_ex_reg = {};
    ex_mem_reg = {};
    mem_wb_reg = {};
}

void CPU::clock() {
    IF_ID_Reg next_if_id = if_id_reg;
    ID_EX_Reg next_id_ex = id_ex_reg;
    EX_MEM_Reg next_ex_mem = ex_mem_reg;
    MEM_WB_Reg next_mem_wb = mem_wb_reg;
    uint32_t target_pc = pc;
    uint32_t sequential_pc = pc;
    bool next_flush = false;

    csrs[CSR_MCYCLE]++;

    wb_stage();
    mem_stage(next_mem_wb);
    ex_stage(next_ex_mem, target_pc, next_flush);
    id_stage(next_id_ex, next_if_id);
    if (!stall) {
        if_stage(next_if_id, sequential_pc);
    }

    if (next_flush) {
        pc = target_pc;
        if_id_reg = {};
        id_ex_reg = {};
    } else {
        pc = sequential_pc;
        if_id_reg = next_if_id;
        id_ex_reg = next_id_ex;
    }
    ex_mem_reg = next_ex_mem;
    mem_wb_reg = next_mem_wb;
    regs[0] = 0;
}

void CPU::wb_stage() {
    if (mem_wb_reg.valid) {
        csrs[CSR_MINSTRET]++;
        if (mem_wb_reg.controls.reg_write && mem_wb_reg.rd != 0) {
            uint32_t result = mem_wb_reg.controls.mem_read ? mem_wb_reg.mem_data : mem_wb_reg.alu_result;
            regs[mem_wb_reg.rd] = result;
        }
        if (mem_wb_reg.controls.halt) {
            halted = true;
        }
    }
}

void CPU::if_stage(IF_ID_Reg& next_if_id, uint32_t& next_pc) {
    next_if_id.instruction = mem.read32(pc);
    next_if_id.pc = pc;
    next_if_id.valid = true;
    next_pc = pc + 4;
}

void CPU::id_stage(ID_EX_Reg& next_id_ex, IF_ID_Reg& next_if_id) {
    (void)next_if_id; // Suppress unused parameter warning
    uint32_t instr = if_id_reg.instruction;
    uint8_t rs1 = (instr >> 15) & 0x1F;
    uint8_t rs2 = (instr >> 20) & 0x1F;
    uint8_t rd = (instr >> 7) & 0x1F;
    uint8_t opcode = instr & 0x7F;

    if (if_id_reg.valid && id_ex_reg.valid && id_ex_reg.controls.mem_read && (id_ex_reg.rd == rs1 || id_ex_reg.rd == rs2) && id_ex_reg.rd != 0) {
        stall = true;
        next_id_ex = {}; 
    } else {
        stall = false;
        next_id_ex.valid = if_id_reg.valid;
        next_id_ex.pc = if_id_reg.pc;
        next_id_ex.rs1 = rs1;
        next_id_ex.rs2 = rs2;
        next_id_ex.rd = rd;
        next_id_ex.reg_val1 = regs[rs1];
        next_id_ex.reg_val2 = regs[rs2];
        next_id_ex.imm = 0;
        next_id_ex.controls = {};
        next_id_ex.controls.funct3 = (instr >> 12) & 0x07;
        next_id_ex.controls.funct7 = (instr >> 25) & 0x7F;

        if (if_id_reg.valid) {
            switch (opcode) {
                case 0x37: next_id_ex.controls.reg_write = true; next_id_ex.imm = (instr & 0xFFFFF000); next_id_ex.controls.alu_op = 0; break;
                case 0x17: next_id_ex.controls.reg_write = true; next_id_ex.imm = (instr & 0xFFFFF000); next_id_ex.controls.alu_op = 1; break;
                case 0x6F: next_id_ex.controls.reg_write = true; next_id_ex.controls.jump = true; next_id_ex.imm = sign_extend(((instr >> 31) << 20) | (((instr >> 12) & 0xFF) << 12) | (((instr >> 20) & 0x1) << 11) | (((instr >> 21) & 0x3FF) << 1), 21); next_id_ex.controls.alu_op = 2; break;
                case 0x67: next_id_ex.controls.reg_write = true; next_id_ex.controls.jump = true; next_id_ex.controls.alu_src = true; next_id_ex.imm = sign_extend((instr >> 20) & 0xFFF, 12); next_id_ex.controls.alu_op = 3; break;
                case 0x63: next_id_ex.controls.branch = true; next_id_ex.imm = sign_extend(((instr >> 31) << 12) | (((instr >> 7) & 0x1) << 11) | (((instr >> 25) & 0x3F) << 5) | (((instr >> 8) & 0xF) << 1), 13); next_id_ex.controls.alu_op = 4; break;
                case 0x03: next_id_ex.controls.reg_write = true; next_id_ex.controls.mem_read = true; next_id_ex.controls.alu_src = true; next_id_ex.imm = sign_extend((instr >> 20) & 0xFFF, 12); next_id_ex.controls.alu_op = 5; break;
                case 0x23: next_id_ex.controls.mem_write = true; next_id_ex.controls.alu_src = true; next_id_ex.imm = sign_extend(((instr >> 25) << 5) | ((instr >> 7) & 0x1F), 12); next_id_ex.controls.alu_op = 6; break;
                case 0x13: next_id_ex.controls.reg_write = true; next_id_ex.controls.alu_src = true; next_id_ex.imm = sign_extend((instr >> 20) & 0xFFF, 12); next_id_ex.controls.alu_op = 7; break;
                case 0x33: next_id_ex.controls.reg_write = true; next_id_ex.controls.alu_op = 8; break;
                case 0x73: 
                    next_id_ex.controls.reg_write = true; 
                    next_id_ex.controls.alu_op = 9; 
                    next_id_ex.imm = (instr >> 20); 
                    if (next_id_ex.imm == 0 && next_id_ex.controls.funct3 == 0) {
                        next_id_ex.controls.halt = true;
                    }
                    break;
            }
        }
    }
}

void CPU::ex_stage(EX_MEM_Reg& next_ex_mem, uint32_t& next_pc, bool& flush) {
    uint32_t op1 = id_ex_reg.reg_val1;
    uint32_t op2 = id_ex_reg.reg_val2;
    // Forwarding
    if (ex_mem_reg.valid && ex_mem_reg.controls.reg_write && ex_mem_reg.rd != 0) {
        if (ex_mem_reg.rd == id_ex_reg.rs1) op1 = ex_mem_reg.alu_result;
        if (ex_mem_reg.rd == id_ex_reg.rs2) op2 = ex_mem_reg.alu_result;
    }
    if (mem_wb_reg.valid && mem_wb_reg.controls.reg_write && mem_wb_reg.rd != 0) {
        uint32_t wb_data = mem_wb_reg.controls.mem_read ? mem_wb_reg.mem_data : mem_wb_reg.alu_result;
        if (mem_wb_reg.rd == id_ex_reg.rs1 && !(ex_mem_reg.valid && ex_mem_reg.controls.reg_write && ex_mem_reg.rd == id_ex_reg.rs1)) op1 = wb_data;
        if (mem_wb_reg.rd == id_ex_reg.rs2 && !(ex_mem_reg.valid && ex_mem_reg.controls.reg_write && ex_mem_reg.rd == id_ex_reg.rs2)) op2 = wb_data;
    }

    uint32_t alu_op2 = id_ex_reg.controls.alu_src ? id_ex_reg.imm : op2;
    uint32_t alu_res = 0;
    if (id_ex_reg.valid) {
        uint8_t alu_op = id_ex_reg.controls.alu_op;
        uint8_t funct3 = id_ex_reg.controls.funct3;
        uint8_t funct7 = id_ex_reg.controls.funct7;
        switch (alu_op) {
            case 0: alu_res = id_ex_reg.imm; break; // LUI
            case 1: alu_res = id_ex_reg.pc + id_ex_reg.imm; break; // AUIPC
            case 2: case 3: alu_res = id_ex_reg.pc + 4; break; // JAL, JALR
            case 7: case 8: // OP-IMM, OP
                switch (funct3) {
                    case 0x0: alu_res = (alu_op == 8 && funct7 == 0x20) ? op1 - alu_op2 : op1 + alu_op2; break;
                    case 0x1: alu_res = op1 << (alu_op2 & 0x1F); break;
                    case 0x2: alu_res = ((int32_t)op1 < (int32_t)alu_op2) ? 1 : 0; break;
                    case 0x3: alu_res = (op1 < alu_op2) ? 1 : 0; break;
                    case 0x4: alu_res = op1 ^ alu_op2; break;
                    case 0x5: alu_res = (funct7 == 0x20) ? (int32_t)op1 >> (alu_op2 & 0x1F) : op1 >> (alu_op2 & 0x1F); break;
                    case 0x6: alu_res = op1 | alu_op2; break;
                    case 0x7: alu_res = op1 & alu_op2; break;
                }
                break;
            case 5: case 6: alu_res = op1 + alu_op2; break; // LOAD, STORE
            case 9: // SYSTEM
                uint32_t csr_addr = id_ex_reg.imm;
                uint8_t f3 = id_ex_reg.controls.funct3;
                if (f3 == 0) { // ECALL or MRET
                    if (csr_addr == 0x0) { // ECALL
                    flush = true;
                    if (csrs.count(CSR_MTVEC) && csrs[CSR_MTVEC] != 0) {
                        trap(CAUSE_ECALL_M_MODE, id_ex_reg.pc);
                        next_pc = pc;
                    } else {
                        next_pc = id_ex_reg.pc;
                    }
                } else if (csr_addr == 0x302) { // MRET
                        next_pc = csrs.count(CSR_MEPC) ? csrs[CSR_MEPC] : 0;
                        flush = true;
                    }
                } else { // CSR
                    uint32_t t = csrs.count(csr_addr) ? csrs[csr_addr] : 0;
                    if (id_ex_reg.rd != 0) alu_res = t;
                    if (f3 == 1) csrs[csr_addr] = op1;
                    else if (f3 == 2) csrs[csr_addr] = t | op1;
                    else if (f3 == 3) csrs[csr_addr] = t & ~op1;
                    else if (f3 == 5) csrs[csr_addr] = id_ex_reg.rs1;
                    else if (f3 == 6) csrs[csr_addr] = t | id_ex_reg.rs1;
                    else if (f3 == 7) csrs[csr_addr] = t & ~id_ex_reg.rs1;
                }
                break;
        }
        // Jumps/Branches
        if (id_ex_reg.controls.jump) {
            flush = true;
            next_pc = (alu_op == 2) ? id_ex_reg.pc + id_ex_reg.imm : (op1 + id_ex_reg.imm) & ~1;
        } else if (id_ex_reg.controls.branch) {
            bool take = false;
            switch (funct3) {
                case 0x0: take = (op1 == op2); break;
                case 0x1: take = (op1 != op2); break;
                case 0x4: take = ((int32_t)op1 < (int32_t)op2); break;
                case 0x5: take = ((int32_t)op1 >= (int32_t)op2); break;
                case 0x6: take = (op1 < op2); break;
                case 0x7: take = (op1 >= op2); break;
            }
            if (take) { flush = true; next_pc = id_ex_reg.pc + id_ex_reg.imm; }
        }
    }
    next_ex_mem.valid = id_ex_reg.valid;
    next_ex_mem.alu_result = alu_res;
    next_ex_mem.reg_val2 = op2;
    next_ex_mem.rd = id_ex_reg.rd;
    next_ex_mem.controls = id_ex_reg.controls;
}

void CPU::mem_stage(MEM_WB_Reg& next_mem_wb) {
    next_mem_wb.valid = ex_mem_reg.valid;
    next_mem_wb.controls = ex_mem_reg.controls;
    next_mem_wb.rd = ex_mem_reg.rd;
    next_mem_wb.alu_result = ex_mem_reg.alu_result;
    uint32_t addr = ex_mem_reg.alu_result;
    uint8_t funct3 = ex_mem_reg.controls.funct3;

    // Cache Access (only for non-UART addresses)
    if (ex_mem_reg.valid && (ex_mem_reg.controls.mem_read || ex_mem_reg.controls.mem_write)) {
        if (addr < Memory::UART_BASE) {
            dcache.access(addr, ex_mem_reg.controls.mem_write);
        }
    }

    if (ex_mem_reg.valid && ex_mem_reg.controls.mem_read) {
        switch (funct3) {
            case 0x0: next_mem_wb.mem_data = sign_extend(mem.read8(addr), 8); break;
            case 0x1: next_mem_wb.mem_data = sign_extend(mem.read16(addr), 16); break;
            case 0x2: next_mem_wb.mem_data = mem.read32(addr); break;
            case 0x4: next_mem_wb.mem_data = mem.read8(addr); break;
            case 0x5: next_mem_wb.mem_data = mem.read16(addr); break;
        }
    }
    if (ex_mem_reg.valid && ex_mem_reg.controls.mem_write) {
        switch (funct3) {
            case 0x0: mem.write8(addr, ex_mem_reg.reg_val2 & 0xFF); break;
            case 0x1: mem.write16(addr, ex_mem_reg.reg_val2 & 0xFFFF); break;
            case 0x2: mem.write32(addr, ex_mem_reg.reg_val2); break;
        }
    }
}

// --- Other Methods (unchanged for now, but execute_* are gone) ---

void CPU::trap(uint32_t cause, uint32_t trap_pc, uint32_t tval) {
    csrs[CSR_MCAUSE] = cause;
    csrs[CSR_MEPC] = trap_pc;
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
