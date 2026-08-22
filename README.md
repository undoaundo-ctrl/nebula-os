# Nebula OS

A hobby operating system with a **custom, non-Linux kernel** written in C++ for
x86_64, booted via a Multiboot2-compliant assembly stub and packaged as a
bootable `.iso` (built with GRUB).

This is a real, from-scratch kernel — not a Linux distro, not a fork.

## What's implemented

- **Bootloader glue** (`boot/boot.asm`): Multiboot2 header, CPU feature checks
  (CPUID/long-mode), identity-mapped paging (first 1 GiB via 2 MiB huge
  pages), transition from 32-bit protected mode into 64-bit long mode.
- **Core** (`kernel/core/`):
  - GDT (flat 64-bit code/data segments, ring 0 + ring 3 descriptors)
  - IDT with all 32 CPU exception vectors and 16 IRQ vectors wired up,
    PIC remapped to vectors 32–47
  - Multiboot2 info-tag parser (memory map, framebuffer info)
  - A tiny C++ freestanding runtime (global constructors, `operator new`/
    `delete`, pure-virtual and stack-protector stubs)
- **Drivers** (`kernel/drivers/`):
  - VGA text-mode console (80x25, scrolling, hardware cursor)
  - 16550 UART serial driver (COM1, used as a debug console)
  - PS/2 keyboard driver (IRQ1, scancode set 1, US layout, shift support)
  - PIT timer driver (IRQ0, configurable frequency, tick counter)
  - PCI bus enumeration (via 0xCF8/0xCFC config space)
  - ATA PIO disk driver (28-bit LBA read/write, IDENTIFY)
- **Memory**: a simple bump-allocator heap backing `new`/`delete`.
- **Shell**: a minimal interactive command line on the VGA console
  (`help`, `uptime`, `mem`, `pci`, `disk`, `clear`).

## What is deliberately NOT implemented — and why

This kernel does **not** include NVIDIA's driver, Mesa3D, OpenGL, or Vulkan,
and it's important to be upfront about why rather than pretend otherwise:

- NVIDIA's driver (both the proprietary blob and the open `nvidia-open`
  kernel modules) is written against the **Linux kernel's internal ABI**
  (DRM/KMS, GEM/TTM memory management, `dma-buf`, the Linux module-loading
  and locking primitives, etc.). It is not portable to an arbitrary kernel —
  there is no supported way to load it outside Linux (or, historically,
  Windows via a separate driver).
- Mesa3D (the open-source implementation of OpenGL/Vulkan/OpenCL) likewise
  assumes a Linux/BSD-like host: DRM render nodes, GBM, POSIX threads,
  `mmap` semantics, etc. It isn't a portable library you can drop onto any
  kernel — porting it means reimplementing large parts of that substrate
  first.
- Building a from-scratch GPU driver stack (mode-setting, command
  submission, memory management, shader compilation, a Vulkan ICD) is a
  multi-year effort even for a single vendor's hardware — this is
  effectively what the Linux graphics stack represents after over a decade
  of work by many companies and contributors.

What this kernel *can* do graphically: GRUB can set a linear framebuffer via
VBE/VESA before handing off to the kernel (see the Multiboot2 framebuffer tag
parsed in `kernel/core/multiboot2.hpp`), which gives you raw pixel access for
a 2D software renderer. That's a realistic next step; 3D acceleration is not.

The PCI driver *will* detect an NVIDIA GPU if one is present (vendor ID
`0x10DE`) and print a note about this limitation — it does not attempt to
drive it.

## Building

Requires: `g++` (C++17), `nasm`, `ld`, `grub-mkrescue`, `xorriso`, `mtools`.

```sh
make        # build kernel/build/nebula_kernel.bin
make iso    # build nebula-os.iso
make run    # build + boot in QEMU (needs qemu-system-x86_64)
```

## Boot-testing

```sh
qemu-system-x86_64 -cdrom nebula-os.iso -m 256M -serial stdio
```

## Project layout

```
boot/            multiboot2 entry, long-mode setup, linker script, grub.cfg
kernel/core/     GDT, IDT/ISR/IRQ, C++ runtime, multiboot2 parsing, kernel_main
kernel/drivers/  VGA, serial, keyboard, PIT timer, PCI, ATA
kernel/mm/       bump-allocator heap
kernel/include/  freestanding port-I/O and libc-lite helpers
Makefile
```

## Status / disclaimer

This is a boot-time educational kernel: no virtual memory beyond identity
paging, no user-mode processes, no filesystem (raw sector I/O only), no SMP,
no preemptive scheduler. It's a solid foundation to build those on top of,
not a production OS.
