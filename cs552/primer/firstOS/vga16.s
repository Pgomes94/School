
# Test vga settings using real mode
# This dumps various register values used for setting up a video
# mode such as 16 color 640x480		
 
	.globl _start
	
	.code16

_start:
	movw $0x9000, %ax
	movw %ax, %ss
	xorw %sp, %sp

# set video mode	
	movw $0x0003, %ax
	int $0x10

#sequencer	
	movw $0x3c4, %dx
	xorb %al, %al
	movw $5, %cx
1:	
	outb %al, %dx
	incw %dx
	pushw %ax
	inb %dx, %al
	decw %dx

	call print
	
	popw %ax
	incb %al
	loop 1b

	movw $0x0e0d, %ax
	movw $0x07, %bx
	int $0x10
	movw $0x0e0a, %ax
	movw $0x07, %bx
	int $0x10

#attribute controller	
	movw $0x3c0, %dx
	movb $0x10, %al
	movw $4, %cx
1:	
	outb %al, %dx
	incw %dx
	pushw %ax
	inb %dx, %al
	decw %dx
	
	call print
	
	popw %ax
	incb %al
	loop 1b

	movb $0x34, %al
	outb %al, %dx
	incw %dx
	inb %dx, %al

	call print

	movw $0x0e0d, %ax
	movw $0x07, %bx
	int $0x10
	movw $0x0e0a, %ax
	movw $0x07, %bx
	int $0x10

#graphics register
	movw $0x3ce, %dx
	xorb %al, %al
	movw $9, %cx
1:	
	outb %al, %dx
	incw %dx
	pushw %ax
	inb %dx, %al
	decw %dx

	call print
	
	popw %ax
	incb %al
	loop 1b

	movw $0x0e0d, %ax
	movw $0x07, %bx
	int $0x10
	movw $0x0e0a, %ax
	movw $0x07, %bx
	int $0x10

#crt controller	
	movw $0x3d4, %dx
	xorb %al, %al
	movw $25, %cx
1:	
	outb %al, %dx
	incw %dx
	pushw %ax
	inb %dx, %al
	decw %dx

	call print
	
	popw %ax
	incb %al
	loop 1b

	movw $0x0e0d, %ax
	movw $0x07, %bx
	int $0x10
	movw $0x0e0a, %ax
	movw $0x07, %bx
	int $0x10

#misc o/p register
	movw $0x3cc, %dx
	inb %dx, %al

	call print

1:	jmp 1b
	
print:	pushw %dx
	movb %al, %dl
	shrb $4, %al
	cmpb $10, %al
	jge 1f
	addb $0x30, %al
	jmp 2f
1:	addb $55, %al		
2:	movb $0x0E, %ah
	movw $0x07, %bx
	int $0x10

	movb %dl, %al
	andb $0x0f, %al
	cmpb $10, %al
	jge 1f
	addb $0x30, %al
	jmp 2f
1:	addb $55, %al		
2:	movb $0x0E, %ah
	movw $0x07, %bx
	int $0x10
	popw %dx
	ret

# This is going to be in our MBR for Bochs, so we need a valid signature
	.org 0x1FE

	.byte 0x55
	.byte 0xAA

# To test:	
# as --32 vga16.s -o vga16.o
# ld -T vga.ld vga16.o -o vga16
# dd bs=1 if=vga16 of=vga16_test skip=4096 count=512
# bochs -qf bochsrc-vga
	