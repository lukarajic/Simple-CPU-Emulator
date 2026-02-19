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

    // Read a 16-bit halfword from memory
    uint16_t read16(uint32_t address) const;

    // Read an 8-bit byte from memory
    uint8_t read8(uint32_t address) const;

    // Write a 32-bit word to memory
    void write32(uint32_t address, uint32_t value);

    // Write a 16-bit halfword to memory
    void write16(uint32_t address, uint16_t value);

    // Write an 8-bit byte to memory
    void write8(uint32_t address, uint8_t value);

    // Load a program into memory starting at an offset
    void load_program(const std::vector<uint32_t>& program, uint32_t start_address = 0);

private:
    std::vector<uint8_t> mem;
};

#endif // MEMORY_HPP
