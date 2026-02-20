#include <iostream>
#include "CPU.hpp"
#include "Memory.hpp"

int main() {
    std::cout << "Simple CPU Emulator Initialized" << std::endl;

    Memory mem(1024 * 1024); // 1MB Memory
    CPU cpu(mem);

    // Load program into memory
    // Tests for BRANCH instructions
    // 1. addi x1, x0, 10
    // 2. addi x2, x0, 10
    // 3. beq x1, x2, 8      ; Should jump to instruction 5
    // 4. addi x3, x0, 1     ; Should be skipped
    // 5. addi x4, x0, 2
    // 6. bne x1, x0, 8      ; Should jump to instruction 8
    // 7. addi x5, x0, 3     ; Should be skipped
    // 8. addi x6, x0, 4
    std::vector<uint32_t> program = {
        0x00a00093,
        0x00a00113,
        0x00208463,
        0x00100193,
        0x00200213,
        0x00009463,
        0x00300293,
        0x00400313
    };
    mem.load_program(program);

    std::cout << "Running program..." << std::endl;
    // We don't know exactly how many steps because of jumps
    // We'll run for a fixed number of steps that is enough
    for (int i = 0; i < 10; ++i) {
        if (cpu.fetch_pc() >= program.size() * 4) break; 
        cpu.step();
    }

    cpu.dump_registers();

    return 0;
}
