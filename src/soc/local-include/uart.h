#ifndef __SOC_UART_H__
#define __SOC_UART_H__

#include "device.h"

#include <queue>

class UART final : public Device {
  private:
    std::queue<char> fifo;

  public:
    UART(paddr_t base);
    virtual ~UART() override;

    virtual void write(paddr_t addr, int len, word_t data) override;
    virtual word_t read(paddr_t addr, int len) override;

    // 填充字符缓冲区
    void refill();
    // 是否有中断请求
    word_t get_ip();
};

#endif
