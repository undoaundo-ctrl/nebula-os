#pragma once
#include <stdint.h>

namespace core {

struct GdtEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct GdtPointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

class Gdt {
public:
    void init();
private:
    GdtEntry entries[5];
    GdtPointer pointer;
    void set_entry(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
};

extern Gdt gdt;

} // namespace core
