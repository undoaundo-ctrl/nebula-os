#include "ata.hpp"
#include "../include/io.hpp"

namespace drivers {

AtaDisk primary_master;

void AtaDisk::wait_bsy() {
    while (io::inb(io_base + 7) & 0x80) { }
}

bool AtaDisk::wait_drq() {
    // poll status until BSY clears and either DRQ or ERR set
    for (int i = 0; i < 100000; i++) {
        uint8_t status = io::inb(io_base + 7);
        if (status & 0x01) return false;      // ERR
        if (!(status & 0x80) && (status & 0x08)) return true; // ready
    }
    return false;
}

bool AtaDisk::identify(bool primary, bool master) {
    io_base = primary ? 0x1F0 : 0x170;
    ctrl_base = primary ? 0x3F6 : 0x376;
    drive_select = master ? 0xA0 : 0xB0;

    io::outb(io_base + 6, drive_select);
    io::outb(io_base + 2, 0);
    io::outb(io_base + 3, 0);
    io::outb(io_base + 4, 0);
    io::outb(io_base + 5, 0);
    io::outb(io_base + 7, 0xEC); // IDENTIFY

    uint8_t status = io::inb(io_base + 7);
    if (status == 0) { found = false; return false; } // no drive

    wait_bsy();

    uint8_t mid = io::inb(io_base + 4);
    uint8_t hi  = io::inb(io_base + 5);
    if (mid || hi) { found = false; return false; } // not ATA (likely ATAPI)

    if (!wait_drq()) { found = false; return false; }

    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) identify_data[i] = io::inw(io_base);

    found = true;
    return true;
}

bool AtaDisk::read_sectors(uint32_t lba, uint8_t count, void* buffer) {
    if (!found) return false;
    uint16_t* buf = static_cast<uint16_t*>(buffer);

    io::outb(io_base + 6, static_cast<uint8_t>(drive_select | ((lba >> 24) & 0x0F)));
    io::outb(io_base + 2, count);
    io::outb(io_base + 3, static_cast<uint8_t>(lba & 0xFF));
    io::outb(io_base + 4, static_cast<uint8_t>((lba >> 8) & 0xFF));
    io::outb(io_base + 5, static_cast<uint8_t>((lba >> 16) & 0xFF));
    io::outb(io_base + 7, 0x20); // READ SECTORS

    for (int s = 0; s < count; s++) {
        if (!wait_drq()) return false;
        for (int i = 0; i < 256; i++) buf[s * 256 + i] = io::inw(io_base);
    }
    return true;
}

bool AtaDisk::write_sectors(uint32_t lba, uint8_t count, const void* buffer) {
    if (!found) return false;
    const uint16_t* buf = static_cast<const uint16_t*>(buffer);

    io::outb(io_base + 6, static_cast<uint8_t>(drive_select | ((lba >> 24) & 0x0F)));
    io::outb(io_base + 2, count);
    io::outb(io_base + 3, static_cast<uint8_t>(lba & 0xFF));
    io::outb(io_base + 4, static_cast<uint8_t>((lba >> 8) & 0xFF));
    io::outb(io_base + 5, static_cast<uint8_t>((lba >> 16) & 0xFF));
    io::outb(io_base + 7, 0x30); // WRITE SECTORS

    for (int s = 0; s < count; s++) {
        if (!wait_drq()) return false;
        for (int i = 0; i < 256; i++) io::outw(io_base, buf[s * 256 + i]);
        io::outb(io_base + 7, 0xE7); // FLUSH CACHE
    }
    return true;
}

} // namespace drivers
