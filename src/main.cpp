#include <iostream>
#include "CPU.hpp"
#include "Memory.hpp"

int main() {
    std::cout << "Simple CPU Emulator Initialized" << std::endl;

    Memory mem(1024 * 1024); // 1MB Memory
    CPU cpu(mem);

    // Load program into memory
    // Tests for LOAD instructions
    // Store 0x11223344 at address 0x100
    // addi x1, x0, 0x100 ; x1 = 0x100
    //
    // 1. lw x2, 0(x1)   ; x2 = 0x11223344
    // 2. lh x3, 0(x1)   ; x3 = 0xFFFF3344 (signed extension of 0x3344)
    // 3. lhu x4, 0(x1)  ; x4 = 0x00003344 (unsigned extension of 0x3344)
    // 4. lb x5, 0(x1)   ; x5 = 0xFFFFFF44 (signed extension of 0x44)
    // 5. lbu x6, 0(x1)  ; x6 = 0x00000044 (unsigned extension of 0x44)
    std::vector<uint32_t> program = {
        0x10000093, // addi x1, x0, 0x100 (sets x1 = 0x100 directly)
        0x0000A103, // lw x2, 0(x1)   ; Corrected: rs1=1
        0x00009183, // lh x3, 0(x1)   ; Corrected: rs1=1
        0x0000D203, // lhu x4, 0(x1)  ; Corrected: rs1=1
        0x00008283, // lb x5, 0(x1)   ; Corrected: rs1=1
        0x0000C303  // lbu x6, 0(x1)  ; Corrected: rs1=1
    };
    mem.load_program(program);

    // Manually write the test data into memory at 0x100
    mem.write32(0x100, 0x11223344);

    std::cout << "Running program..." << std::endl;
    for (size_t i = 0; i < program.size(); ++i) {
        cpu.step();
    }

    cpu.dump_registers();

    return 0;
}
