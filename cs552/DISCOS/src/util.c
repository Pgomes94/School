#include "types.h"

void *
memset(void *dest, char val, size_t count) {
  // Have to typecast to a temp because you can't dereference void*
  unsigned char *temp = (unsigned char *)dest;
  for( ; count != 0; count--) *temp++ = val;
  return dest;
}

unsigned char inportb(unsigned short port) {
  unsigned char rv;
  __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (port));
  return rv;
}

void outportb(unsigned short port, unsigned char data) {
  __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));
}
