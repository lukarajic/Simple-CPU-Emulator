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
        cpu.reset(); // Reset CPU state before running each program
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

TEST_F(InstructionTest, LoadStoreInstructions) {
    // Test Load instructions
    // Store 0x11223344 at address 0x100
    mem.write32(0x100, 0x11223344u);

    std::vector<uint32_t> load_program = {
        0x10000093, // addi x1, x0, 0x100 (sets x1 = 0x100 directly)
        0x0000A103, // lw x2, 0(x1)
        0x00009183, // lh x3, 0(x1)
        0x0000D203, // lhu x4, 0(x1)
        0x00008283, // lb x5, 0(x1)
        0x0000C303  // lbu x6, 0(x1)
    };
    load_and_run(load_program);

    ASSERT_EQ(cpu.get_reg(2), 0x11223344u);
    ASSERT_EQ(cpu.get_reg(3), 0x00003344u); // 0x3344 as signed 16-bit is positive
    ASSERT_EQ(cpu.get_reg(4), 0x00003344u);
    ASSERT_EQ(cpu.get_reg(5), 0x00000044u); // 0x44 as signed 8-bit is positive
    ASSERT_EQ(cpu.get_reg(6), 0x00000044u);

    // Test Store instructions
    // Set x2 to 0x12345678 and x1 to 0x200
    // Then store and load back to verify
    std::vector<uint32_t> store_program = {
        0x12345137, // lui x2, 0x12345  ; x2 = 0x12345000
        0x67810113, // addi x2, x2, 0x678 ; x2 = 0x12345678
        0x20000093, // addi x1, x0, 0x200 ; x1 = 0x200

        0x0020A023, // sw x2, 0(x1)   ; mem[0x200] = 0x12345678
        0x00209223, // sh x2, 4(x1)   ; mem[0x204] = 0x5678
        0x00208423, // sb x2, 8(x1)   ; mem[0x208] = 0x78

        0x0000A183, // lw x3, 0(x1)   ; x3 = mem[0x200] = 0x12345678
        0x0040A203, // lw x4, 4(x1)   ; x4 = mem[0x204] = 0x00005678
        0x0080A283  // lw x5, 8(x1)   ; x5 = mem[0x208] = 0x00000078
    };
    load_and_run(store_program);

    ASSERT_EQ(cpu.get_reg(3), 0x12345678u);
    ASSERT_EQ(cpu.get_reg(4), 0x00005678u);
    ASSERT_EQ(cpu.get_reg(5), 0x00000078u);
}

TEST_F(InstructionTest, BranchInstructions) {
    // 1. addi x1, x0, 10
    // 2. addi x2, x0, 20
    // 3. beq x1, x2, 8      ; Should NOT take branch (10 != 20)
    // 4. addi x3, x0, 1     ; Should execute
    // 5. beq x1, x1, 8      ; Should take branch (10 == 10)
    // 6. addi x4, x0, 1     ; Should be skipped
    // 7. addi x5, x0, 2     ; Target of second BEQ
    // 8. blt x1, x2, 8      ; Should take branch (10 < 20)
    // 9. addi x6, x0, 1     ; Should be skipped
    // 10. addi x7, x0, 3    ; Target of BLT
    std::vector<uint32_t> program = {
        0x00a00093,
        0x01400113,
        0x00208463,
        0x00100193,
        0x00108463,
        0x00100213,
        0x00200293,
        0x0020c463,
        0x00100313,
        0x00300393
    };
    // Run for a fixed number of steps since branches change flow
    cpu.reset();
    mem.load_program(program);
    for (int i = 0; i < 10; ++i) {
        if (cpu.fetch_pc() >= program.size() * 4) break; 
        cpu.step();
    }

    ASSERT_EQ(cpu.get_reg(3), 1u); // x3 should be 1 (first BEQ not taken)
    ASSERT_EQ(cpu.get_reg(4), 0u); // x4 should be 0 (second BEQ taken)
    ASSERT_EQ(cpu.get_reg(5), 2u); // x5 should be 2 (target of second BEQ)
    ASSERT_EQ(cpu.get_reg(6), 0u); // x6 should be 0 (BLT taken)
    ASSERT_EQ(cpu.get_reg(7), 3u); // x7 should be 3 (target of BLT)
}

TEST_F(InstructionTest, JumpInstructions) {
    // 1. jal x1, 8          ; Jump to PC+8, store return address (PC+4) in x1
    // 2. addi x2, x0, 1     ; Should be skipped
    // 3. addi x3, x0, 2     ; Target of JAL
    // 4. jalr x4, x1, 0     ; Jump to address in x1 (which is PC of instruction 2)
    // 5. addi x5, x0, 3     ; Target of JALR (since x1 points here)
    std::vector<uint32_t> program = {
        0x008000ef,
        0x00100113,
        0x00200193,
        0x00008267,
        0x00300293
    };

    cpu.reset();
    mem.load_program(program);
    // Step 1: JAL
    cpu.step(); 
    ASSERT_EQ(cpu.fetch_pc(), 8u); // Should be at instruction 3
    ASSERT_EQ(cpu.get_reg(1), 4u); // Return address should be 4 (instruction 2)

    // Step 2: Instruction 3 (ADDI x3, x0, 2)
    cpu.step();
    ASSERT_EQ(cpu.get_reg(3), 2u);

    // Step 3: JALR (Jump to address in x1 = 4)
    cpu.step();
    ASSERT_EQ(cpu.fetch_pc(), 4u); // Should be at instruction 2

    // Step 4: Instruction 2 (ADDI x2, x0, 1)
    cpu.step();
    ASSERT_EQ(cpu.get_reg(2), 1u);
}

TEST_F(InstructionTest, CSRInstructions) {
    // 1. addi x1, x0, 0x5
    // 2. addi x2, x0, 0xC
    // 3. csrrw x3, mstatus, x1  ; x3 = mstatus, mstatus = 5
    // 4. csrrs x4, mstatus, x2  ; x4 = mstatus (5), mstatus = 5 | 12 = 13
    // 5. csrrc x5, mstatus, x1  ; x5 = mstatus (13), mstatus = 13 & ~5 = 8
    std::vector<uint32_t> program = {
        0x00500093,
        0x00C00113,
        0x300091F3, // CSRRW: csr=mstatus(0x300), rd=x3, rs1=x1
        0x30012273, // CSRRS: csr=mstatus(0x300), rd=x4, rs1=x2
        0x3000B2F3  // CSRRC: csr=mstatus(0x300), rd=x5, rs1=x1
    };
    load_and_run(program);

    ASSERT_EQ(cpu.get_reg(3), 0u);          // Initial mstatus was 0
    ASSERT_EQ(cpu.get_reg(4), 5u);          // mstatus was 5 before CSRRS
    ASSERT_EQ(cpu.get_reg(5), 13u);         // mstatus was 13 before CSRRC
    ASSERT_EQ(cpu.get_csr(CPU::CSR_MSTATUS), 8u); // Final mstatus is 8
}
