#include "hart.h"

#include "local-include/decode.h"

static inline sword_t sgn(word_t x) {
  return *reinterpret_cast<sword_t *>(&x);
}

void Hart::step() {
  inst_t inst = inst_fetch();

  inst_t c_op = c_opcode(inst);
  Assert(c_op == 0b11, "C extension is not supported yet!");

  inst_t op = opcode(inst);

  const vaddr_t pc = get_pc();
  vaddr_t dnpc = pc + 4;

  if (op == 0b01101) { // LUI
    gpr_write(rd(inst), immU(inst));
  } else if (op == 0b00101) { // AUIPC
    gpr_write(rd(inst), pc + immU(inst));
  } else if (op == 0b11011) { // JAL
    dnpc = pc + immJ(inst);
    gpr_write(rd(inst), pc + 4);
  } else if (op == 0b11001) { // JALR
    dnpc = (gpr_read(rs1(inst)) + immI(inst)) & ~1;
    gpr_write(rd(inst), pc + 4);
  } else if (op == 0b11000) { // BRANCH
    word_t src1 = gpr_read(rs1(inst));
    word_t src2 = gpr_read(rs2(inst));

    bool jump_en = false;
    switch (funct3(inst)) {
      case 0b000: jump_en = (src1 == src2); break;
      case 0b001: jump_en = (src1 != src2); break;
      case 0b100: jump_en = (sgn(src1) < sgn(src2)); break;
      case 0b101: jump_en = (sgn(src1) >= sgn(src2)); break;
      case 0b110: jump_en = (src1 < src2); break;
      case 0b111: jump_en = (src1 >= src2); break;
      default: if constexpr (rt_check) assert(0);
    }

    if (jump_en) {
      dnpc = pc + immB(inst);
    }
  } else if (op == 0b00000) { // LOAD
    vaddr_t addr = gpr_read(rs1(inst)) + immI(inst);
    word_t data;

    switch (funct3(inst)) {
      case 0b000: data = sext<8>(vaddr_read(addr, 1)); break;
      case 0b001: data = sext<16>(vaddr_read(addr, 2)); break;
      case 0b010: data = sext<32>(vaddr_read(addr, 4)); break;
      case 0b100: data = vaddr_read(addr, 1); break;
      case 0b101: data = vaddr_read(addr, 2); break;
      default: if constexpr (rt_check) assert(0);
    }

    gpr_write(rd(inst), data);
  } else if (op == 0b01000) { // STORE
    vaddr_t addr = gpr_read(rs1(inst)) + immS(inst);
    word_t data = gpr_read(rs2(inst));

    switch (funct3(inst)) {
      case 0b000: vaddr_write(addr, 1, data); break;
      case 0b001: vaddr_write(addr, 2, data); break;
      case 0b010: vaddr_write(addr, 4, data); break;
      default: if constexpr (rt_check) assert(0);
    }
  } else if (op == 0b00100) { // CALRI
    word_t src1 = gpr_read(rs1(inst));
    word_t imm = immI(inst);
    word_t res;

    switch (funct3(inst)) {
      case 0b000: res = src1 + imm; break;
      case 0b010: res = sgn(src1) < sgn(imm); break;
      case 0b011: res = src1 < imm; break;
      case 0b100: res = src1 ^ imm; break;
      case 0b110: res = src1 | imm; break;
      case 0b111: res = src1 & imm; break;
      case 0b001:
        if constexpr (rt_check) assert(funct7(inst) == 0);
        res = src1 << imm;
      break;
      case 0b101:
        switch (funct7(inst)) {
          case 0b0000000: res = src1 >> imm; break;
          case 0b0100000: res = sgn(src1) >> (imm & bit_mask(5)); break;
          default: if constexpr (rt_check) assert(0);
        }
      break;
      default: if constexpr (rt_check) assert(0);
    }

    gpr_write(rd(inst), res);
  } else if (op == 0b01100) { // CALRR
    constexpr word_t WORD_MIN = static_cast<word_t>(1) << (xlen - 1);
    constexpr word_t ALL_ONE = static_cast<word_t>(-1);
    inst_t f7 = funct7(inst), f3 = funct3(inst);
    word_t src1 = gpr_read(rs1(inst)), src2 = gpr_read(rs2(inst));
    word_t res;
    if (f7 & 1) { // RV32M
      if constexpr (rt_check) assert(f7 == 1);
      switch (f3) {
        case 0b000: res = src1 * src2; break;
        case 0b001: res = (static_cast<int64_t>(sgn(src1)) * static_cast<int64_t>(sgn(src2))) >> 32; break;
        case 0b010: res = (static_cast<int64_t>(sgn(src1)) * static_cast<int64_t>(src2)) >> 32; break;
        case 0b011: res = (static_cast<uint64_t>(src1) * static_cast<uint64_t>(src2)) >> 32; break;
        case 0b100: // div
          if (src1 == WORD_MIN && src2 == ALL_ONE) res = src1;
          else if (src2) res = sgn(src1) / sgn(src2);
          else res = ALL_ONE;
        break;
        case 0b101: // divu
          if (src2) res = src1 / src2;
          else res = ALL_ONE;
        break;
        case 0b110: // rem
          if (src1 == WORD_MIN && src2 == ALL_ONE) res = 0;
          else if (src2) res = sgn(src1) % sgn(src2);
          else res = src1;
        break;
        case 0b111: // remu
          if (src2) res = src1 % src2;
          else res = src1;
        break;
        default: if constexpr (rt_check) assert(0);
      }
    } else {
      switch (f3) {
        case 0b000: // add sub
          switch (f7) {
            case 0b0000000: res = src1 + src2; break;
            case 0b0100000: res = src1 - src2; break;
            default: if constexpr (rt_check) assert(0);
          }
        break;
        case 0b001: // sll
          if constexpr (rt_check) assert(f7 == 0);
          res = src1 << (src2 & bit_mask(5));
        break;
        case 0b010: // slt
          if constexpr (rt_check) assert(f7 == 0);
          res = sgn(src1) < sgn(src2);
        break;
        case 0b011: // sltu
          if constexpr (rt_check) assert(f7 == 0);
          res = src1 < src2;
        break;
        case 0b100: // xor
          if constexpr (rt_check) assert(f7 == 0);
          res = src1 ^ src2;
        break;
        case 0b101: // srl sra
          switch (f7) {
            case 0b0000000: res = src1 >> (src2 & bit_mask(5)); break;
            case 0b0100000: res = sgn(src1) >> (src2 & bit_mask(5)); break;
            default: if constexpr (rt_check) assert(0);
          }
        break;
        case 0b110: // or
          if constexpr (rt_check) assert(f7 == 0);
          res = src1 | src2;
        break;
        case 0b111: // and
          if constexpr (rt_check) assert(f7 == 0);
          res = src1 & src2;
        break;
        default: if constexpr (rt_check) assert(0);
      }
    }

    gpr_write(rd(inst), res);
  } else if (op == 0b11100) { // SYS
    emu_state.set_state(GlobalState::ST_END);
    emu_state.set_ret(gpr_read(10));
    return;
  } else {
    if constexpr (rt_check) assert(0);
  }

  set_pc(dnpc);
}
