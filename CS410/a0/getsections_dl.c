#include "include/objlib.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <string.h>

#define rdtsc(x)      __asm__ __volatile__("rdtsc \n\t" : "=A" (*(x)));

// code based on code from http://www.geeksforgeeks.org/implement-itoa/
char* toCharString(long long timeToLog) {
    char strp[64];
    char *str = strp;
    int i;
    for(i = 0; i < 63; i++) { 
      strp[i] = '0';
    }
    strp[63] = '\0';
    i = 62;
    while (timeToLog > 0)
    {
        int rem = timeToLog % 10;
        strp[i--] = (rem > 9) ? (rem-10) + 'a' : rem + '0';
        timeToLog = timeToLog / 10;
    }
    return str;
}

int main(int argc, const char *argv[]){
  unsigned long long start, finish, delta, total;
  void *libPtr;
  if (argc != 3) {
    write(1, "Incorrect arguments.\n", 21);
    return; 
  } else { 
    int fileDescriptor = open("stats", O_WRONLY|O_CREAT|O_APPEND, S_IWUSR|S_IRUSR);
    if (strcmp(argv[2], "RTLD_LAZY") == 0) {
      write(fileDescriptor, "Sections RTLD_LAZY\n", 19);
      int i;
      total = 0;
      for(i = 0; i < 50; i++) { 
        rdtsc(&start);
        libPtr = dlopen("./libobjdata.so", RTLD_LAZY);
        rdtsc(&finish);
        delta = finish - start;
	total += delta;
	char* STR = toCharString(delta);
	write(fileDescriptor, STR, 64);
	write(fileDescriptor, "\n", 1);
      }
      write(fileDescriptor, "Average is: ", 12);
      char *STR = toCharString(total/50);
      write(fileDescriptor, STR, 64);
      write(fileDescriptor, "\n", 1);
    } else if (strcmp(argv[2],"RTLD_NOW") == 0) { 
      write(fileDescriptor, "Sections RTLD_NOW\n", 18);
      int i;
      total = 0;
      for(i = 0; i < 50; i++) { 
        rdtsc(&start);
        libPtr = dlopen("./libobjdata.so", RTLD_NOW);
        rdtsc(&finish);
        delta = finish - start;
        total += delta;
	char* STR = toCharString(delta);
        write(fileDescriptor, STR, 64);
        write(fileDescriptor, "\n", 1);
      }
      write(fileDescriptor, "Average is: ", 12);
      char *STR = toCharString(total/50);
      write(fileDescriptor, STR, 64);
      write(fileDescriptor, "\n", 1);
    } else { 
      write(1, "Incorrect 2nd argument.\n", 24);
      return 0;
    }
    if (libPtr == NULL) { 
      char* error = dlerror();
      write(1, error, strlen(error)); 
    } else { 
      void (*func)(bfd *ptr, asection *sect, void *obj);
      func = dlsym(libPtr, "writeSectionInfo");
      if (dlerror()) { 
        char* error = dlerror();
        write(1, error, strlen(error));
      } else { 
        bfd_init();
        bfd* ptr = bfd_openr(argv[1], "default");
        if (bfd_check_format(ptr, bfd_object) == FALSE){
          bfd_perror("Error occured");
        } else { 
          makeSectionHeader();
          bfd_map_over_sections(ptr, func, NULL);       
        }
      }
    }
    dlclose(libPtr);
  }
}
