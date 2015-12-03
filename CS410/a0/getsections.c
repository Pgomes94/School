#include "include/objlib.h"

int main(int argc, const char* argv[]) {
  if (argc != 2) {
     return -1;
  } else {
     bfd_init();
     bfd* ptr = bfd_openr(argv[1], "default");
     if (bfd_check_format(ptr, bfd_object) == FALSE){
       bfd_perror("Error occured");
     } else { 
       makeSectionHeader();
       bfd_map_over_sections(ptr, writeSectionInfo, NULL);       
     }
  }
}
