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

TEST_F(InstructionTest, ShiftInstructions) {
    // 1. addi x1, x0, 2          ; Shift amount
    // 2. addi x2, x0, 8          ; Value to shift
    // 3. sll x3, x2, x1          ; 8 << 2 = 32
    // 4. srl x4, x2, x1          ; 8 >> 2 = 2
    // 5. addi x5, x0, -100       ; Negative value for arithmetic shift
    // 6. sra x6, x5, x1          ; -100 >> 2 = -25
    // 7. slli x7, x2, 3          ; 8 << 3 = 64
    // 8. srli x8, x2, 1          ; 8 >> 1 = 4
    // 9. srai x9, x5, 3          ; -100 >> 3 = -13
    std::vector<uint32_t> program = {
        0x00200093,
        0x00800113,
        0x001111B3,
        0x00115233,
        0xF9C00293,
        0x4012D333,
        0x00311393,
        0x00115413,
        0x4032D493
    };
    load_and_run(program);

    ASSERT_EQ(cpu.get_reg(3), 32u);
    ASSERT_EQ(cpu.get_reg(4), 2u);
    ASSERT_EQ(cpu.get_reg(6), (uint32_t)-25);
    ASSERT_EQ(cpu.get_reg(7), 64u);
    ASSERT_EQ(cpu.get_reg(8), 4u);
    ASSERT_EQ(cpu.get_reg(9), (uint32_t)-13);
}
