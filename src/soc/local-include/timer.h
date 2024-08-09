#ifndef __SOC_TIMER_H__
#define __SOC_TIMER_H__

#include "device.h"

class Timer final : public Device {
  public:
    Timer(paddr_t addr);
    virtual ~Timer() override;

    virtual void write(paddr_t addr, int len, word_t data) override;
    virtual word_t read(paddr_t addr, int len) override;
};

#endif
