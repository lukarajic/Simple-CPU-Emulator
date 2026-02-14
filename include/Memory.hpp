#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstdint>
#include <vector>

class Memory {
public:
    // Initialize memory with a specific size (default 1MB)
    Memory(uint32_t size = 1024 * 1024);

    // Read a 32-bit word from memory
    uint32_t read32(uint32_t address) const;

    // Write a 32-bit word to memory
    void write32(uint32_t address, uint32_t value);

    // Load a program into memory starting at an offset
    void load_program(const std::vector<uint32_t>& program, uint32_t start_address = 0);

private:
    std::vector<uint8_t> mem;
};

#endif // MEMORY_HPP
