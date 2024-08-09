#include "utils.h"

GlobalState emu_state;

void GlobalState::set_state(int state) {
  this->state = state;
}

void GlobalState::set_ret(int ret) {
  halt_ret = ret;
}

int GlobalState::get_state() {
  return state;
}

int GlobalState::get_ret() {
  return halt_ret;
}

int GlobalState::exit_code() {
  if ((state == ST_END && halt_ret == 0) || state == ST_QUIT) {
    return 0;
  } else {
    return 1;
  }
}
