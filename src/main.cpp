#include <iostream>
#include "CPU.hpp"
#include "Memory.hpp"

int main() {
    std::cout << "Simple CPU Emulator Initialized" << std::endl;

    Memory mem(1024 * 1024); // 1MB Memory
    CPU cpu(mem);

    // Load program into memory
    // 1. addi x1, x0, 10  -> 0x00a00093
    // 2. addi x2, x0, 20  -> 0x01400113
    // 3. add  x3, x1, x2  -> 0x002081b3 (x3 = 10 + 20 = 30)
    // 4. sub  x4, x2, x1  -> 0x40110233 (x4 = 20 - 10 = 10)
    std::vector<uint32_t> program = {
        0x00a00093,
        0x01400113,
        0x002081b3,
        0x40110233
    };
    mem.load_program(program);

    std::cout << "Running program..." << std::endl;
    cpu.step(); 
    cpu.step(); 
    cpu.step(); 
    cpu.step(); 

    cpu.dump_registers();

    return 0;
}
