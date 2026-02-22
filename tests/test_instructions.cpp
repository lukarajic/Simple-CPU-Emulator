#include <gtest/gtest.h>
#include "CPU.hpp"
#include "Memory.hpp"
#include <vector>

class InstructionTest : public ::testing::Test {
protected:
    Memory mem;
    CPU cpu;

    InstructionTest() : mem(1024 * 1024), cpu(mem) {}

    void load_and_run(const std::vector<uint32_t>& program) {
        mem.load_program(program);
        for (size_t i = 0; i < program.size(); ++i) {
            cpu.step();
        }
    }
};

TEST_F(InstructionTest, ADDI) {
    // 1. addi x1, x0, 10
    // 2. addi x2, x1, -5
    // 3. addi x3, x0, 0
    std::vector<uint32_t> program = {
        0x00A00093,
        0xFFB08113,
        0x00000193
    };
    load_and_run(program);

    ASSERT_EQ(cpu.get_reg(1), 10u);
    ASSERT_EQ(cpu.get_reg(2), 5u);
    ASSERT_EQ(cpu.get_reg(3), 0u);
}

TEST_F(InstructionTest, LUI) {
    // 1. lui x1, 0x12345
    // 2. addi x1, x1, 0x678
    std::vector<uint32_t> program = {
        0x123450B7,
        0x67808093
    };
    load_and_run(program);

    ASSERT_EQ(cpu.get_reg(1), 0x12345678u);
}
