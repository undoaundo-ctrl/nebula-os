#include "idt.hpp"
#include "../include/io.hpp"
#include "../drivers/vga.hpp"

namespace core {

Idt idt;

// ISR/IRQ assembly stub symbols
extern "C" {
    void isr0();  void isr1();  void isr2();  void isr3();  void isr4();
    void isr5();  void isr6();  void isr7();  void isr8();  void isr9();
    void isr10(); void isr11(); void isr12(); void isr13(); void isr14();
    void isr15(); void isr16(); void isr17(); void isr18(); void isr19();
    void isr20(); void isr21(); void isr22(); void isr23(); void isr24();
    void isr25(); void isr26(); void isr27(); void isr28(); void isr29();
    void isr30(); void isr31();
    void irq0();  void irq1();  void irq2();  void irq3();  void irq4();
    void irq5();  void irq6();  void irq7();  void irq8();  void irq9();
    void irq10(); void irq11(); void irq12(); void irq13(); void irq14();
    void irq15();
}

static void remap_pic() {
    io::outb(0x20, 0x11); io::outb(0xA0, 0x11);
    io::outb(0x21, 0x20); io::outb(0xA1, 0x28); // remap to 32-47
    io::outb(0x21, 0x04); io::outb(0xA1, 0x02);
    io::outb(0x21, 0x01); io::outb(0xA1, 0x01);
    io::outb(0x21, 0x00); io::outb(0xA1, 0x00);
}

void Idt::set_gate(uint8_t num, uint64_t handler, uint16_t sel, uint8_t flags) {
    entries[num].offset_low  = handler & 0xFFFF;
    entries[num].offset_mid  = (handler >> 16) & 0xFFFF;
    entries[num].offset_high = (handler >> 32) & 0xFFFFFFFF;
    entries[num].selector    = sel;
    entries[num].ist         = 0;
    entries[num].type_attr   = flags;
    entries[num].zero        = 0;
}

void Idt::init() {
    pointer.limit = sizeof(entries) - 1;
    pointer.base  = reinterpret_cast<uint64_t>(&entries);

    remap_pic();

    void (*isr_table[32])() = {
        isr0,isr1,isr2,isr3,isr4,isr5,isr6,isr7,isr8,isr9,isr10,isr11,
        isr12,isr13,isr14,isr15,isr16,isr17,isr18,isr19,isr20,isr21,
        isr22,isr23,isr24,isr25,isr26,isr27,isr28,isr29,isr30,isr31
    };
    for (int i = 0; i < 32; i++)
        set_gate(i, reinterpret_cast<uint64_t>(isr_table[i]), 0x08, 0x8E);

    void (*irq_table[16])() = {
        irq0,irq1,irq2,irq3,irq4,irq5,irq6,irq7,irq8,irq9,irq10,irq11,
        irq12,irq13,irq14,irq15
    };
    for (int i = 0; i < 16; i++)
        set_gate(32 + i, reinterpret_cast<uint64_t>(irq_table[i]), 0x08, 0x8E);

    idt_load(reinterpret_cast<uint64_t>(&pointer));
}

void Idt::register_irq_handler(uint8_t irq, IrqHandler handler) {
    handlers[32 + irq] = handler;
}

void Idt::dispatch(Registers* regs) {
    if (handlers[regs->int_no]) {
        handlers[regs->int_no](regs);
    } else if (regs->int_no < 32) {
        drivers::console.write("\n[EXCEPTION] vector=");
        drivers::console.write_dec(regs->int_no);
        drivers::console.write(" err=");
        drivers::console.write_hex(regs->err_code);
        drivers::console.write(" -- system halted\n");
        asm volatile("cli");
        for (;;) asm volatile("hlt");
    }
    if (regs->int_no >= 40) io::outb(0xA0, 0x20); // slave EOI
    if (regs->int_no >= 32) io::outb(0x20, 0x20); // master EOI
}

} // namespace core

extern "C" void isr_common_handler(core::Registers* regs) {
    core::idt.dispatch(regs);
}
