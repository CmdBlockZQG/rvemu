#include "hart.h"
#include "soc.h"

#include "local-include/decode.h"
#include "local-include/misc.h"

static inline sword_t sgn(word_t x) {
  return *reinterpret_cast<sword_t *>(&x);
}

void Hart::do_inst() {
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
    word_t data = 0;

    switch (funct3(inst)) {
      case 0b000: data = sext<8>(vaddr_load(addr, 1)); break;
      case 0b001: data = sext<16>(vaddr_load(addr, 2)); break;
      case 0b010: data = sext<32>(vaddr_load(addr, 4)); break;
      case 0b100: data = vaddr_load(addr, 1); break;
      case 0b101: data = vaddr_load(addr, 2); break;
      default: if constexpr (rt_check) assert(0);
    }

    gpr_write(rd(inst), data);
  } else if (op == 0b01000) { // STORE
    vaddr_t addr = gpr_read(rs1(inst)) + immS(inst);
    word_t data = gpr_read(rs2(inst));

    switch (funct3(inst)) {
      case 0b000: vaddr_store(addr, 1, data); break;
      case 0b001: vaddr_store(addr, 2, data); break;
      case 0b010: vaddr_store(addr, 4, data); break;
      default: if constexpr (rt_check) assert(0);
    }
  } else if (op == 0b00100) { // CALRI
    word_t src1 = gpr_read(rs1(inst));
    word_t imm = immI(inst);
    word_t res = 0;

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
    word_t res = 0;
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
    inst_t f3 = funct3(inst);
    if (f3) { // zicsr
      const word_t csr_addr = funct12(inst);
      // TVM=1，在S模式操作satp会触发非法指令异常
      if (priv == PRIV_S && csr_addr == 0x180 && mstatus_TVM) {
        throw Exception {2, inst};
      }
      word_t csr_rdata;
      try {
        csr_rdata = csr_read(csr_addr);
      } catch (...) {
        // 非法CSR读取，非法指令异常
        throw Exception {2, inst};
      }

      word_t csr_wdata = 0;
      word_t opnd;
      if (f3 & 0b100) opnd = zimm(inst);
      else opnd = gpr_read(rs1(inst));
      switch (f3 & 0b11) {
        case 0b01: csr_wdata = opnd; break;
        case 0b10: csr_wdata = csr_rdata | opnd; break;
        case 0b11: csr_wdata = csr_rdata & ~opnd; break;
        default: if constexpr (rt_check) assert(0);
      }

      if (csr_wdata != csr_rdata) { // CSR写入
        try {
          csr_write(csr_addr, csr_wdata);
        } catch (...) {
          // 非法CSR写入，非法指令异常
          throw Exception {2, inst};
        }
      }
      
      gpr_write(rd(inst), csr_rdata);
    } else {
      if ((inst & ~(bit_mask(10) << 15)) == 0x12000073) { // sfence.vma
        // TVM=1，在S模式执行SFENCE.VMA会触发非法指令异常
        if (priv == PRIV_S && mstatus_TVM) throw Exception {2, inst};
      } else switch (inst) {
        // ecall
        case 0x00000073: throw Exception {8 | priv, 0};
        // ebreak
        case 0x00100073: throw Exception {3, 0};
        // mret
        case 0x30200073:
          // 若TSR=1，在S模式执行SRET会触发非法指令异常
          if (mstatus_TSR && priv == PRIV_S) throw Exception {2, inst};
          dnpc = csr.mepc;
          // 将mstatus.MPIE恢复到mstatus.MIE
          csr.mstatus = (csr.mstatus & ~(1 << 3)) | (mstatus_MPIE << 3);
          // 将mstatus.MPIE置为1
          csr.mstatus = csr.mstatus | (1 << 7);
          // 将特权级设置为mstatus.MPP
          priv = mstatus_MPP;
          // 若mstatus.MPP不为M模式，将mstatus_MPRV设为0
          if (mstatus_MPP != 0b11) csr.mstatus = (csr.mstatus & ~(1 << 17));
          // 将mstatus.MPP设为U模式，即0
          csr.mstatus = (csr.mstatus & ~(0b11 << 11));
        break;
        // sret
        case 0x10200073:
          dnpc = csr.sepc;
          // 将mstatus.SPIE恢复到mstatus.SIE
          csr.mstatus = (csr.mstatus & ~(1 << 1)) | (mstatus_SPIE << 1);
          // 将mstatus.SPIE置为1
          csr.mstatus = csr.mstatus | (1 << 5);
          // 将特权级设置为mstatus.SPP
          priv = mstatus_SPP;
          // 若mstatus.M=SPP不为M模式（总是成立），将mstatus_MPRV设为0
          csr.mstatus = (csr.mstatus & ~(1 << 17));
          // 将mstatus.SPP设为U模式，即0
          csr.mstatus = (csr.mstatus & ~(1 << 8));
        break;
        // wfi
        case 0x10500073:
          // TW=1，在U模式执行wfi直接抛出非法指令异常
          if (mstatus_TW && priv == PRIV_U) throw Exception {2, inst};
          // 否则nop
        break;
        default: if constexpr (rt_check) assert(0);
      }
    }
  } else if (op == 0b00011) { // FENCE
    // nop
  } else if (op == 0b01011) { // A 原子指令拓展
    static vaddr_t reserved_addr = 0;
    if (funct3(inst) != 0b010) throw Exception {2, inst};
    inst_t f5 = funct5(inst);
    word_t vaddr = gpr_read(rs1(inst)); // src1
    if (f5 == 0b00010) { // LR
      if (rs2(inst)) throw Exception {2, inst};
      word_t data = vaddr_load(vaddr, 4);
      gpr_write(rd(inst), sext<32>(data));
      reserved_addr = vaddr;
    } else if (f5 == 0b00011) { // SC
      if (reserved_addr == vaddr) {
        word_t src2 = gpr_read(rs2(inst));
        vaddr_store(vaddr, 4, src2);
        gpr_write(rd(inst), 0);
      } else {
        gpr_write(rd(inst), 1);
      }
      reserved_addr = 0;
    } else { // AMO
      word_t src2 = gpr_read(rs2(inst));
      word_t paddr = mmu_translate(vaddr, ACS_STORE);
      word_t t = paddr_read(paddr, 4);
      word_t data;
      switch (f5) {
        case 0b00001: data = src2; break;
        case 0b00000: data = t + src2; break;
        case 0b00100: data = t ^ src2; break;
        case 0b01100: data = t & src2; break;
        case 0b01000: data = t | src2; break;
        case 0b10000: data = std::min(sgn(t), sgn(src2)); break;
        case 0b10100: data = std::max(sgn(t), sgn(src2)); break;
        case 0b11000: data = std::min(t, src2); break;
        case 0b11100: data = std::max(t, src2); break;
        default: throw Exception {2, inst};
      }
      paddr_write(paddr, 4, data);
      gpr_write(rd(inst), sext<32>(t));
    }
  } else {
    Log("Warning: invalid instruction: %08x", inst);
    throw Exception {2, inst};
  }

  set_pc(dnpc);
}
