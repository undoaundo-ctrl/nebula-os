; Nebula OS - boot.asm
; Multiboot2-compliant 32-bit entry point that sets up identity-mapped
; paging and transitions the CPU into x86_64 long mode before jumping
; into the C++ kernel (kernel_main).

MBALIGN     equ 1<<0
MEMINFO     equ 1<<1
FLAGS       equ MBALIGN | MEMINFO
MAGIC       equ 0xE852DB4D          ; multiboot2 magic
ARCH        equ 0                  ; i386 protected mode

section .multiboot2
align 8
mb2_header_start:
    dd MAGIC
    dd ARCH
    dd mb2_header_end - mb2_header_start
    dd -(MAGIC + ARCH + (mb2_header_end - mb2_header_start))

    ; end tag
    dw 0
    dw 0
    dd 8
mb2_header_end:

section .bss
align 16
stack_bottom:
    resb 65536                     ; 64 KiB stack
stack_top:

align 4096
p4_table: resb 4096
p3_table: resb 4096
p2_table: resb 4096

section .text
bits 32
global _start
extern kernel_main_trampoline

_start:
    cli
    mov esp, stack_top
    mov edi, ebx                   ; save multiboot2 info pointer
    mov esi, eax                   ; save multiboot2 magic

    call check_multiboot
    call check_cpuid
    call check_long_mode

    call set_up_page_tables
    call enable_paging

    lgdt [gdt64.pointer]
    jmp gdt64.code_seg:long_mode_start

    hlt

; ---- sanity checks ----
check_multiboot:
    cmp esi, 0x36d76289
    jne .no_multiboot
    ret
.no_multiboot:
    mov al, "0"
    jmp error

check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    cmp eax, ecx
    je .no_cpuid
    ret
.no_cpuid:
    mov al, "1"
    jmp error

check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    ret
.no_long_mode:
    mov al, "2"
    jmp error

; ---- identity map first 1 GiB with 2 MiB pages ----
set_up_page_tables:
    mov eax, p3_table
    or eax, 0b11
    mov [p4_table], eax

    mov eax, p2_table
    or eax, 0b11
    mov [p3_table], eax

    xor ecx, ecx
.map_p2:
    mov eax, 0x200000
    mul ecx
    or eax, 0b10000011              ; present + writable + huge page
    mov [p2_table + ecx * 8], eax
    inc ecx
    cmp ecx, 512
    jne .map_p2
    ret

enable_paging:
    mov eax, p4_table
    mov cr3, eax

    mov eax, cr4
    or eax, 1 << 5                  ; PAE
    mov cr4, eax

    mov ecx, 0xC0000080             ; EFER MSR
    rdmsr
    or eax, 1 << 8                  ; long mode enable
    wrmsr

    mov eax, cr0
    or eax, 1 << 31                 ; paging enable
    mov cr0, eax
    ret

error:
    ; print "ERR: X" to VGA text buffer and halt
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte  [0xb800a], al
    hlt

section .rodata
align 8
gdt64:
    dq 0
.code_seg: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53)  ; exec, code, present, 64-bit
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

bits 64
section .text
long_mode_start:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; edi = multiboot2 info ptr (still valid, zero-extended into rdi)
    mov rsp, stack_top
    call kernel_main_trampoline
.hang:
    hlt
    jmp .hang
