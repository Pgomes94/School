#include "types.h"
#include "util.h"
#include "sched.h"
#include "print.h"
#include "mem.h"
#include "thread.h"

wq waiting[MAX_WAIT];

int wqHead = 0;
int wqLength = 0;
int rqLength = 0;
extern tcb threads[MAX_THREADS];
extern unsigned int *stacks[MAX_THREADS];
extern void c_s(unsigned int, unsigned int);

bool allDone() {
  int i;
  for (i = 0; i < MAX_THREADS; i++) {
    if (threads[i].idle == FALSE) {
      return FALSE;
    }
  }
  return TRUE;
}

void context_switch(void) {
  int next = schedule();
  set_fdt((fd *)&threads[next].file_descriptor_table);
  c_s(threads[next].state.esp, threads[next].state.eip);
}

void complete_end() {
  __asm__ __volatile__ ("cli");
  int curr = current_tid();
  int i = 0;

  for ( i = 0; i < FDT_LENGTH; i++ ) {
    if (threads[curr].file_descriptor_table[i].inode != EMPTY_FDT_ENTRY) {
      rd_close(threads[curr].file_descriptor_table[i].inode);
    }
  }

  threads[curr].idle = TRUE;
  if (wqLength > 0) {
    rqLength--;
    thread_create(stacks[curr], waiting[wqHead].task);
    rqLength++;
    wqLength--;
    wqHead++;
  } else {
    rqLength--;
  }
  __asm__ __volatile__ ("sti");
  if (!allDone()) {
    context_switch();
  }
  write("We are done", TRUE);
  for(;;);
}

int thread_create(void *s, void *func) {
  int i;
  unsigned int* stack;
  stack = s;
  // find an idle thread
  for (i = 0; i < MAX_THREADS; i++){
      if (threads[i].idle == TRUE){
        rqLength++;
        threads[i].tid = i;
        threads[i].idle = FALSE;
        threads[i].state.esp = (unsigned int) &stack[1023];
        threads[i].state.eip = (unsigned int) func;
        int fd = 0;
        for ( ; fd < 1024; fd++) {
          threads[i].file_descriptor_table[fd].offset = 0;
          threads[i].file_descriptor_table[fd].inode = EMPTY_FDT_ENTRY;
        }
        stack[1023] = (unsigned int)complete_end;
        return i;
    }
  }
  // if it gets here this thread was not created (i.e. wait queue it)
  if (wqLength == MAX_WAIT) {
    write("Wait queue is full.", TRUE);
    return -2;
  }
  waiting[(wqLength + wqHead)%MAX_WAIT].task = func;
  wqLength++;
  return -1;
}
