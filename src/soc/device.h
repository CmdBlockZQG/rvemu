#ifndef __SOC_DEVICE_H__
#define __SOC_DEVICE_H__

#include "common.h"

class Device {
  protected:
    paddr_t base;
    paddr_t size;
  
  public:
    Device(paddr_t base, paddr_t size);
    virtual ~Device() = 0;

    bool in(paddr_t addr);

    virtual void write(paddr_t addr, int len, word_t data) = 0;
    virtual word_t read(paddr_t addr, int len) = 0;
};

class RAM final : public Device {
  private:
    uint8_t *ptr;
  public:
    RAM(paddr_t base, paddr_t size);
    virtual ~RAM() override;

    virtual void write(paddr_t addr, int len, word_t data) override;
    virtual word_t read(paddr_t addr, int len) override;

    void *get_ptr(paddr_t addr);
};

#endif
