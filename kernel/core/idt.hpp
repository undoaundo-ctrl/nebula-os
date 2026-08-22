#pragma once
#include <stdint.h>

namespace core {

struct IdtEntry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct IdtPointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct Registers {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

using IrqHandler = void (*)(Registers*);

class Idt {
public:
    void init();
    void set_gate(uint8_t num, uint64_t handler, uint16_t sel, uint8_t flags);
    void register_irq_handler(uint8_t irq, IrqHandler handler);
    void dispatch(Registers* regs);
private:
    IdtEntry entries[256];
    IdtPointer pointer;
    IrqHandler handlers[256] = {};
};

extern Idt idt;

} // namespace core

extern "C" void idt_load(uint64_t ptr);
