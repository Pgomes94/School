#include "types.h"
#include "print.h"
#include "util.h"
#include "irqs.h"
#include "sched.h"
#include "thread.h"

int pic_ticks = 0;

extern tcb threads[MAX_THREADS];

extern void resume();

void pic_handler(struct iregs *r) {
  pic_ticks++;
  if (pic_ticks % 800 == 0 && !allDone()) {
    int tid = current_tid();
    int esp;
    __asm__ __volatile__ ("movl %esp, %eax\n\t");
    __asm__ __volatile__ ("movl %%eax, %0" : "=r"(esp));
    threads[tid].state.esp = esp;
    threads[tid].state.eip = (unsigned int) resume;

    outportb(0x20, 0x20);
    context_switch();
    __asm__ __volatile__ (".globl resume\n");
    __asm__ __volatile__ ("resume:\n");
  }
}

void pic_install() {
  unsigned long hz = 1000;                                // 1 tick / mSec
   __asm__ __volatile__ ("cli");                                             // Stop interrupts
   unsigned long divisor = 1193180 / hz;                   // Find the divisor
   outportb(0x43, 0x36);              // Tell PIC we're setting it's frequency
   unsigned char l = (unsigned char)(divisor & 0xFF);      // Bitmap Low byte
   unsigned char h = (unsigned char)(divisor >> 8) & 0xFF; // Bitmap High byte
   outportb(0x40, l);                                      // Send Low Byte!
   outportb(0x40, h);                                      // Send High Byte!
   irq_install_handler(0, pic_handler);  // Install Pointer to the ISRS Table
   __asm__ __volatile__ ("sti"); // Restore interrupts
}

/* This will continuously loop until the given time has
*  been reached */
void sleep(int duration) {
  unsigned int wakeUp = duration + pic_ticks;

  while(pic_ticks < wakeUp);
}
