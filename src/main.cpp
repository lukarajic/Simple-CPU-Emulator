#include <iostream>
#include "CPU.hpp"
#include "Memory.hpp"

int main() {
    std::cout << "Simple CPU Emulator Initialized" << std::endl;

    Memory mem(1024 * 1024); // 1MB Memory
    CPU cpu(mem);

    // Basic Memory Test
    uint32_t test_addr = 0x100;
    uint32_t test_val = 0xDEADBEEF;
    mem.write32(test_addr, test_val);
    
    // Test Fetch
    mem.write32(0, 0x12345678); // Write dummy instruction at address 0
    uint32_t instr = cpu.fetch();

    std::cout << "Fetch Test: fetched 0x" << std::hex << instr << " from PC 0" << std::endl;
    cpu.dump_registers();

    return 0;
}
