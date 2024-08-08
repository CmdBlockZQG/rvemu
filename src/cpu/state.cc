#include "common.h"
#include "cpu.h"

HartState::HartState(int id) {
  pc = CONF_RESET_VEC;
  gpr[0] = 0;

  priv = HartPriv::PRIV_M;
}

HartState::~HartState() { }

vaddr_t HartState::get_pc() {
  return pc;
}

void HartState::set_pc(vaddr_t dnpc) {
  pc = dnpc;
}

word_t HartState::gpr_read(int id) {
  if constexpr (CONF_RT_CHECK) {
    assert(0 <= id && id < gpr_n);
  }
  return id ? gpr[id] : 0;
}

void HartState::gpr_write(int id, word_t data) {
  if constexpr (CONF_RT_CHECK) {
    assert(0 <= id && id < gpr_n);
  }
  if (id) gpr[id] = data;
}
