[BITS 32]

; Declare constants used for creating a multiboot header.
MBALIGN     equ  1<<0                   ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                   ; provide memory map
FLAGS       equ  MBALIGN | MEMINFO      ; this is the Multiboot 'flag' field
MAGIC       equ  0x1BADB002             ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)        ; checksum of above, to prove we are multiboot

; Declare a header as in the Multiboot Standard. We put this into a special
; section so we can force the header to be in the start of the final program.
; You don't need to understand all these details as it is just magic values that
; is documented in the multiboot standard. The bootloader will search for this
; magic sequence and recognize us as a multiboot kernel.
section .multiboot
align 4
  dd MAGIC
  dd FLAGS
  dd CHECKSUM

; Currently the stack pointer register (esp) points at anything and using it may
; cause massive harm. Instead, we'll provide our own stack. We will allocate
; room for a small temporary stack by creating a symbol at the bottom of it,
; then allocating 16384 bytes for it, and finally creating a symbol at the top.
section .bootstrap_stack, nobits
align 4
stack_bottom:
  resb 16384
stack_top:

section .bss
ESP_TEMP: resd 1
EIP_TEMP: resd 1
global FILE_SYSTEM
FILE_SYSTEM: resb 2097152  ; 2MB for filesystem


section .text
; Global functions uses in linking and C files
global _start
global gdt_init
global idt_init
global c_s

_start:            ; Initiate stack, move to real_start (where main & stuff is)
  mov esp, stack_top
  extern _init
  call _init
  cli
.hang:
  hlt
  jmp .hang

gdt_init:
  mov eax, [esp+4]
  lgdt [eax]
  ; Reload data segment registers
  mov ax, 0x10      ; 0x10 points at the new data selector
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  jmp 0x08:flush    ; 0x08 points at the new code selector
flush:
  ret

idt_init:
  mov eax, [esp+4]
  lidt [eax]
  ret

c_s:
  mov eax, [esp+4]
  mov dword [ESP_TEMP], eax
  mov eax, [esp+8]
  mov dword [EIP_TEMP], eax
  mov eax, [ESP_TEMP]
  mov esp, eax
  push dword [EIP_TEMP]
  ret
