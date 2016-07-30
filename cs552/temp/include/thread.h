#ifndef _THREAD_H
#define _THREAD_H

bool allDone();

int thread_create(void *, void*);

void context_switch(void);

#endif
