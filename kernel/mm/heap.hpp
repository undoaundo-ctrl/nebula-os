#pragma once
#include <stddef.h>
#include <stdint.h>

namespace mm {

class Heap {
public:
    void init(uintptr_t start, size_t size);
    void* alloc(size_t size, size_t align = 16);
    size_t used() const { return offset; }
    size_t total() const { return heap_size; }

private:
    uintptr_t heap_start = 0;
    size_t heap_size = 0;
    size_t offset = 0;
};

extern Heap kheap;

} // namespace mm
