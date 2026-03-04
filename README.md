# Simple RISC-V Pipelined Emulator

A high-performance, 5-stage pipelined RISC-V (RV32I) CPU emulator written in modern C++. This project demonstrates core computer architecture concepts including pipeline orchestration, hazard resolution, memory-mapped I/O, and cache hierarchy.

## 🚀 Key Features

*   **Instruction Set:** Full support for the RISC-V **RV32I** Base Integer ISA.
*   **Pipelined Architecture:** Implements a classic **5-stage pipeline** (Fetch, Decode, Execute, Memory, Write-back).
*   **Hazard Handling:**
    *   **Data Hazards:** Full data forwarding unit and load-use stalling logic.
    *   **Control Hazards:** Pipeline flushing mechanism for taken branches and jumps.
*   **Memory Hierarchy:** Integrated **Direct-Mapped L1 Data Cache** with hit/miss performance tracking.
*   **System Level:**
    *   Support for **Control and Status Registers (CSRs)** (e.g., `mstatus`, `mepc`, `mtvec`).
    *   Trap/Exception mechanism with `ecall` and `mret` support.
    *   **Memory-Mapped I/O (MMIO)** featuring a virtual UART for console output.
*   **Performance Monitoring:** Real-time tracking of clock cycles (`mcycle`), retired instructions (`minstret`), IPC, and cache hit rates.
*   **Testing:** Comprehensive unit test suite powered by **Google Test**.

## 🛠 Architecture Overview

### Pipeline Stages
1.  **IF (Instruction Fetch):** Retrieves 32-bit instructions from memory and manages the Program Counter.
2.  **ID (Instruction Decode):** Decodes opcodes, reads registers, and generates control signals.
3.  **EX (Execute):** Performs ALU operations, calculates branch targets, and handles data forwarding.
4.  **MEM (Memory Access):** Handles load/store operations and L1 cache lookups.
5.  **WB (Write-back):** Retires instructions and updates the architectural register file.

### Memory Map
*   **RAM:** `0x00000000` - `0x000FFFFF` (1MB default)
*   **UART MMIO:** `0x10000000` (Transmitter Holding Register)

## 📦 Getting Started

### Prerequisites
*   C++17 compatible compiler (e.g., `g++` or `clang++`)
*   `make` build utility

### Build & Test
```bash
# Clone the repository (including submodules for Google Test)
git clone --recursive https://github.com/yourusername/Simple-CPU-Emulator.git
cd Simple-CPU-Emulator

# Build the emulator and the test runner
make

# Run the unit test suite
make test
```

### Running a Binary
The emulator accepts raw binary files. You can run it using:
```bash
./bin/emulator path/to/your/program.bin
```

## 📊 Performance Reporting
At the end of execution, the emulator provides a detailed architectural summary:
```text
--- Execution Summary ---
Total Cycles:      125
Instructions:      84
IPC:               0.67
Cache Hits:        42
Cache Misses:      12
Cache Hit Rate:    77.78%
-------------------------
```

## 🧪 Development & Testing
This project follows a test-driven approach. Each instruction group and architectural feature (forwarding, flushes, CSRs) is verified using formal unit tests in the `tests/` directory.

---
*Created as a demonstration of Computer Architecture and Systems Programming expertise.*
