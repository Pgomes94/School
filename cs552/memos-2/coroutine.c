/* 
 * Simple stackless thread example
 * Copyright Rich West, 2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define crBegin static int state=0; switch(state) { case 0:

/*
 * ANSI C provides the __LINE__ macro to return the current source line number
 * We can use the line number as a simple form of programmatic state!
 */
#define crReturn(x) do { state=__LINE__; return x; \
                         case __LINE__:; } while (0)
#define crFinish }

#define MAX_THREADS 2

typedef enum {
  FALSE,
  TRUE
} bool;

typedef struct runqueue {
  int (*task)();
  int tid; // thread ID
  struct runqueue *next;
  struct runqueue *prev;
} rq;

static rq head;
static bool done[MAX_THREADS];

int (*f[MAX_THREADS])(void);

int thread1 (void) {

  int i; 
  static int j;

  crBegin;
  while (1) {
    for (i = 0; i < 10; i++) {
      printf ("<1>");
      usleep (100000);
      fflush (stdout);
    }
    printf ("\n");
    crReturn (1); // Let's cooperate and yield

    if (++j == 6)
      break;
  }
  done[0] = TRUE;

  printf ("Done <1>!\n");

  crFinish;

  return 1;
}


int thread2 (void) {

  int i;
  static int j;

  crBegin;
  while (1) {
    for (i = 0; i < 5; i++) {
      printf ("<2>");
      usleep (100000);
      fflush (stdout);
    }
    printf ("\n");
    crReturn (2); // Time to yield

    if (++j == 10)
      break;
  }
  done[1] = TRUE;

  printf ("Done <2>!\n");

  crFinish;

  return 2;
}


void schedule (void) {

  rq *current; // Current thread in runqueue
  rq *finished; // A thread that's finished execution

  int threads = MAX_THREADS;
  
  current = &head;

  while (current) {
    (current->task)();
    
    if (done[current->tid]) { // Remove from runqueue
      
      if (threads == 1) // We've finished last one
	return;

      finished = current;
      finished->prev->next = current->next;
      current = current->next;
      current->prev = finished->prev;

      if (current->next == finished) { // Down to last thread
	current->next = finished->next;
      }

      if (finished != &head)
	free (finished);
      threads--;
    }
    else
      current = current->next;
  }
}


int main (int argc, char *argv[]) {

  int i;
  rq *ptr, *pptr;

  /* Build a "runqueue" */
  f[0] = thread1;
  f[1] = thread2;

  // Setup runqueue head
  head.task = f[0];
  head.tid = 0;
  head.next = NULL;
  head.prev = NULL;

  // Add any additional threads after the first
  for (i = 1; i < MAX_THREADS; i++) {
    ptr = (rq *) malloc (sizeof (rq));

    if (i == 1) {
      head.next = ptr;
      pptr = &head;
    }
    else { 
      pptr = pptr->next;
      pptr->next = ptr;
    }

    ptr->prev = pptr;

    ptr->task = f[i];
    ptr->tid = i;
    ptr->next = &head; // Wraparound
    head.prev = ptr;
  }
  
  // Let's get going...
  schedule ();

  return 0;
}