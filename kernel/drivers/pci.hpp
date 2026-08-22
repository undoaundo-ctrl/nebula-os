#pragma once
#include <stdint.h>
#include <stddef.h>

namespace drivers {

struct PciDevice {
    uint8_t bus, slot, func;
    uint16_t vendor_id, device_id;
    uint8_t class_code, subclass, prog_if;
};

class Pci {
public:
    static constexpr size_t MAX_DEVICES = 64;

    void scan();
    size_t count() const { return device_count; }
    const PciDevice& device(size_t i) const { return devices[i]; }

    uint32_t read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

private:
    PciDevice devices[MAX_DEVICES];
    size_t device_count = 0;
};

extern Pci pci;

} // namespace drivers
