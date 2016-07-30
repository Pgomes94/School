#include "types.h"
#include "print.h"
#include "idt.h"
#include "isrs.h"

void isrs_install() {
    idt_set_gate(0, (unsigned)_isr0, 0x08, 0x8E);
     idt_set_gate(1, (unsigned)_isr1, 0x08, 0x8E);
     idt_set_gate(2, (unsigned)_isr2, 0x08, 0x8E);
     idt_set_gate(3, (unsigned)_isr3, 0x08, 0x8E);
     idt_set_gate(4, (unsigned)_isr4, 0x08, 0x8E);
     idt_set_gate(5, (unsigned)_isr5, 0x08, 0x8E);
     idt_set_gate(6, (unsigned)_isr6, 0x08, 0x8E);
     idt_set_gate(7, (unsigned)_isr7, 0x08, 0x8E);

     idt_set_gate(8, (unsigned)_isr8, 0x08, 0x8E);
     idt_set_gate(9, (unsigned)_isr9, 0x08, 0x8E);
     idt_set_gate(10, (unsigned)_isr10, 0x08, 0x8E);
     idt_set_gate(11, (unsigned)_isr11, 0x08, 0x8E);
     idt_set_gate(12, (unsigned)_isr12, 0x08, 0x8E);
     idt_set_gate(13, (unsigned)_isr13, 0x08, 0x8E);
     idt_set_gate(14, (unsigned)_isr14, 0x08, 0x8E);
     //write("Is the length the problem?", TRUE);
     idt_set_gate(15, (unsigned)_isr15, 0x08, 0x8E);

     idt_set_gate(16, (unsigned)_isr16, 0x08, 0x8E);
     idt_set_gate(17, (unsigned)_isr17, 0x08, 0x8E);
     idt_set_gate(18, (unsigned)_isr18, 0x08, 0x8E);
     idt_set_gate(19, (unsigned)_isr19, 0x08, 0x8E);
     idt_set_gate(20, (unsigned)_isr20, 0x08, 0x8E);
     idt_set_gate(21, (unsigned)_isr21, 0x08, 0x8E);
     idt_set_gate(22, (unsigned)_isr22, 0x08, 0x8E);
     idt_set_gate(23, (unsigned)_isr23, 0x08, 0x8E);
     idt_set_gate(24, (unsigned)_isr24, 0x08, 0x8E);
     idt_set_gate(25, (unsigned)_isr25, 0x08, 0x8E);
     idt_set_gate(26, (unsigned)_isr26, 0x08, 0x8E);
     idt_set_gate(27, (unsigned)_isr27, 0x08, 0x8E);
     idt_set_gate(28, (unsigned)_isr28, 0x08, 0x8E);
     idt_set_gate(29, (unsigned)_isr29, 0x08, 0x8E);
     idt_set_gate(30, (unsigned)_isr30, 0x08, 0x8E);
     idt_set_gate(31, (unsigned)_isr31, 0x08, 0x8E);
}

const char *exception_messages[] = {
   "Division By Zero Exception",
   "Debug Exception",
   "Non Maskable Interrupt Exception",
   "Breakpoint Exception",
   "Into Detected Overflow Exception",
   "Out of Bounds Exception",
   "Invalid Opcode Exception",
   "No Coprocessor Exception",
   "Double Fault Exception",
   "Coprocessor Segment Overrun Exception",
   "Bad TSS Exception",
   "Segment Not Proesent Exception",
   "Stack Fault Exception",
   "General Protection Fault Exception",
   "Page Fault Exception",
   "Unknwon Interrupt Exception",
   "Coprocessor Fault Exception",
   "Alignment Check Exception (486+)",
   "Machine Check Exception (Pentium/586+)",
   "Reserved Exception",
   "Reserved Exception",
   "Reserved Exception",
   "Reserved Exception",
   "Reserved Exception",
   "Reserved Exception",
   "Reserved Exception",
   "Reserved Exception",
   "Reserved Exception",
   "Reserved Exception",
   "Reserved Exception",
   "Reserved Exception",
  "Reserved Exception"
};

/* Handles all Interrupt Service Routines. All we do now is
 * halt the system and print the message. All ISRs disable
 * interrupts while they are being serviced as a locking
 * mechanism to prevent an IRQ from happening and messing
 * up kernel data structures.
 */
void fault_handler(struct iregs *r) {
  if (r->int_no < 32) {
    write(exception_messages[r->int_no], TRUE);
    write("System Halted!", TRUE);
    for(;;);
  }
}
