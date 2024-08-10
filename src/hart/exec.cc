#include "hart.h"

#include "local-include/exception.h"

void Hart::step() {
  try {
    do_inst();
  } catch (Exception &e) {
    
  }
}
