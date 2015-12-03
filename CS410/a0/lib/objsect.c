#include "../include/objlib.h"

void printMemoryAddress(int mem) {
  char buf[32];
  char *bufp = buf;

  do {
    char digit = '0' + (mem & 0x0f);
    if (digit > '9') {
      digit += 'a' - '0' - 10;
    }
    *bufp++ = digit;
    mem >>= 4;
  } while (mem != 0);
  
  while (bufp > buf) {
    *--bufp;
    write(1, bufp, 1);
  }
}

void writeSectionInfo(bfd *ptr, asection *sect, void *obj){
  write(1, sect->name, strlen(sect->name));
  write(1, "\t", 1);
  write(1, "\t", 1);
  printMemoryAddress(sect->vma);
  write(1, "\t", 1);
  write(1, "\t", 1);
  printMemoryAddress(sect->rawsize);
  write(1, "\t", 1);
  write(1, "\t", 1);
  printMemoryAddress(sect->size);
  write(1, "\t", 1);
  write(1, "\t", 1);
  printMemoryAddress(sect->filepos);
  write(1, "\n", 1);
}

void makeSectionHeader(){
  //header
  write(1, "Name", strlen("Name"));
  write(1, "\t", 1);
  write(1, "\t", 1);
  write(1, "VMA", strlen("VMA"));
  write(1, "\t", 1);
  write(1, "\t", 1);
  write(1, "Rawsize", strlen("Rawsize"));
  write(1, "\t", 1);
  write(1, "\t", 1);
  write(1, "Size", strlen("Size"));
  write(1, "\t", 1);
  write(1, "\t", 1);
  write(1, "FilePos", strlen("FilePos"));
  write(1, "\n", 1);
}
