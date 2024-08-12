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

word_t &HartState::addr_csr(word_t addr) {
  switch (addr & 0xfff) {
    case 0x180: return csr.satp;

    case 0x300: return csr.mstatus;
    case 0x305: return csr.mtvec;
    case 0x340: return csr.mscratch;
    case 0x341: return csr.mepc;
    case 0x342: return csr.mcause;

    case 0xf11: return csr.mvendorid;
    case 0xf12: return csr.marchid;
    case 0xf14: return csr.mhartid;
    default: throw;
  }
}

word_t HartState::csr_read(word_t addr) {
  // 检查当前特权级是否能访问CSR
  word_t csr_priv = bits<9, 8>(addr);
  if (get_priv_code() < csr_priv) throw;
  
  return addr_csr(addr);
}

void HartState::csr_write(word_t addr, word_t data) {
  // 检查CSR是否只读
  word_t csr_flag = bits<11, 10>(addr);
  if (csr_flag == 0b11) throw;
  // 检查当前特权级是否能访问CSR
  word_t csr_priv = bits<9, 8>(addr);
  if (get_priv_code() < csr_priv) throw;

  addr_csr(addr) = data;
}
