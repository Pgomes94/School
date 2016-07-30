#ifndef _UTIL_H
#define _UTIL_H

#include "types.h"

void *
memset(void *, char, size_t);

unsigned char inportb(unsigned short);

void outportb(unsigned short, unsigned char);

#endif
