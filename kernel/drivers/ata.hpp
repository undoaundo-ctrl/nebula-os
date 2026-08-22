#pragma once
#include <stdint.h>
#include <stddef.h>

namespace drivers {

class AtaDisk {
public:
    bool identify(bool primary, bool master);
    bool read_sectors(uint32_t lba, uint8_t count, void* buffer);
    bool write_sectors(uint32_t lba, uint8_t count, const void* buffer);
    bool present() const { return found; }

private:
    uint16_t io_base = 0x1F0;
    uint16_t ctrl_base = 0x3F6;
    uint8_t drive_select = 0xE0;
    bool found = false;

    void wait_bsy();
    bool wait_drq();
};

extern AtaDisk primary_master;

} // namespace drivers
