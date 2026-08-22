#include "pci.hpp"
#include "../include/io.hpp"

namespace drivers {

Pci pci;

uint32_t Pci::read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1u << 31) | (static_cast<uint32_t>(bus) << 16) |
                        (static_cast<uint32_t>(slot) << 11) |
                        (static_cast<uint32_t>(func) << 8) |
                        (offset & 0xFC);
    io::outl(0xCF8, address);
    return io::inl(0xCFC);
}

void Pci::scan() {
    device_count = 0;
    for (uint16_t bus = 0; bus < 256 && device_count < MAX_DEVICES; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id = read_config(static_cast<uint8_t>(bus), slot, func, 0x00);
                uint16_t vendor = id & 0xFFFF;
                if (vendor == 0xFFFF) continue; // no device

                uint32_t class_reg = read_config(static_cast<uint8_t>(bus), slot, func, 0x08);

                PciDevice dev;
                dev.bus = static_cast<uint8_t>(bus);
                dev.slot = slot;
                dev.func = func;
                dev.vendor_id = vendor;
                dev.device_id = (id >> 16) & 0xFFFF;
                dev.prog_if = (class_reg >> 8) & 0xFF;
                dev.subclass = (class_reg >> 16) & 0xFF;
                dev.class_code = (class_reg >> 24) & 0xFF;

                if (device_count < MAX_DEVICES) devices[device_count++] = dev;
            }
        }
    }
}

} // namespace drivers
