#include <iostream>
#include "CPU.hpp"
#include "Memory.hpp"

int main() {
    std::cout << "Simple CPU Emulator Initialized" << std::endl;

    Memory mem(1024 * 1024); // 1MB Memory
    CPU cpu(mem);

    // Load program into memory
    // Tests for logical immediate and shift instructions
    // 1. ori x5, x0, 10           ; x5 = 10
    // 2. andi x6, x5, 6           ; x6 = 10 & 6 = 2
    // 3. xori x7, x6, 15          ; x7 = 2 ^ 15 = 13
    // 4. slli x8, x7, 2           ; x8 = 13 << 2 = 52
    // 5. srli x9, x8, 1           ; x9 = 52 >> 1 = 26
    // 6. addi x10, x0, -100       ; x10 = -100
    // 7. srai x11, x10, 2         ; x11 = -100 >> 2 = -25 (arithmetic)
    // 8. slti x12, x0, -1         ; x12 = (0 < -1) = 0
    // 9. sltiu x13, x0, 1         ; x13 = (0 < 1) = 1
    std::vector<uint32_t> program = {
        0x00A06293,
        0x0062F313,
        0x00F34393,
        0x00239413,
        0x00145493,
        0xF9C00513,
        0x40255593,
        0xFFF02613,
        0x00103693
    };
    mem.load_program(program);

    std::cout << "Running program..." << std::endl;
    for (size_t i = 0; i < program.size(); ++i) {
        cpu.step();
    }

    cpu.dump_registers();

    return 0;
}
