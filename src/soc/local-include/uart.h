#ifndef __SOC_UART_H__
#define __SOC_UART_H__

#include "device.h"

class UART final : public Device {
  public:
    UART(paddr_t addr);
    virtual ~UART() override;

    virtual void write(paddr_t addr, int len, word_t data) override;
    virtual word_t read(paddr_t addr, int len) override;
};

#endif
