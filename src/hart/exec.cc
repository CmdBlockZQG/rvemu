#include "hart.h"
#include "soc.h"

#include "local-include/exception.h"

void Hart::trap(word_t cause, word_t tval) {
  csr.mcause = cause;
  csr.mepc = get_pc();
  // TODO: mtval
  set_pc(csr.mtvec);

  // 将mstatus.MIE保存至mstatus.MPIE
  csr.mstatus = (csr.mstatus & ~(1 << 7)) | (((csr.mstatus >> 3) & 1) << 7);
  // 将mstatus.MIE置为0
  csr.mstatus = csr.mstatus & ~(1 << 3);
}

void Hart::step() {
  // TODO: 根据中断情况更新CSR的值

  try {
    do_inst();

    // 中断触发
    if (((csr.mstatus >> 3) & 1) && clint.get_mtip()) { // 时钟中断
      trap(0x80000007, 0);
    }
    IFDEF(CONF_UART_INPUT, uart.refill());
  } catch (Exception &e) { // 同步异常
    trap(e.cause, e.tval);

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
