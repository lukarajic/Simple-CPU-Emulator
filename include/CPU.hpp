#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <array>

class Memory; // Forward declaration

class CPU {
public:
    CPU(Memory& memory);

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

    // Get current PC
    uint32_t fetch_pc() const { return pc; }

private:
    std::array<uint32_t, 32> regs; // x0-x31
    uint32_t pc;                   // Program Counter
    Memory& mem;                   // Reference to system memory

    // Helper: Sign-extend a value from a specific bit width to 32 bits
    int32_t sign_extend(uint32_t value, int bits) {
        if (value & (1 << (bits - 1))) {
            return (int32_t)(value | (0xFFFFFFFF << bits));
        }
        return (int32_t)value;
    }
};

#endif // CPU_HPP
