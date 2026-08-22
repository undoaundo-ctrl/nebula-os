#include "heap.hpp"

namespace mm {

Heap kheap;

void Heap::init(uintptr_t start, size_t size) {
    heap_start = start;
    heap_size = size;
    offset = 0;
}

void* Heap::alloc(size_t size, size_t align) {
    size_t aligned_offset = (offset + (align - 1)) & ~(align - 1);
    if (aligned_offset + size > heap_size) return nullptr; // OOM
    void* ptr = reinterpret_cast<void*>(heap_start + aligned_offset);
    offset = aligned_offset + size;
    return ptr;
}

} // namespace mm

void* operator new(size_t size)   { return mm::kheap.alloc(size); }
void* operator new[](size_t size) { return mm::kheap.alloc(size); }
// Bump allocator: no-op free (adequate for a boot-time kernel demo;
// a real allocator -- slab/buddy -- would replace this).
void operator delete(void*) noexcept {}
void operator delete[](void*) noexcept {}
void operator delete(void*, size_t) noexcept {}
void operator delete[](void*, size_t) noexcept {}
