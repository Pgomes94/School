#ifndef OBJLIB_H
#define OBJLIB_H

#include "bfd.h"
#include "string.h"

void objdump(const char *filename);
void writeSectionInfo(bfd *ptr, asection *sect, void *obj);
void makeSectionHeader();
void printMemoryAddress(int mem);
void nm(const char *filename);
void writeSymbolInfo(asymbol *symbolTable);
void makeSymbolHeader();

#endif
