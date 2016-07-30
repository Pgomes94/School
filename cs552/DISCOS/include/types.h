#ifndef _TYPES_H
#define _TYPES_H

#define FALSE 0
#define TRUE 1

#define NOTIMPLEMENTED 501

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;
typedef unsigned long long int uint64;

typedef unsigned char byte;
typedef byte byte_t;

#define MAX_THREADS 3
#define MAX_WAIT 5
#define FDT_LENGTH 1024

typedef signed char sint8, s8;
typedef signed short int sint16, s16;
typedef signed long int sint32, s32;
typedef signed long long int sint64, s64;

#ifndef _SIZE_T
typedef int size_t;
#define _SIZE_T 1
#endif

typedef signed char bool;

typedef unsigned long uint;
typedef signed long sint;

#ifndef _STDINT_
#define _STDINT_
typedef uint8 uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t;
typedef uint64 uint64_t;
#endif

/* Defines what the stack looks like after an ISR was running */
struct iregs {
  unsigned int gs, fs, es, ds;                             /* pushed the segs last */
  unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;     /* pushed by 'pusha' */
  unsigned int int_no, err_code;                           /* our 'push byte #' and ecodes do this */
  unsigned int eip, cs, eflags, useresp, ss;               /* pushed by the processor automatically */
};

typedef struct file_descriptor {
  uint32_t offset;     // Offset into file
  uint16_t inode;    // Position in index node array in file system (0 - 1023)
} fd;

typedef struct registers {
  unsigned int esp;
  unsigned int eip;
} regs;

typedef struct thread_control_block {
  int tid; // thread ID
  bool idle;
  regs state;
  fd file_descriptor_table[FDT_LENGTH];    // File descriptor table for a thread
} tcb;

typedef struct waitqueue {
  void (*task)();
} wq;

#endif
