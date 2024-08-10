#include "hart.h"

#include "local-include/exception.h"

void Hart::step() {
  try {
    do_inst();
  } catch (Exception &e) {
    csr.mcause = e.cause;
    csr.mepc = get_pc();
    set_pc(csr.mtvec);
    
    switch (e.cause) {
      case 11: // ecall
      break;
      case 3: // ebreak
        emu_state.set_state(GlobalState::ST_END);
        emu_state.set_ret(gpr_read(10));
      break;
      default: if constexpr (rt_check) Assert(0, "Unsupported exception: %d", e.cause);
    }
  }
}
