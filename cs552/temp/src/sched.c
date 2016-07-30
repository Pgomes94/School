#include "types.h"
#include "thread.h"
#include "sched.h"

extern tcb threads[MAX_THREADS];
int next = -1;

int schedule (void) {
  __asm__ __volatile__ ("cli");
  int sched = -1;
  next = (next + 1) % MAX_THREADS;
  for( ; !allDone(); next = (next+1) % MAX_THREADS) {
    if (threads[next].idle == FALSE) {
      sched = next;
      break;
    }
  }
  __asm__ __volatile__ ("sti");
  return sched;
}

int current_tid(void) {
  return next;
}
