#include "serial.hpp"
#include "../include/io.hpp"

namespace drivers {

Serial com1;

void Serial::init(uint16_t port) {
    base = port;
    io::outb(base + 1, 0x00); // disable interrupts
    io::outb(base + 3, 0x80); // enable DLAB
    io::outb(base + 0, 0x03); // divisor low  (38400 baud)
    io::outb(base + 1, 0x00); // divisor high
    io::outb(base + 3, 0x03); // 8 bits, no parity, one stop bit
    io::outb(base + 2, 0xC7); // enable FIFO, clear, 14-byte threshold
    io::outb(base + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

bool Serial::transmit_empty() {
    return (io::inb(base + 5) & 0x20) != 0;
}

void Serial::write_char(char c) {
    while (!transmit_empty()) { }
    io::outb(base, static_cast<uint8_t>(c));
}

void Serial::write(const char* str) {
    for (size_t i = 0; str[i]; i++) {
        if (str[i] == '\n') write_char('\r');
        write_char(str[i]);
    }
}

bool Serial::has_data() {
    return (io::inb(base + 5) & 1) != 0;
}

char Serial::read_char() {
    while (!has_data()) { }
    return static_cast<char>(io::inb(base));
}

} // namespace drivers
