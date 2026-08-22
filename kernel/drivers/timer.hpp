#pragma once
#include <stdint.h>
#include "../core/idt.hpp"

namespace drivers {

class Timer {
public:
    void init(uint32_t frequency_hz);
    uint64_t ticks() const { return tick_count; }
    void sleep_ticks(uint64_t n);
private:
    volatile uint64_t tick_count = 0;
    static void irq_handler(core::Registers* regs);
};

extern Timer timer;

} // namespace drivers
