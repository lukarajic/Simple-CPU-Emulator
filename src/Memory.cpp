#include "Memory.hpp"
#include <iostream>
#include <cstring>

Memory::Memory(uint32_t size) {
    mem.resize(size, 0);
}

uint32_t Memory::read32(uint32_t address) const {
    if (address + 3 >= mem.size()) {
        std::cerr << "Memory read out of bounds at 0x" << std::hex << address << std::endl;
        return 0;
    }
    // Little-endian read
    return mem[address] | 
           (mem[address + 1] << 8) | 
           (mem[address + 2] << 16) | 
           (mem[address + 3] << 24);
}

uint16_t Memory::read16(uint32_t address) const {
    if (address + 1 >= mem.size()) {
        std::cerr << "Memory read16 out of bounds at 0x" << std::hex << address << std::endl;
        return 0;
    }
    // Little-endian read
    return mem[address] | 
           (mem[address + 1] << 8);
}

uint8_t Memory::read8(uint32_t address) const {
    if (address >= mem.size()) {
        std::cerr << "Memory read8 out of bounds at 0x" << std::hex << address << std::endl;
        return 0;
    }
    return mem[address];
}

void Memory::write32(uint32_t address, uint32_t value) {
    if (address + 3 >= mem.size()) {
        std::cerr << "Memory write out of bounds at 0x" << std::hex << address << std::endl;
        return;
    }
    // Little-endian write
    mem[address]     = value & 0xFF;
    mem[address + 1] = (value >> 8) & 0xFF;
    mem[address + 2] = (value >> 16) & 0xFF;
    mem[address + 3] = (value >> 24) & 0xFF;
}

void Memory::write16(uint32_t address, uint16_t value) {
    if (address + 1 >= mem.size()) {
        std::cerr << "Memory write16 out of bounds at 0x" << std::hex << address << std::endl;
        return;
    }
    // Little-endian write
    mem[address]     = value & 0xFF;
    mem[address + 1] = (value >> 8) & 0xFF;
}

void Memory::write8(uint32_t address, uint8_t value) {
    if (address >= mem.size()) {
        std::cerr << "Memory write8 out of bounds at 0x" << std::hex << address << std::endl;
        return;
    }
    mem[address] = value;
}

void Memory::load_program(const std::vector<uint32_t>& program, uint32_t start_address) {
    for (size_t i = 0; i < program.size(); ++i) {
        write32(start_address + (i * 4), program[i]);
    }
}
