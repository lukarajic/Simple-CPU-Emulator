#include <iostream>
#include "CPU.hpp"
#include "Memory.hpp"

int main() {
    std::cout << "Simple CPU Emulator Initialized" << std::endl;

    Memory mem(1024 * 1024); // 1MB Memory
    CPU cpu(mem);

    // Load program into memory
    // Tests for LUI and STORE instructions
    // 1. lui x2, 0x12345        ; x2 = 0x12345000
    // 2. addi x2, x2, 0x678     ; x2 = 0x12345678
    // 3. addi x1, x0, 0x100     ; x1 = 0x100
    // 4. sw x2, 0(x1)           ; mem[0x100] = 0x12345678
    // 5. sh x2, 4(x1)           ; mem[0x104] = 0x5678
    // 6. sb x2, 8(x1)           ; mem[0x108] = 0x78
    // 7. lw x3, 0(x1)           ; x3 = 0x12345678
    // 8. lw x4, 4(x1)           ; x4 = 0x00005678
    // 9. lw x5, 8(x1)           ; x5 = 0x00000078
    std::vector<uint32_t> program = {
        0x12345137,
        0x67810113,
        0x10000093,
        0x0020A023,
        0x00209223,
        0x00208423,
        0x0000A183,
        0x0040A203,
        0x0080A283
    };
    mem.load_program(program);

    std::cout << "Running program..." << std::endl;
    for (size_t i = 0; i < program.size(); ++i) {
        cpu.step();
    }

    cpu.dump_registers();

    return 0;
}
