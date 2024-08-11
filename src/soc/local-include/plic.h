#ifndef __SOC_PLIC_H__
#define __SOC_PLIC_H__

#include "device.h"

// TEMP: 只支持一个硬件线程和一个中断源（UART）
class PLIC final : public Device {
  public:
    PLIC(paddr_t base);
    virtual ~PLIC() override;

    virtual void write(paddr_t addr, int len, word_t data) override;
    virtual word_t read(paddr_t addr, int len) override;
};

#endif
