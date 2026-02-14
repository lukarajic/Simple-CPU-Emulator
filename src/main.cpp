#include <iostream>
#include "CPU.hpp"
#include "Memory.hpp"

int main() {
    std::cout << "Simple CPU Emulator Initialized" << std::endl;

    CPU cpu;
    Memory mem(1024 * 1024); // 1MB Memory

    cpu.dump_registers();

    // Basic Memory Test
    uint32_t test_addr = 0x100;
    uint32_t test_val = 0xDEADBEEF;
    mem.write32(test_addr, test_val);
    uint32_t read_val = mem.read32(test_addr);

    std::cout << "Memory Test: wrote 0x" << std::hex << test_val 
              << " to 0x" << test_addr 
              << ", read back 0x" << read_val << std::endl;

    return 0;
}
