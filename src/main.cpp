#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include "CPU.hpp"
#include "Memory.hpp"

void print_summary(const CPU& cpu) {
    uint32_t cycles = cpu.get_csr(CPU::CSR_MCYCLE);
    uint32_t instret = cpu.get_csr(CPU::CSR_MINSTRET);
    double ipc = (cycles > 0) ? (double)instret / cycles : 0;

    std::cout << "\n--- Execution Summary ---" << std::endl;
    std::cout << "Total Cycles:      " << cycles << std::endl;
    std::cout << "Instructions:      " << instret << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "IPC:               " << ipc << std::endl;
    std::cout << "-------------------------" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <binary_file>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint32_t> program(size / 4);
    if (!file.read(reinterpret_cast<char*>(program.data()), size)) {
        std::cerr << "Error: Could not read file " << filename << std::endl;
        return 1;
    }

    Memory mem(1024 * 1024); // 1MB Memory
    mem.load_program(program);

    CPU cpu(mem);

    std::cout << "Starting execution of " << filename << "..." << std::endl;

    // Run the pipeline
    // For now, we'll run for a max number of cycles or until an error
    // In a real system, we'd wait for a 'halt' signal or ecall
    const uint32_t MAX_CYCLES = 100000;
    uint32_t current_cycle = 0;

    while (current_cycle < MAX_CYCLES) {
        cpu.clock();
        current_cycle++;

        // Basic exit condition: if we hit a sequence of 0s (uninitialized memory)
        // or a very high address. This is a simplification.
        if (cpu.fetch_pc() >= size + 16) { // +16 to allow pipeline to drain
            break;
        }
    }

    std::cout << "Execution finished." << std::endl;
    cpu.dump_registers();
    print_summary(cpu);

    return 0;
}
