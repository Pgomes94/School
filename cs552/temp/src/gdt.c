#include "gdt.h"

struct gdt_entry gdt[3];     // 3 Entries: NULL, Code, & Data
struct gdt_ptr gp;           // Pointer passed to assembly function

void
create_descriptor(int pos, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran) {
  /* Setup the descriptor base address */
  gdt[pos].base_low = (base & 0xFFFF);
  gdt[pos].base_middle = (base >> 16) & 0xFF;
  gdt[pos].base_high = (base >> 24) & 0xFF;

  /* Setup the descriptor limits */
  gdt[pos].limit_low = (limit & 0xFFFF);
  gdt[pos].granularity = ((limit >> 16) & 0x0F);

  /* Finally, set up the granularity and access flags */
  gdt[pos].granularity |= (gran & 0xF0);
  gdt[pos].access = access;
}

void
gdt_setup() {
  gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
  gp.base = (unsigned int)&gdt;

  create_descriptor(0, 0, 0, 0, 0);
  create_descriptor(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);   // Code Segment
  create_descriptor(2, 0, 0xFFFFFFFF, 0x92, 0xCF);   // Data Segment

  gdt_init((unsigned int)&gp);
}
