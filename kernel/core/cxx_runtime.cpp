#include <stdint.h>
#include <stddef.h>

// Minimal C++ freestanding runtime support.

typedef void (*ctor_t)();
extern "C" ctor_t __init_array_start[];
extern "C" ctor_t __init_array_end[];

extern "C" void call_global_constructors() {
    for (ctor_t* ctor = __init_array_start; ctor != __init_array_end; ctor++) {
        (*ctor)();
    }
}

extern "C" void __cxa_pure_virtual() {
    for (;;) asm volatile("cli; hlt");
}

// Stack protector support (compiled without -fstack-protector, but some
// toolchains still reference these symbols; provide harmless stubs).
extern "C" uintptr_t __stack_chk_guard = 0x595e9fbd94fda766;
extern "C" void __stack_chk_fail() {
    for (;;) asm volatile("cli; hlt");
}

void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* p) noexcept;
void operator delete[](void* p) noexcept;
void operator delete(void* p, size_t) noexcept;
void operator delete[](void* p, size_t) noexcept;
