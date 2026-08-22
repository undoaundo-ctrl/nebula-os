#include "keyboard.hpp"
#include "../include/io.hpp"

namespace drivers {

Keyboard keyboard;

static const char scancode_ascii[128] = {
    0,27,'1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0,' ', 0,
    0,0,0,0,0,0,0,0,0,0, // F1-F10
    0,0, // numlock, scrolllock
    '7','8','9','-','4','5','6','+','1','2','3','0','.',
    0,0,0,0,0
};

static const char scancode_ascii_shift[128] = {
    0,27,'!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,'A','S','D','F','G','H','J','K','L',':','"','~',
    0,'|','Z','X','C','V','B','N','M','<','>','?', 0,
    '*', 0,' ', 0,
};

void Keyboard::push(char c) {
    size_t next = (head + 1) % BUF_SIZE;
    if (next == tail) return; // buffer full, drop
    buffer[head] = c;
    head = next;
}

bool Keyboard::has_key() { return head != tail; }

char Keyboard::pop_key() {
    if (!has_key()) return 0;
    char c = buffer[tail];
    tail = (tail + 1) % BUF_SIZE;
    return c;
}

void Keyboard::irq_handler(core::Registers*) {
    uint8_t sc = io::inb(0x60);

    if (sc == 0x2A || sc == 0x36) { keyboard.shift = true; return; }
    if (sc == 0xAA || sc == 0xB6) { keyboard.shift = false; return; }
    if (sc & 0x80) return; // key release, ignore otherwise

    if (sc < 128) {
        char c = keyboard.shift ? scancode_ascii_shift[sc] : scancode_ascii[sc];
        if (c) keyboard.push(c);
    }
}

void Keyboard::init() {
    core::idt.register_irq_handler(1, &Keyboard::irq_handler);
}

} // namespace drivers
