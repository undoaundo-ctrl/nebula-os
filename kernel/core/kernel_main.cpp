#include <stdint.h>
#include "gdt.hpp"
#include "idt.hpp"
#include "multiboot2.hpp"
#include "../mm/heap.hpp"
#include "../drivers/vga.hpp"
#include "../drivers/serial.hpp"
#include "../drivers/keyboard.hpp"
#include "../drivers/timer.hpp"
#include "../drivers/pci.hpp"
#include "../drivers/ata.hpp"
#include "../include/libk.hpp"

extern "C" void call_global_constructors();

using namespace drivers;

static void print_pci_devices() {
    console.write("\nPCI devices:\n");
    for (size_t i = 0; i < pci.count(); i++) {
        const auto& d = pci.device(i);
        console.write("  ");
        console.write_hex(d.vendor_id);
        console.write(":");
        console.write_hex(d.device_id);
        console.write(" class=");
        console.write_hex(d.class_code);
        console.write(" sub=");
        console.write_hex(d.subclass);

        if (d.vendor_id == 0x10DE) {
            console.set_color(VgaConsole::YELLOW, VgaConsole::BLACK);
            console.write("  <- NVIDIA device detected (no accelerated driver: "
                           "GPU accel needs the Linux DRM/GEM ABI, see README)");
            console.set_color(VgaConsole::LIGHT_GREY, VgaConsole::BLACK);
        } else if (d.class_code == 0x03) {
            console.write("  <- display controller");
        }
        console.write("\n");
    }
}

static void shell() {
    console.write("\nnebula> ");
    char line[128];
    size_t len = 0;

    for (;;) {
        if (keyboard.has_key()) {
            char c = keyboard.pop_key();
            if (c == '\n') {
                console.put_char('\n');
                line[len] = '\0';

                if (libk::strcmp(line, "help") == 0) {
                    console.write("commands: help, uptime, mem, pci, disk, clear\n");
                } else if (libk::strcmp(line, "uptime") == 0) {
                    console.write("ticks: ");
                    console.write_dec(timer.ticks());
                    console.write("\n");
                } else if (libk::strcmp(line, "mem") == 0) {
                    console.write("heap used: ");
                    console.write_dec(mm::kheap.used());
                    console.write(" / ");
                    console.write_dec(mm::kheap.total());
                    console.write(" bytes\n");
                } else if (libk::strcmp(line, "pci") == 0) {
                    print_pci_devices();
                } else if (libk::strcmp(line, "disk") == 0) {
                    if (primary_master.present()) {
                        console.write("primary master ATA disk: present\n");
                    } else {
                        console.write("primary master ATA disk: not found\n");
                    }
                } else if (libk::strcmp(line, "clear") == 0) {
                    console.clear();
                } else if (len > 0) {
                    console.write("unknown command: ");
                    console.write(line);
                    console.write("\n");
                }

                len = 0;
                console.write("nebula> ");
            } else if (c == '\b') {
                if (len > 0) { len--; console.put_char('\b'); console.put_char(' '); console.put_char('\b'); }
            } else if (len < sizeof(line) - 1) {
                line[len++] = c;
                console.put_char(c);
            }
        } else {
            asm volatile("hlt");
        }
    }
}

extern "C" void kernel_main_trampoline(uint64_t mb_info_addr) {
    call_global_constructors();

    console.init();
    com1.init();

    console.set_color(VgaConsole::LIGHT_GREEN, VgaConsole::BLACK);
    console.write("Nebula OS -- custom C++ kernel, x86_64 long mode\n");
    console.set_color(VgaConsole::LIGHT_GREY, VgaConsole::BLACK);
    com1.write("[boot] Nebula OS kernel starting\n");

    core::gdt.init();
    com1.write("[boot] GDT loaded\n");

    core::idt.init();
    com1.write("[boot] IDT loaded, PIC remapped\n");

    core::MultibootInfo info = core::parse_multiboot2(mb_info_addr);
    console.write("Detected RAM: ");
    console.write_dec(info.total_memory / 1024 / 1024);
    console.write(" MiB\n");

    // Reserve a heap region well above the kernel image (identity-mapped
    // by the first 1 GiB of huge pages set up in boot.asm).
    mm::kheap.init(0x400000, 16 * 1024 * 1024); // 16 MiB heap at 4 MiB
    com1.write("[boot] heap initialized\n");

    timer.init(100); // 100 Hz
    com1.write("[boot] PIT timer initialized (100 Hz)\n");

    keyboard.init();
    com1.write("[boot] PS/2 keyboard driver registered\n");

    asm volatile("sti");
    com1.write("[boot] interrupts enabled\n");

    pci.scan();
    console.write("PCI scan found ");
    console.write_dec(pci.count());
    console.write(" device(s). Type 'pci' to list them.\n");

    if (primary_master.identify(true, true)) {
        console.write("Primary master ATA disk detected.\n");
    }

    if (info.has_framebuffer) {
        console.write("Framebuffer: ");
        console.write_dec(info.fb_width);
        console.write("x");
        console.write_dec(info.fb_height);
        console.write(" @ ");
        console.write_dec(info.fb_bpp);
        console.write("bpp (generic VBE/VESA mode set by GRUB -- 2D pixel "
                       "access only, no 3D acceleration)\n");
    }

    console.write("Type 'help' for a list of commands.\n");
    shell();
}
