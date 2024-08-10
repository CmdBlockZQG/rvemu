#ifndef __HART_DECODE_H__
#define __HART_DECODE_H__

#include "common.h"

using inst_t = uint32_t;

// -------------------- Helper --------------------

static inline constexpr uint64_t bit_mask(int x) {
  uint64_t t = 1;
  return (t << x) - 1;
}

// 类似verilog中的 x[hi:lo]
template<int H, int L>
static inline uint64_t bits(uint64_t x) {
  // return (x >> L) & ((1 << (H - L + 1)) - 1);
  return (x >> L) & bit_mask(H - L + 1);
}

// 将B位补码进行符号拓展
template<int B>
static inline uint64_t sext(uint64_t x) {
  struct { int64_t x: B; } s;
  return s.x = x;
}

// 将x[hi:lo]补码进行符号拓展
template<int H, int L>
static inline uint64_t sbits(uint64_t x) {
  return sext<H - L + 1>(bits<H, L>(x));
}

// -------------------- 立即数 --------------------

static inline word_t immI(inst_t inst) {
  return sbits<31, 20>(inst);
}

static inline word_t immS(inst_t inst) {
  return (sbits<31, 25>(inst) << 5) | bits<11, 7>(inst);
}

static inline word_t immB(inst_t inst) {
  return (sbits<31, 31>(inst) << 12) |
         ( bits< 7,  7>(inst) << 11) |
         ( bits<30, 25>(inst) << 5 ) |
         ( bits<11,  8>(inst) << 1 );
}

static inline word_t immU(inst_t inst) {
  return sbits<31, 12>(inst) << 12;
}

static inline word_t immJ(inst_t inst) {
  return (sbits<31, 31>(inst) << 20) |
         ( bits<19, 12>(inst) << 12) |
         ( bits<20, 20>(inst) << 11) |
         ( bits<30, 21>(inst) << 1 );
}

static inline word_t zimm(inst_t inst) {
  return bits<19, 15>(inst);
}

// -------------------- 指令 --------------------
static inline inst_t c_opcode(inst_t inst) {
  return bits<1, 0>(inst);
}

static inline inst_t opcode(inst_t inst) {
  return bits<6, 2>(inst);
}

static inline inst_t rs1(inst_t inst) {
  return bits<19, 15>(inst);
}

static inline inst_t rs2(inst_t inst) {
  return bits<24, 20>(inst);
}

static inline inst_t rd(inst_t inst) {
  return bits<11, 7>(inst);
}

static inline inst_t funct3(inst_t inst) {
  return bits<14, 12>(inst);
}

static inline inst_t funct7(inst_t inst) {
  return bits<31, 25>(inst);
}

static inline inst_t funct12(inst_t inst) {
  return bits<31, 20>(inst);
}

#endif
