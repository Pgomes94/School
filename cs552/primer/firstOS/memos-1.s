# Patrick Gomes and Michael Gerakis
# CS 552 Operating Systems
# Memos-1 

	.globl _start 

	.code16 # real mode

our_string1:
	.string "MemOS: Welcome *** System Memory is: "

our_string2:
	.string "MB"

_start:
	movw $0x9000, %ax 	# setting up the stack
	movw %ax, %ss
	xorw %sp, %sp
	cld					# sets direction flag to 0, precautionary

	# init and set cursor
	movw $0x0003, %ax
	int $0x10


do_ec80:
	xor bx, bx # need to clear ebx
	#mov dx, 0x534D4150  # special value
	movw $0xEC80, %ax
	mov cx, 24		# asking for 24 bytes
	int $0x15
	jc short .end # carry flag set, jump to end. there was an error if this happens
	#mov dx, 0x534D4150	# Some BIOSes apparently trash this register?
	#cmp ax, dx
	push ax

print_begin:
	lodsb
	or al, al
	jz print_done
	move ah, 0x0e
	int 0x10
	jmp print_begin

print_done:








# This is going to be in our MBR for Bochs, so we need a valid signature
	.org 0x1FE

	.byte 0x55
	.byte 0xAA