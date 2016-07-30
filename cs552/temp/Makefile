ASM      := nasm
CC       := gcc
CFLAGS   := -m32 -fno-builtin -fno-stack-protector -fno-strict-aliasing -I include/ -fno-delete-null-pointer-checks -nostdinc -g -Wall -std=c99
PROGS    := discos
CFILES   := $(wildcard src/*.c)
ASMFILES := $(wildcard src/*.asm)
# addprefix adds a prefix (the dir we want to look in)
# notdir extracts everything up to last slash
#  ex. notdir src/init.c  -> init.c
OBJS   := $(addprefix bin/,$(notdir $(CFILES:.c=.o)))  $(addprefix bin/,$(notdir $(ASMFILES:.asm=.o)))


all : $(PROGS)

discos : $(OBJS)
	$(LD) -T discos.ld -o $@ $^

bin/%.o : src/%.asm
	nasm -felf -o $@ $^

bin/%.o : src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

install:
	cp $(PROGS) /mnt/C/boot/grub
	sync

clean :
	rm discos bin/*.o
