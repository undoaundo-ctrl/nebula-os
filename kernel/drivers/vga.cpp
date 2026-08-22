#include "vga.hpp"
#include "../include/libk.hpp"
#include "../include/io.hpp"

namespace drivers {

VgaConsole console;

static inline uint8_t make_color(VgaConsole::Color fg, VgaConsole::Color bg) {
    return static_cast<uint8_t>(fg) | (static_cast<uint8_t>(bg) << 4);
}

static inline uint16_t make_entry(char c, uint8_t clr) {
    return static_cast<uint16_t>(c) | (static_cast<uint16_t>(clr) << 8);
}

void VgaConsole::init() {
    color = make_color(LIGHT_GREY, BLACK);
    clear();
}

void VgaConsole::clear() {
    for (size_t y = 0; y < HEIGHT; y++)
        for (size_t x = 0; x < WIDTH; x++)
            put_entry_at(' ', color, x, y);
    row = col = 0;
    set_cursor(0, 0);
}

void VgaConsole::set_color(Color fg, Color bg) {
    color = make_color(fg, bg);
}

void VgaConsole::put_entry_at(char c, uint8_t clr, size_t x, size_t y) {
    buffer[y * WIDTH + x] = make_entry(c, clr);
}

void VgaConsole::scroll() {
    for (size_t y = 1; y < HEIGHT; y++)
        for (size_t x = 0; x < WIDTH; x++)
            buffer[(y - 1) * WIDTH + x] = buffer[y * WIDTH + x];
    for (size_t x = 0; x < WIDTH; x++)
        put_entry_at(' ', color, x, HEIGHT - 1);
    row = HEIGHT - 1;
}

void VgaConsole::put_char(char c) {
    if (c == '\n') {
        col = 0;
        if (++row == HEIGHT) scroll();
        set_cursor(row, col);
        return;
    }
    if (c == '\r') { col = 0; set_cursor(row, col); return; }

    put_entry_at(c, color, col, row);
    if (++col == WIDTH) {
        col = 0;
        if (++row == HEIGHT) scroll();
    }
    set_cursor(row, col);
}

void VgaConsole::write(const char* str) {
    for (size_t i = 0; str[i]; i++) put_char(str[i]);
}

void VgaConsole::write_hex(uint64_t val) {
    char buf[19];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 0; i < 16; i++) {
        uint8_t nibble = (val >> ((15 - i) * 4)) & 0xF;
        buf[2 + i] = nibble < 10 ? ('0' + nibble) : ('a' + nibble - 10);
    }
    buf[18] = '\0';
    write(buf);
}

void VgaConsole::write_dec(uint64_t val) {
    if (val == 0) { put_char('0'); return; }
    char buf[21];
    int i = 20;
    buf[i--] = '\0';
    while (val > 0) {
        buf[i--] = '0' + (val % 10);
        val /= 10;
    }
    write(&buf[i + 1]);
}

void VgaConsole::set_cursor(size_t r, size_t c) {
    row = r; col = c;
    uint16_t pos = static_cast<uint16_t>(r * WIDTH + c);
    io::outb(0x3D4, 0x0F);
    io::outb(0x3D5, static_cast<uint8_t>(pos & 0xFF));
    io::outb(0x3D4, 0x0E);
    io::outb(0x3D5, static_cast<uint8_t>((pos >> 8) & 0xFF));
}

} // namespace drivers
