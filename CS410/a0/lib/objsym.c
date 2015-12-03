#include "../include/objlib.h"

void writeSymbolInfo(asymbol *symbolTable){
  write(1, symbolTable->name, strlen(symbolTable->name));
  write(1, "\t", 1);
  write(1, "\t", 1);
  printMemoryAddress(symbolTable->value + symbolTable->section->vma);
  write(1, "\n", 1);
}

void makeSymbolHeader() {
  //header
  write(1, "Name", 4);
  write(1, "\t", 1);
  write(1, "\t", 1);
  write(1, "VMA", 3);
  write(1, "\n", 1);
}
