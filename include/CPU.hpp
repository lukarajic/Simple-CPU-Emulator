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

    // Debug: Print register contents
    void dump_registers() const;

private:
    std::array<uint32_t, 32> regs; // x0-x31
    uint32_t pc;                   // Program Counter
    Memory& mem;                   // Reference to system memory
};

#endif // CPU_HPP
