# Nebula OS build system

CXX      := g++
AS       := nasm
LD       := ld

BUILD    := build
ISO_DIR  := iso
KERNEL   := $(BUILD)/nebula_kernel.bin
ISO      := nebula-os.iso

CXXFLAGS := -m64 -ffreestanding -fno-exceptions -fno-rtti \
            -fno-stack-protector -mno-red-zone \
            -mno-sse -mno-sse2 -mno-mmx -mno-80387 \
            -Wall -Wextra -Wno-unused-parameter -O2 -std=c++17 \
            -MMD -MP

LDFLAGS  := -n -T boot/linker.ld -nostdlib

ASFLAGS  := -f elf64

CXX_SOURCES := $(shell find kernel -name '*.cpp')
ASM_SOURCES := boot/boot.asm kernel/core/gdt_flush.asm kernel/core/isr.asm

CXX_OBJECTS := $(patsubst %.cpp,$(BUILD)/%.o,$(CXX_SOURCES))
ASM_OBJECTS := $(patsubst %.asm,$(BUILD)/%.o,$(ASM_SOURCES))
OBJECTS     := $(ASM_OBJECTS) $(CXX_OBJECTS)
DEPS        := $(CXX_OBJECTS:.o=.d)

.PHONY: all clean iso run run-debug

all: $(KERNEL)

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(KERNEL): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

iso: $(KERNEL)
	@mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/nebula_kernel.bin
	cp boot/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISO_DIR) 2>&1 | grep -v "^xorriso" || true

run: iso
	qemu-system-x86_64 -cdrom $(ISO) -m 256M -serial stdio

run-debug: iso
	qemu-system-x86_64 -cdrom $(ISO) -m 256M -serial stdio -d int -no-reboot -no-shutdown

clean:
	rm -rf $(BUILD) $(ISO_DIR)/boot/nebula_kernel.bin $(ISO)

-include $(DEPS)
