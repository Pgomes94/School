#ifndef _IDT_H
#define _IDT_H

struct idt_entry {
  unsigned short base_low;
  unsigned short sel;         /* Our kernel segment goes here! */
  unsigned char always0;      /* This will ALWAYS be set to 0! */
  unsigned char flags;        /* Set using the above table     */
  unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
  unsigned short limit;
  unsigned int base;
} __attribute__((packed));

extern void
idt_init(unsigned int);

void
idt_set_gate(unsigned char, unsigned long, unsigned short, unsigned char);

void idt_setup();

#endif
