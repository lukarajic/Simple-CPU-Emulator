#include <iostream>
#include "CPU.hpp"
#include "Memory.hpp"

int main() {
    std::cout << "Simple CPU Emulator Initialized" << std::endl;

    Memory mem(1024 * 1024); // 1MB Memory
    CPU cpu(mem);

    // Load program into memory
    // 1. addi x1, x0, 10  -> 0x00a00093
    // 2. addi x2, x1, -5  -> 0xffb08113
    std::vector<uint32_t> program = {
        0x00a00093,
        0xffb08113
    };
    mem.load_program(program);

    std::cout << "Running program..." << std::endl;
    cpu.step(); // Execute addi x1, x0, 10
    cpu.step(); // Execute addi x2, x1, -5

    cpu.dump_registers();

    return 0;
}
