#include <iostream>
#include "CPU.hpp"

int main() {
    std::cout << "Simple CPU Emulator Initialized" << std::endl;

    CPU cpu;
    cpu.dump_registers();

    return 0;
}
