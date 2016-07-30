# Patrick Gomes and Michael Gerakis
# CS 552 Operating Systems
# linker for memos-1

OUTPUT_FORMAT("elf32-1386")
OUTPUT_ARCH(i386)
ENTRY(_start)
SECTIONS
{
	.foo : {*(.*)}
}