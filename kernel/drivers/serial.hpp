#pragma once
#include <stdint.h>
#include <stddef.h>

namespace drivers {

class Serial {
public:
    void init(uint16_t port = 0x3F8);
    void write_char(char c);
    void write(const char* str);
    bool has_data();
    char read_char();

private:
    uint16_t base = 0x3F8;
    bool transmit_empty();
};

extern Serial com1;

} // namespace drivers
