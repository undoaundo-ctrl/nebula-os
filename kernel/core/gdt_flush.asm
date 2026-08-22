bits 64
section .text
global gdt_flush

gdt_flush:
    lgdt [rdi]
    mov ax, 0x10        ; kernel data selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    push 0x08            ; kernel code selector
    lea rax, [rel .flush]
    push rax
    retfq
.flush:
    ret
