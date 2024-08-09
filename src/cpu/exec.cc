#include "common.h"
#include "cpu.h"
#include "hart.h"

Hart cpu {0};

void cpu_exec(uint64_t n) {
  switch (emu_state.get_state()) {
    case GlobalState::ST_END:
    case GlobalState::ST_ABORT:
      printf("Program execution has ended. To restart the program, exit and run again.\n");
      return;
    default: emu_state.set_state(GlobalState::ST_RUNNING);
  }

  while (n--) {
    cpu.step();
    if (emu_state.get_state() != GlobalState::ST_RUNNING) {
      break;
    }
  }

  switch (emu_state.get_state()) {
    case GlobalState::ST_RUNNING:
      emu_state.set_state(GlobalState::ST_STOP);
    break;
    case GlobalState::ST_ABORT:
      Log("rvemu " ANSI_FMT("ABORT", ANSI_FG_RED));
    break;
    case GlobalState::ST_END:
      if (emu_state.get_ret() == 0) {
        Log("rvemu " ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN));
      } else {
        Log("rvemu " ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED));
      }
    break;
    default: ;
  }
}
