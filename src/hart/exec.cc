#include "hart.h"
#include "soc.h"

#include "local-include/decode.h"
#include "local-include/misc.h"

#include <initializer_list>

Exception::Exception(word_t cause, word_t tval): cause(cause), tval(tval) { }

void Hart::step() {
  // 根据中断信号更新mip
  word_t mip_i = (clint.get_msip() << 3 ) |
                 (clint.get_mtip() << 7 ) |
                 ( plic.get_meip() << 11) ;
  csr.mip = (csr.mip & ~0x888) | mip_i;

  try {
    // 执行指令
    do_inst();

    // 异步中断
    constexpr word_t flag_intr = static_cast<word_t>(1) << 31;
    word_t mi = csr.mip & csr.mie & ~csr.mideleg;
    word_t si = csr.mip & csr.mie & csr.mideleg;
    if (mi && ((priv == PRIV_M && mstatus_MIE) || priv < PRIV_M)) {
      bool f = false;
      for (word_t i : {11, 3, 7, 9, 1, 5}) {
        if ((mi >> i) & 1) {
          f = true;
          mtrap(i | flag_intr, 0);
          break;
        }
      }
      if constexpr (rt_check) assert(f);
    } else if (si && ((priv == PRIV_S && mstatus_SIE) || priv < PRIV_S)) {
      bool f = false;
      for (word_t i : {9, 1, 5}) {
        if ((si >> i) & 1) {
          f = true;
          strap(i | flag_intr, 0);
          break;
        }
      }
      if constexpr (rt_check) assert(f);
    }
  } catch (Exception &e) { // 同步异常
    // Log("Exception: %d, 0x%08x, at pc=0x%08x", e.cause, e.tval, get_pc());
    if (priv == PRIV_M) {
      mtrap(e.cause, e.tval);
    } else {
      uint64_t medeleg = (static_cast<uint64_t>(csr.medelegh) << 32) | csr.medeleg;
      // 32位以上的异常都是Reserved，都没实现
      // but i do this anyway
      if ((medeleg >> e.cause) & 1) { // 异常被委托到S模式
        strap(e.cause, e.tval);        
      } else { // 还在M模式处理
        mtrap(e.cause, e.tval);
      }
    }
    
    if constexpr (ISDEF(CONF_AM)) {
      if (e.cause == 3) { // ebreak
        emu_state.set_state(GlobalState::ST_END);
        emu_state.set_ret(gpr_read(10));
      }
    }
  }

  csr.counter += 1;
}

// trap到M模式异常处理程序
void Hart::mtrap(word_t mcause, word_t mtval) {
  csr.mcause = mcause;
  csr.mepc = get_pc();
  csr.mtval = mtval;
  if ((csr.mtvec & 0b11) == 1 && (mcause >> 31)) {
    set_pc((csr.mtvec & ~0b11) + (mcause << 2));
  } else {
    set_pc(csr.mtvec);
  }
  // 将mstatus.MIE保存至mstatus.MPIE
  csr.mstatus = (csr.mstatus & ~(1 << 7)) | (mstatus_MIE << 7);
  // 将mstatus.MIE设为0
  csr.mstatus = csr.mstatus & ~(1 << 3);
  // 将mstatus.MPP设为当前特权级
  csr.mstatus = (csr.mstatus & ~(0b11 << 11)) | (priv << 11);
  // 将当前特权级设为M
  priv = PRIV_M;
}

// trap到S模式异常处理程序
void Hart::strap(word_t scause, word_t stval) {
  if constexpr (rt_check) assert(priv <= PRIV_S); 
  csr.scause = scause;
  csr.sepc = get_pc();
  csr.stval = stval;
  if ((csr.stvec & 0b11) == 1 && (scause >> 31)) {
    set_pc((csr.stvec & ~0b11) + (scause << 2));
  } else {
    set_pc(csr.stvec);
  }
  // 将mstatus.SIE保存至mstatus.SPIE
  csr.mstatus = (csr.mstatus & ~(1 << 5)) | (mstatus_SIE << 5);
  // 将mstatus.SIE设为0
  csr.mstatus = csr.mstatus & ~(1 << 1);
  // 将mstatus.SPP设为当前特权级
  csr.mstatus = (csr.mstatus & ~(1 << 8)) | ((priv & 1) << 8);
  // 将当前特权级设为S
  priv = PRIV_S;
}

