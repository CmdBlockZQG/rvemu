#include "common.h"

#include "local-include/state.h"

HartState::HartState(int id): csr(id) {
  pc = CONF_RESET_VEC;
  gpr[0] = 0;

  priv = HartPriv::PRIV_M;
}

HartState::~HartState() { }

HartPriv HartState::get_priv() {
  return priv;
}

word_t HartState::get_priv_code() {
  return static_cast<word_t>(priv);
}

void HartState::set_priv(HartPriv priv) {
  this->priv = priv;
}

vaddr_t HartState::get_pc() {
  return pc;
}

void HartState::set_pc(vaddr_t dnpc) {
  pc = dnpc;
}

word_t HartState::gpr_read(int id) {
  if constexpr (rt_check) {
    assert(0 <= id && id < gpr_n);
  }
  return id ? gpr[id] : 0;
}

void HartState::gpr_write(int id, word_t data) {
  if constexpr (rt_check) {
    assert(0 <= id && id < gpr_n);
  }
  if (id) gpr[id] = data;
}

HartCSR::HartCSR(int id) {
  mhartid = id;
}

word_t &HartCSR::csr(word_t addr) {
  switch (addr & 0xfff) {
    case 0x180: return satp;

    case 0x300: return mstatus;
    case 0x305: return mtvec;
    case 0x340: return mscratch;
    case 0x341: return mepc;
    case 0x342: return mcause;

    case 0xf11: return mvendorid;
    case 0xf12: return marchid;
    case 0xf14: return mhartid;
    default: Assert(0, "Unsupported CSR: 0x%03x", addr);
  }
}

void HartCSR::write(word_t addr, word_t data) {
  csr(addr) = data;
}

word_t HartCSR::read(word_t addr) {
  return csr(addr);
}
