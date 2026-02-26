#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <array>
#include <unordered_map>

class Memory; // Forward declaration

class CPU {
public:
    CPU(Memory& memory);

    // CSR Addresses (Machine Mode)
    static constexpr uint32_t CSR_MSTATUS = 0x300;
    static constexpr uint32_t CSR_MTVEC   = 0x305;
    static constexpr uint32_t CSR_MEPC    = 0x341;
    static constexpr uint32_t CSR_MCAUSE  = 0x342;
    static constexpr uint32_t CSR_MTVAL   = 0x343;
    static constexpr uint32_t CSR_MIE     = 0x304;
    static constexpr uint32_t CSR_MIP     = 0x344;
    static constexpr uint32_t CSR_MCYCLE  = 0xB00;

    // Reset the CPU state
    void reset();

    // Fetch the next instruction from memory
    uint32_t fetch();

    // Execute a single instruction cycle (Fetch + Execute)
    void step();

    // Execute a specific instruction
    void execute(uint32_t instr);

    // Debug: Print register contents
    void dump_registers() const;

    // Get a specific register value
    uint32_t get_reg(int reg_num) const;

    // Get a specific CSR value
    uint32_t get_csr(uint32_t csr_addr) const;

    // Get current PC
    uint32_t fetch_pc() const { return pc; }

private:
    std::array<uint32_t, 32> regs; // x0-x31
    uint32_t pc;                   // Program Counter
    Memory& mem;                   // Reference to system memory
    std::unordered_map<uint32_t, uint32_t> csrs; // Control and Status Registers

    // Helper: Sign-extend a value from a specific bit width to 32 bits
    int32_t sign_extend(uint32_t value, int bits) {
        if (value & (1 << (bits - 1))) {
            return (int32_t)(value | (0xFFFFFFFF << bits));
        }
        return (int32_t)value;
    }

    // Private helper methods for instruction execution
    void execute_lui_auipc(uint8_t opcode, uint8_t rd, int32_t u_imm);
    void execute_jal(uint8_t rd, int32_t j_imm);
    void execute_jalr(uint8_t rd, uint8_t rs1, int32_t imm);
    void execute_branch(uint8_t funct3, uint8_t rs1, uint8_t rs2, int32_t b_imm);
    void execute_op_imm(uint8_t funct3, uint8_t rd, uint8_t rs1, int32_t imm, uint8_t funct7, uint8_t shamt);
    void execute_load(uint8_t funct3, uint8_t rd, uint8_t rs1, int32_t imm);
    void execute_store(uint8_t funct3, uint8_t rs1, uint8_t rs2, int32_t s_imm);
    void execute_op(uint8_t funct3, uint8_t rd, uint8_t rs1, uint8_t rs2, uint8_t funct7);
    void execute_system(uint8_t funct3, uint8_t rd, uint8_t rs1, uint32_t csr_addr);
};

#endif // CPU_HPP
