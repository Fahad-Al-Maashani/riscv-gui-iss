#include "cpu.hpp"
#include <sstream>
#include <iomanip>
#include <stdexcept>

RV32ISimulator::RV32ISimulator(size_t memory_size) 
    : memory_(memory_size, 0), pc_(0), instructions_retired_(0) {
    regs_[0] = 0;
}

void RV32ISimulator::reset() {
    regs_.fill(0);
    regs_[0] = 0;
    pc_ = 0;
    instructions_retired_ = 0;
    std::fill(memory_.begin(), memory_.end(), 0);
}

void RV32ISimulator::load_binary(const std::vector<uint8_t>& binary, uint32_t entry_point) {
    if (entry_point + binary.size() > memory_.size()) {
        throw std::runtime_error("Binary exceeds memory boundaries.");
    }
    for (size_t i = 0; i < binary.size(); ++i) {
        memory_[entry_point + i] = binary[i];
    }
    pc_ = entry_point;
}

uint32_t RV32ISimulator::fetch() {
    if (pc_ + 3 >= memory_.size()) return 0;
    return static_cast<uint32_t>(memory_[pc_]) |
           (static_cast<uint32_t>(memory_[pc_ + 1]) << 8) |
           (static_cast<uint32_t>(memory_[pc_ + 2]) << 16) |
           (static_cast<uint32_t>(memory_[pc_ + 3]) << 24);
}

InstructionFields RV32ISimulator::decode(uint32_t inst) const {
    InstructionFields f{};
    f.opcode = inst & 0x7F;
    f.rd     = (inst >> 7) & 0x1F;
    f.funct3 = (inst >> 12) & 0x7;
    f.rs1    = (inst >> 15) & 0x1F;
    f.rs2    = (inst >> 20) & 0x1F;
    f.funct7 = (inst >> 25) & 0x7F;

    f.imm_i  = static_cast<int32_t>(inst) >> 20;
    f.imm_s  = static_cast<int32_t>((inst & 0xFE000000) | ((inst & 0x00000F80) << 13)) >> 20;
    f.imm_b  = static_cast<int32_t>((inst & 0x80000000) | ((inst & 0x80) << 23) | ((inst & 0x7E000000) >> 20) | ((inst & 0x0F00) >> 4)) >> 19;
    f.imm_u  = static_cast<int32_t>(inst & 0xFFFFF000);
    f.imm_j  = static_cast<int32_t>((inst & 0x80000000) | ((inst & 0x000FF000) << 9) | ((inst & 0x00100000) << 2) | ((inst & 0x7FE00000) >> 20)) >> 11;
    return f;
}

std::string RV32ISimulator::disassemble(uint32_t inst, uint32_t addr) const {
    InstructionFields f = decode(inst);
    std::stringstream ss;
    
    if (inst == 0x00000073) return "ecall";
    if (inst == 0x00000000) return "nop";

    switch (f.opcode) {
        case 0x33:
            if (f.funct7 == 0x01) {
                switch(f.funct3) {
                    case 0x0: ss << "mul x" << f.rd << ", x" << f.rs1 << ", x" << f.rs2; break;
                    case 0x4: ss << "div x" << f.rd << ", x" << f.rs1 << ", x" << f.rs2; break;
                    case 0x6: ss << "rem x" << f.rd << ", x" << f.rs1 << ", x" << f.rs2; break;
                    default: ss << "unknown_m";
                }
            } else {
                switch (f.funct3) {
                    case 0x0: ss << (f.funct7 == 0x20 ? "sub " : "add "); break;
                    case 0x1: ss << "sll "; break;
                    case 0x2: ss << "slt "; break;
                    case 0x4: ss << "xor "; break;
                    case 0x5: ss << (f.funct7 == 0x20 ? "sra " : "srl "); break;
                    case 0x6: ss << "or  "; break;
                    case 0x7: ss << "and "; break;
                    default: ss << "unknown";
                }
                ss << "x" << f.rd << ", x" << f.rs1 << ", x" << f.rs2;
            }
            break;
        case 0x13: ss << "addi x" << f.rd << ", x" << f.rs1 << ", " << f.imm_i; break;
        case 0x37: ss << "lui  x" << f.rd << ", 0x" << std::hex << (f.imm_u >> 12); break;
        case 0x6F: ss << "jal  x" << f.rd << ", " << (addr + f.imm_j); break;
        case 0x03: ss << "lw   x" << f.rd << ", " << f.imm_i << "(x" << f.rs1 << ")"; break;
        case 0x23: ss << "sw   x" << f.rs2 << ", " << f.imm_s << "(x" << f.rs1 << ")"; break;
        default: ss << "unknown_op";
    }
    return ss.str();
}

void RV32ISimulator::execute(uint32_t instruction) {
    InstructionFields f = decode(instruction);
    regs_[0] = 0;
    uint32_t next_pc = pc_ + 4;

    switch (f.opcode) {
        case 0x33:
            if (f.funct7 == 0x01) {
                int32_t rs1_s = static_cast<int32_t>(regs_[f.rs1]);
                int32_t rs2_s = static_cast<int32_t>(regs_[f.rs2]);
                switch (f.funct3) {
                    case 0x0: regs_[f.rd] = rs1_s * rs2_s; break;
                    case 0x4: if (rs2_s != 0) regs_[f.rd] = rs1_s / rs2_s; break;
                    case 0x6: if (rs2_s != 0) regs_[f.rd] = rs1_s % rs2_s; break;
                }
            } else {
                switch (f.funct3) {
                    case 0x0: regs_[f.rd] = (f.funct7 == 0x20) ? regs_[f.rs1] - regs_[f.rs2] : regs_[f.rs1] + regs_[f.rs2]; break;
                    case 0x1: regs_[f.rd] = regs_[f.rs1] << (regs_[f.rs2] & 0x1F); break;
                    case 0x4: regs_[f.rd] = regs_[f.rs1] ^ regs_[f.rs2]; break;
                    case 0x6: regs_[f.rd] = regs_[f.rs1] | regs_[f.rs2]; break;
                    case 0x7: regs_[f.rd] = regs_[f.rs1] & regs_[f.rs2]; break;
                }
            }
            break;
        case 0x13: regs_[f.rd] = regs_[f.rs1] + f.imm_i; break;
        case 0x37: regs_[f.rd] = f.imm_u; break;
        case 0x6F: regs_[f.rd] = pc_ + 4; next_pc = pc_ + f.imm_j; break;
        case 0x03: {
            uint32_t addr = regs_[f.rs1] + f.imm_i;
            if (f.funct3 == 0x2) {
                regs_[f.rd] = static_cast<uint32_t>(memory_[addr]) | (static_cast<uint32_t>(memory_[addr+1]) << 8) |
                              (static_cast<uint32_t>(memory_[addr+2]) << 16) | (static_cast<uint32_t>(memory_[addr+3]) << 24);
            }
            break;
        }
        case 0x23: {
            uint32_t addr = regs_[f.rs1] + f.imm_s;
            if (f.funct3 == 0x2) {
                uint32_t val = regs_[f.rs2];
                memory_[addr] = val & 0xFF; memory_[addr+1] = (val >> 8) & 0xFF;
                memory_[addr+2] = (val >> 16) & 0xFF; memory_[addr+3] = (val >> 24) & 0xFF;
            }
            break;
        }
    }
    pc_ = next_pc;
    instructions_retired_++;
}

void RV32ISimulator::step() {
    uint32_t inst = fetch();
    if (inst != 0) execute(inst);
}
