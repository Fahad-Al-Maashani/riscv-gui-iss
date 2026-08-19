#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <string>

struct InstructionFields {
    uint32_t opcode;
    uint32_t rd;
    uint32_t funct3;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t funct7;
    int32_t  imm_i;
    int32_t  imm_s;
    int32_t  imm_b;
    int32_t  imm_u;
    int32_t  imm_j;
};

class RV32ISimulator {
public:
    explicit RV32ISimulator(size_t memory_size = 65536);
    ~RV32ISimulator() = default;

    void load_binary(const std::vector<uint8_t>& binary, uint32_t entry_point = 0x0);
    void step();
    void reset();

    uint32_t get_pc() const { return pc_; }
    uint32_t get_register(size_t idx) const { return regs_[idx]; }
    uint64_t get_instructions_retired() const { return instructions_retired_; }
    const std::vector<uint8_t>& get_memory() const { return memory_; }
    
    std::string disassemble(uint32_t inst, uint32_t addr) const;

private:
    std::array<uint32_t, 32> regs_{};
    std::vector<uint8_t> memory_;
    uint32_t pc_;
    uint64_t instructions_retired_;

    uint32_t fetch();
    InstructionFields decode(uint32_t instruction) const;
    void execute(uint32_t instruction);
};
