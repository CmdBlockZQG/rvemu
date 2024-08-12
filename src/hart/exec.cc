#include "hart.h"
#include "soc.h"

#include "local-include/exception.h"
#include "local-include/decode.h"

// 捕获异常，trap到异常处理程序
void Hart::trap(word_t cause, word_t tval) {
  csr.mcause = cause;
  csr.mepc = get_pc();
  csr.mtval = tval;
  set_pc(csr.mtvec);

  // 将mstatus.MIE保存至mstatus.MPIE
  csr.mstatus = (csr.mstatus & ~(1 << 7)) | (mstatus_MIE << 7);
  // 将mstatus.MIE设为0
  csr.mstatus = csr.mstatus & ~(1 << 3);
  // 将mstatus.MPP设为当前特权级
  csr.mstatus = (csr.mstatus & ~(0b11 << 11)) | (get_priv_code() << 11);
  assert(mstatus_MPP == 3);
  // 将当前特权级设为M
  set_priv(HartPriv::PRIV_M);

  // TODO: 异常中断委托，trap到S模式 P26
}

void Hart::step() {
  // TODO: 根据中断情况更新CSR的值


  try {
    do_inst();

    // 时钟中断
    if (mstatus_MIE && clint.get_mtip()) {
      trap(0x80000007, 0);
    }

    // 外部中断
    IFDEF(CONF_UART_INPUT, uart.refill());


  } catch (Exception &e) { // 同步异常
    trap(e.cause, e.tval);

    if (e.cause == 3) { // ebreak
      emu_state.set_state(GlobalState::ST_END);
      emu_state.set_ret(gpr_read(10));
    }
  }
}
