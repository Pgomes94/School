#include "util.h"
#include "idt.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

void
idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
  idt[num].base_low = (base & 0xFFFF);
  idt[num].base_high = (base >> 16) & 0xFFFF;

  idt[num].sel = sel;
  idt[num].always0 = 0;
  idt[num].flags = flags;
}

void
idt_setup() {
  idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
  idtp.base = (unsigned int) &idt;

  /* Clear out the entire IDT, initializing it to zeros */
  memset(&idt, 0, (sizeof(struct idt_entry) * 256));

  /* Add any new ISRs to the IDT here using idt_set_gate */

  idt_init((unsigned int)&idtp);
}
