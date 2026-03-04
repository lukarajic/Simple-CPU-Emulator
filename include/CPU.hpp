#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <array>
#include <vector>
#include <unordered_map>
#include <string>
#include "Cache.hpp"

class Memory; // Forward declaration

// Control signals for the pipeline
struct ControlUnit {
    bool reg_write = false;
    bool mem_read = false;
    bool mem_write = false;
    bool branch = false;
    bool jump = false;
    bool halt = false;
    uint8_t alu_op = 0; // Opcode group: 0:LUI, 1:AUIPC, 2:JAL, 3:JALR, 4:BRANCH, 5:LOAD, 6:STORE, 7:OP-IMM, 8:OP, 9:SYSTEM
    uint8_t funct3 = 0;
    uint8_t funct7 = 0;
    bool alu_src = false; // false: reg, true: immediate
};

// Pipeline Registers
struct IF_ID_Reg {
    uint32_t instruction = 0;
    uint32_t pc = 0;
    bool valid = false;
};

struct ID_EX_Reg {
    uint32_t pc = 0;
    uint32_t reg_val1 = 0;
    uint32_t reg_val2 = 0;
    int32_t imm = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;
    uint8_t rd = 0;
    ControlUnit controls;
    bool valid = false;
};

struct EX_MEM_Reg {
    uint32_t alu_result = 0;
    uint32_t reg_val2 = 0; // Value to store
    uint8_t rd = 0;
    ControlUnit controls;
    bool valid = false;
};

struct MEM_WB_Reg {
    uint32_t mem_data = 0;
    uint32_t alu_result = 0;
    uint8_t rd = 0;
    ControlUnit controls;
    bool valid = false;
};

class CPU {
public:
    CPU(Memory& memory);

    // CSR Addresses
    static constexpr uint32_t CSR_MSTATUS = 0x300, CSR_MTVEC = 0x305, CSR_MEPC = 0x341;
    static constexpr uint32_t CSR_MCAUSE  = 0x342, CSR_MTVAL = 0x343, CSR_MIE = 0x304;
    static constexpr uint32_t CSR_MIP = 0x344, CSR_MCYCLE = 0xB00, CSR_MINSTRET = 0xB02;


    // Exception Causes
    static constexpr uint32_t CAUSE_ECALL_M_MODE = 11;

    void reset();
    void clock(); // Main method to advance the pipeline by one cycle

    // Debugging and Testing
    void dump_registers() const;
    uint32_t get_reg(int reg_num) const;
    uint32_t get_csr(uint32_t csr_addr) const;
    uint32_t fetch_pc() const { return pc; }
    bool is_halted() const { return halted; }

    // Cache Stats
    uint32_t get_cache_hits() const { return dcache.get_hits(); }
    uint32_t get_cache_misses() const { return dcache.get_misses(); }

private:
    std::array<uint32_t, 32> regs;
    uint32_t pc;
    Memory& mem;
    std::unordered_map<uint32_t, uint32_t> csrs;

    Cache dcache;

    bool stall = false;
    bool halted = false;

    // Pipeline registers
    IF_ID_Reg if_id_reg;
    ID_EX_Reg id_ex_reg;
    EX_MEM_Reg ex_mem_reg;
    MEM_WB_Reg mem_wb_reg;

    // Pipeline stage methods
    void if_stage(IF_ID_Reg& next_if_id, uint32_t& next_pc);
    void id_stage(ID_EX_Reg& next_id_ex, IF_ID_Reg& next_if_id);
    void ex_stage(EX_MEM_Reg& next_ex_mem, uint32_t& next_pc, bool& flush);
    void mem_stage(MEM_WB_Reg& next_mem_wb);
    void wb_stage();

    // Private helpers
    void trap(uint32_t cause, uint32_t trap_pc, uint32_t tval = 0);
    int32_t sign_extend(uint32_t value, int bits);
};

#endif // CPU_HPP
