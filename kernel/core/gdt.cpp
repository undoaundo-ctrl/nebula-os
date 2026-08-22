#include "gdt.hpp"

namespace core {

Gdt gdt;

extern "C" void gdt_flush(uint64_t ptr);

void Gdt::set_entry(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    entries[idx].base_low    = base & 0xFFFF;
    entries[idx].base_mid    = (base >> 16) & 0xFF;
    entries[idx].base_high   = (base >> 24) & 0xFF;
    entries[idx].limit_low   = limit & 0xFFFF;
    entries[idx].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    entries[idx].access      = access;
}

void Gdt::init() {
    set_entry(0, 0, 0, 0, 0);                    // null
    set_entry(1, 0, 0, 0x9A, 0xA0);               // 64-bit kernel code
    set_entry(2, 0, 0, 0x92, 0xA0);               // 64-bit kernel data
    set_entry(3, 0, 0, 0xFA, 0xA0);               // 64-bit user code
    set_entry(4, 0, 0, 0xF2, 0xA0);               // 64-bit user data

    pointer.limit = sizeof(entries) - 1;
    pointer.base  = reinterpret_cast<uint64_t>(&entries);

    gdt_flush(reinterpret_cast<uint64_t>(&pointer));
}

} // namespace core
