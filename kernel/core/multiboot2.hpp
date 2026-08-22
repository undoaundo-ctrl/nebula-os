#pragma once
#include <stdint.h>

namespace core {

struct Mb2Tag {
    uint32_t type;
    uint32_t size;
};

struct Mb2MemMapEntry {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
};

struct Mb2Framebuffer {
    uint32_t type, size;
    uint64_t addr;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t  bpp;
    uint8_t  fb_type;
    uint16_t reserved;
};

struct MultibootInfo {
    uint64_t total_memory = 0;   // sum of "available" regions
    bool has_framebuffer = false;
    uint64_t fb_addr = 0;
    uint32_t fb_width = 0, fb_height = 0, fb_pitch = 0;
    uint8_t  fb_bpp = 0;
};

inline MultibootInfo parse_multiboot2(uint64_t info_addr) {
    MultibootInfo info;
    uint8_t* ptr = reinterpret_cast<uint8_t*>(info_addr) + 8; // skip total_size+reserved
    for (;;) {
        Mb2Tag* tag = reinterpret_cast<Mb2Tag*>(ptr);
        if (tag->type == 0) break; // end tag

        if (tag->type == 6) { // memory map
            uint8_t* entry_ptr = ptr + 16;
            uint32_t entry_size = *reinterpret_cast<uint32_t*>(ptr + 8);
            uint8_t* end = ptr + tag->size;
            while (entry_ptr < end) {
                Mb2MemMapEntry* e = reinterpret_cast<Mb2MemMapEntry*>(entry_ptr);
                if (e->type == 1) info.total_memory += e->length; // available RAM
                entry_ptr += entry_size;
            }
        } else if (tag->type == 8) { // framebuffer
            Mb2Framebuffer* fb = reinterpret_cast<Mb2Framebuffer*>(ptr);
            info.has_framebuffer = true;
            info.fb_addr = fb->addr;
            info.fb_width = fb->width;
            info.fb_height = fb->height;
            info.fb_pitch = fb->pitch;
            info.fb_bpp = fb->bpp;
        }

        ptr += (tag->size + 7) & ~7u; // 8-byte align
    }
    return info;
}

} // namespace core
