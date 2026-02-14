#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <array>

class CPU {
public:
    CPU();

    // Reset the CPU state
    void reset();

    // Debug: Print register contents
    void dump_registers() const;

private:
    std::array<uint32_t, 32> regs; // x0-x31
    uint32_t pc;                   // Program Counter
};

#endif // CPU_HPP
