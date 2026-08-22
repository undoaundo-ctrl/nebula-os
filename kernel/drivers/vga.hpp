#pragma once
#include <stdint.h>
#include <stddef.h>

namespace drivers {

class VgaConsole {
public:
    enum Color : uint8_t {
        BLACK = 0, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHT_GREY,
        DARK_GREY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN, LIGHT_RED,
        LIGHT_MAGENTA, YELLOW, WHITE
    };

    void init();
    void clear();
    void set_color(Color fg, Color bg);
    void put_char(char c);
    void write(const char* str);
    void write_hex(uint64_t val);
    void write_dec(uint64_t val);
    void set_cursor(size_t row, size_t col);

private:
    static constexpr size_t WIDTH = 80;
    static constexpr size_t HEIGHT = 25;
    volatile uint16_t* buffer = reinterpret_cast<uint16_t*>(0xB8000);
    size_t row = 0, col = 0;
    uint8_t color = 0x0F;

    void scroll();
    void put_entry_at(char c, uint8_t clr, size_t x, size_t y);
};

extern VgaConsole console;

} // namespace drivers
