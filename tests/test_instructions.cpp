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

TEST_F(InstructionTest, RTypeArithmetic) {
    // 1. addi x1, x0, 15
    // 2. addi x2, x0, 10
    // 3. add x3, x1, x2   ; 15 + 10 = 25
    // 4. sub x4, x1, x2   ; 15 - 10 = 5
    // 5. xor x5, x1, x2   ; 15 ^ 10 = 5
    // 6. or  x6, x1, x2   ; 15 | 10 = 15
    // 7. and x7, x1, x2   ; 15 & 10 = 10
    std::vector<uint32_t> program = {
        0x00F00093,
        0x00A00113,
        0x002081B3,
        0x40208233,
        0x0020C2B3,
        0x0020E333,
        0x0020F3B3
    };
    load_and_run(program);

    ASSERT_EQ(cpu.get_reg(3), 25u);
    ASSERT_EQ(cpu.get_reg(4), 5u);
    ASSERT_EQ(cpu.get_reg(5), 5u);
    ASSERT_EQ(cpu.get_reg(6), 15u);
    ASSERT_EQ(cpu.get_reg(7), 10u);
}

TEST_F(InstructionTest, ITypeLogical) {
    // 1. addi x10, x0, 0b1010
    // 2. andi x11, x10, 0b1100  ; 10 & 12 = 8
    // 3. ori  x12, x10, 0b1100  ; 10 | 12 = 14
    // 4. xori x13, x10, 0b1100  ; 10 ^ 12 = 6
    std::vector<uint32_t> program = {
        0x00A00513,
        0x00C57593,
        0x00C56613,
        0x00C54693
    };
    load_and_run(program);

    ASSERT_EQ(cpu.get_reg(11), 8u);
    ASSERT_EQ(cpu.get_reg(12), 14u);
    ASSERT_EQ(cpu.get_reg(13), 6u);
}
