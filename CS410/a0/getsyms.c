#include "include/objlib.h"

int main(int argc, const char* argv[]) {
  if (argc != 2) {
     return -1;
  } else {
    bfd_init();
    bfd *ptr = bfd_openr(argv[1], "default");
    if (bfd_check_format(ptr, bfd_object) == FALSE) {
      bfd_perror("Error occured");
    } else {
      long storageNeeded = bfd_get_symtab_upper_bound(ptr);
      if (storageNeeded == 0) {
        return;
      } else if (storageNeeded < 0) {
        write(1, "Storage Needed less than 0.\n", 28);
        return;
      } else {
        asymbol **symbolTable = malloc(storageNeeded);
        long numOfSymTables = bfd_canonicalize_symtab(ptr, symbolTable);
        if (numOfSymTables < 0) {
          write(1, "SymTable error.\n", 16);
          return;
        } else {
          makeSymbolHeader();
          int i;
          for (i=0; i < numOfSymTables; i++) {
            writeSymbolInfo(symbolTable[i]);
          }
        }
      }
    }
  }
}

