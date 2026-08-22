#pragma once
#include <stdint.h>
#include <stddef.h>
#include "../core/idt.hpp"

namespace drivers {

class Keyboard {
public:
    void init();
    bool has_key();
    char pop_key();
private:
    static constexpr size_t BUF_SIZE = 256;
    char buffer[BUF_SIZE] = {};
    size_t head = 0, tail = 0;
    bool shift = false;

    void push(char c);
    static void irq_handler(core::Registers* regs);
};

extern Keyboard keyboard;

} // namespace drivers
