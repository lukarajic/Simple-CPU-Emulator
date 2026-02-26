#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <array>
#include <vector>
#include <unordered_map>
#include <string>

class Memory; // Forward declaration

// Control signals for the pipeline
struct ControlUnit {
    bool reg_write = false;
    bool mem_read = false;
    bool mem_write = false;
    bool branch = false;
    bool jump = false;
    uint8_t alu_op = 0; // 0:ADD, 1:SUB, 2:AND, 3:OR, 4:XOR, 5:SLT, 6:SLTU, 7:SLL, 8:SRL, 9:SRA
    bool alu_src = false; // false: reg, true: immediate
};

// Pipeline Registers
struct IF_ID_Reg {
    uint32_t instruction = 0;
    uint32_t pc = 0;
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
};

struct EX_MEM_Reg {
    uint32_t alu_result = 0;
    uint32_t reg_val2 = 0; // Value to store
    uint8_t rd = 0;
    ControlUnit controls;
};

struct MEM_WB_Reg {
    uint32_t mem_data = 0;
    uint32_t alu_result = 0;
    uint8_t rd = 0;
    ControlUnit controls;
};

class CPU {
public:
    CPU(Memory& memory);

    // CSR Addresses
    static constexpr uint32_t CSR_MSTATUS = 0x300, CSR_MTVEC = 0x305, CSR_MEPC = 0x341;
    static constexpr uint32_t CSR_MCAUSE = 0x342, CSR_MTVAL = 0x343, CSR_MIE = 0x304;
    static constexpr uint32_t CSR_MIP = 0x344, CSR_MCYCLE = 0xB00;

    // Exception Causes
    static constexpr uint32_t CAUSE_ECALL_M_MODE = 11;

    void reset();
    void clock(); // Main method to advance the pipeline by one cycle

    // Debugging and Testing
    void dump_registers() const;
    uint32_t get_reg(int reg_num) const;
    uint32_t get_csr(uint32_t csr_addr) const;
    uint32_t fetch_pc() const { return pc; }

private:
    std::array<uint32_t, 32> regs;
    uint32_t pc;
    Memory& mem;
    std::unordered_map<uint32_t, uint32_t> csrs;

    // Pipeline registers
    IF_ID_Reg if_id_reg;
    ID_EX_Reg id_ex_reg;
    EX_MEM_Reg ex_mem_reg;
    MEM_WB_Reg mem_wb_reg;

    // Pipeline stage methods
    void if_stage();
    void id_stage();
    void ex_stage();
    void mem_stage();
    void wb_stage();

    // Private helpers
    void trap(uint32_t cause, uint32_t tval = 0);
    int32_t sign_extend(uint32_t value, int bits);
};

#endif // CPU_HPP
