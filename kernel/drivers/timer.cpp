#include "timer.hpp"
#include "../include/io.hpp"

namespace drivers {

Timer timer;

void Timer::irq_handler(core::Registers*) {
    timer.tick_count++;
}

void Timer::init(uint32_t frequency_hz) {
    core::idt.register_irq_handler(0, &Timer::irq_handler);

    uint32_t divisor = 1193182 / frequency_hz;
    io::outb(0x43, 0x36);
    io::outb(0x40, static_cast<uint8_t>(divisor & 0xFF));
    io::outb(0x40, static_cast<uint8_t>((divisor >> 8) & 0xFF));
}

void Timer::sleep_ticks(uint64_t n) {
    uint64_t target = tick_count + n;
    while (tick_count < target) asm volatile("hlt");
}

} // namespace drivers
