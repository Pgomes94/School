#include "types.h"
#include "util.h"
#include "print.h"
#include "gdt.h"
#include "idt.h"
#include "isrs.h"
#include "irqs.h"
#include "pic.h"
#include "thread.h"
#include "mem.h"

// #define's to control what tests are performed,
// comment out a test if you do not wish to perform it

#define TEST1
#define TEST2
#define TEST3
#define TEST4
#define TEST5

#define CREAT   rd_creat
#define OPEN    rd_open
#define WRITE   rd_write
#define READ    rd_read
#define UNLINK  rd_unlink
#define MKDIR   rd_mkdir
#define READDIR rd_readdir
#define CLOSE   rd_close
#define LSEEK   rd_lseek

#define TEST_SINGLE_INDIRECT
#define TEST_DOUBLE_INDIRECT

#define MAX_FILES 524
#define BLK_SZ 256		/* Block size */
#define DIRECT 8		/* Direct pointers in location attribute */
#define PTR_SZ 4		/* 32-bit [relative] addressing */
#define PTRS_PB  (BLK_SZ / PTR_SZ) /* Pointers per index block */

static char pathname[80];

static char data1[DIRECT*BLK_SZ]; /* Largest data directly accessible */
static char data2[PTRS_PB*BLK_SZ];     /* Single indirect data size */
static char data3[PTRS_PB*PTRS_PB*BLK_SZ]; /* Double indirect data size */
static char addr[PTRS_PB*PTRS_PB*BLK_SZ+1]; /* Scratchpad memory */

unsigned int stack1[1024];
unsigned int stack2[1024];
unsigned int stack3[1024];
unsigned int *stacks[MAX_THREADS] = { stack1, stack2, stack3 };
tcb threads[MAX_THREADS];

void blank() {
  return;
}
void thread1 () {
  write("THREAD 1:", TRUE);
  char my_pathname[80];
  my_pathname[0] = '/';
  my_pathname[1] = 'f';
  my_pathname[2] = 'i';
  my_pathname[3] = 'l';
  my_pathname[4] = 'e';
  my_pathname[5] = '_';
  my_pathname[6] = '1';
  my_pathname[7] = '_';
  int i, retval;
  char status[15];
  for ( i = 0; i < 300; i++) {
    itoa((my_pathname+8), 'd', i);

    retval = CREAT (my_pathname);
    if (retval < 0) {
      //write("rd_creat: File creation error in thread 1! status: ", TRUE);
      itoa(status, 'd', retval);
      write(status, TRUE);
    }
  }
}


void thread2 () {
  write("THREAD 2:", TRUE);
  char my_pathname[80];
  my_pathname[0] = '/';
  my_pathname[1] = 'f';
  my_pathname[2] = 'i';
  my_pathname[3] = 'l';
  my_pathname[4] = 'e';
  my_pathname[5] = '_';
  my_pathname[6] = '2';
  my_pathname[7] = '_';
  int i, retval;
  char status[15];
  for ( i = 0; i < 15; i++) {
    itoa((my_pathname+8), 'd', i);

    retval = CREAT (my_pathname);
    if (retval < 0) {
      write("rd_creat: File creation error in thread 2! status: ", TRUE);
      itoa(status, 'd', retval);
      write(status, TRUE);
    }
  }
}

void _init() {
  terminal_initialize();
  gdt_setup();
  idt_setup();
  isrs_install();
  irq_install();
  pic_install();
  init_mem();
  // Set an fdt for main
  fd global_file_descriptor_table[FDT_LENGTH];
  int i;
  for ( i = 0; i < 1024; i++) {
    global_file_descriptor_table[i].offset = 0;
    global_file_descriptor_table[i].inode = EMPTY_FDT_ENTRY;
  }
  set_fdt((fd *)global_file_descriptor_table);

  for (i = 0; i < MAX_THREADS; i++) {
    threads[i].idle = TRUE;
  }

  // Bare thread to provide failed tests something to 'drop' into
  thread_create(stacks[0], blank);

  memset (data1, '1', sizeof (data1));
  memset (data2, '2', sizeof (data2));
  memset (data3, '3', sizeof (data3));

  int retval;
  int fd;
  int index_node_number;
  char status[15];

  #ifdef TEST1

  /* ****TEST 1: MAXIMUM file creation**** */
  pathname[0] = '/';

  /* Generate MAXIMUM regular files */
  for (i = 0; i < MAX_FILES + 1; i++) { // go beyond the limit
    itoa((pathname+1), 'd', i);
    retval = CREAT (pathname);

    if (retval < 0) {
      write("rd_creat: File creation error! status: ", TRUE);
      itoa(status, 'd', retval);
      write(status, TRUE);

      if (i != MAX_FILES) {
       write("Failed test 1.", TRUE);
       context_switch();
     }
    }

    memset(pathname+1, 0, 79);
  }

  /* Delete all the files created */
  for (i = 0; i < MAX_FILES; i++) {
    itoa((pathname+1), 'd', i);

    retval = UNLINK (pathname);

    if (retval < 0) {
      write("rd_unlink: File unlink error! status: ", TRUE);
      itoa(status, 'd', retval);
      write(status, TRUE);

      write("Failed test 1.", TRUE);
      context_switch();
    }

    memset(pathname+1, 0, 79);
  }

#endif // TEST1

#ifdef TEST2

  /* ****TEST 2: LARGEST file size**** */

  /* Generate one LARGEST file */
  retval = CREAT ("/bigfile");

  if (retval < 0) {
    write("rd_creat: File creation error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 2.", TRUE);
    context_switch();
  }

  retval =  OPEN ("/bigfile"); /* Open file to write to it */

  if (retval < 0) {
    write("rd_open: File open error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 2.", TRUE);
    context_switch();
  }

  fd = retval;      /* Assign valid fd */

  /* Try writing to all direct data blocks */
  retval = WRITE (fd, data1, sizeof(data1));

  if (retval < 0) {
    write("rd_write: File write error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 2 at direct.", TRUE);
    context_switch();
  }

#ifdef TEST_SINGLE_INDIRECT

  /* Try writing to all single-indirect data blocks */
  retval = WRITE (fd, data2, sizeof(data2));

  if (retval < 0) {
    write("rd_write: File write error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 2 at single-indirect.", TRUE);
    context_switch();
  }

#ifdef TEST_DOUBLE_INDIRECT

  /* Try writing to all double-indirect data blocks */
  retval = WRITE (fd, data3, sizeof(data3));

  if (retval < 0) {
    write("rd_write: File write error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 2 at double indirect.", TRUE);
    context_switch();
  }

#endif // TEST_DOUBLE_INDIRECT

#endif // TEST_SINGLE_INDIRECT

#endif // TEST2

#ifdef TEST3

  /* ****TEST 3: Seek and Read file test**** */
  retval = LSEEK (fd, 0); /* Go back to the beginning of your file */

  if (retval < 0) {
    write("rd_lseek: File seek error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 3 at first seek.", TRUE);
    context_switch();
  }

  /* Try reading from all direct data blocks */
  retval = READ (fd, addr, sizeof(data1));

  if (retval < 0) {
    write("rd_read: File read error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 3 block read.", TRUE);
    context_switch();
  }
  /* Should be all 1s here... */
  write("Checking data at addr: ", TRUE);
  for (i = 0; i < sizeof(data1); i++) {
    if (addr[i] != '1') {
      write("Failed test 3, addr does not contain all 1's", TRUE);
      context_switch();
    }
  }
  write("Guaranteed all 1's.", TRUE);

#ifdef TEST_SINGLE_INDIRECT

  /* Try reading from all single-indirect data blocks */
  retval = READ (fd, addr, sizeof(data2));

  if (retval < 0) {
    write("rd_read: File read STAGE2 error! status:  ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 3 at single indirect read.", TRUE);
    context_switch();
  }
  /* Should be all 2s here... */
  write("Checking data at addr: ", TRUE);
  for (i = 0; i < sizeof(data1); i++) {
    if (addr[i] != '2') {
      write("Failed test 3, addr does not contain all 2's", TRUE);
      context_switch();
    }
  }
  write("Guaranteed all 2's.", TRUE);

#ifdef TEST_DOUBLE_INDIRECT

  /* Try reading from all double-indirect data blocks */
  retval = READ (fd, addr, sizeof(data3));

  if (retval < 0) {
    write("rd_read: File read STAGE3 error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 3 at double-indirect.", TRUE);
    context_switch();
  }
  /* Should be all 3s here... */
  write("Checking data at addr: ", TRUE);
  for (i = 0; i < sizeof(data1); i++) {
    if (addr[i] != '3') {
      write("Failed test 3, addr does not contain all 3's", TRUE);
      context_switch();
    }
  }
  write("Guaranteed all 3's.", TRUE);

#endif // TEST_DOUBLE_INDIRECT

#endif // TEST_SINGLE_INDIRECT

  /* Close the bigfile */
  retval = CLOSE(fd);

  if (retval < 0) {
    write("rd_close: File close error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 3 at close.", TRUE);
    context_switch();
  }

  /* Remove the biggest file */

  retval = UNLINK ("/bigfile");

  if (retval < 0) {
    write("rd_unlink: /bigfile file deletion error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 3 at unlink.", TRUE);
    context_switch();
  }

#endif // TEST3

#ifdef TEST4

  /* ****TEST 4: Make directory and read directory entries**** */
  retval = MKDIR ("/dir1");

  if (retval < 0) {
    write("rd_mkdir: Directory 1 creation error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 4 at mkdir.", TRUE);
    context_switch();
  }

  retval = MKDIR ("/dir1/dir3");

  if (retval < 0) {
    write("rd_mkdir: Directory 2 creation error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 4 at mkdir subdirectory.", TRUE);
    context_switch();
  }

  retval = MKDIR ("/dir1/dir4");

  if (retval < 0) {
    write("rd_mkdir: Directory 3 creation error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 4 at mkdir double subdirectory.", TRUE);
    context_switch();
  }

  retval =  OPEN ("/dir1"); /* Open directory file to read its entries */

  if (retval < 0) {
    write("rd_open: Directory open error! status: ", TRUE);
    itoa(status, 'd', retval);
    write(status, TRUE);

    write("Failed test 4 at open subdirectory.", TRUE);
    context_switch();
  }

  fd = retval;      /* Assign valid fd */

  memset (addr, 0, sizeof(addr)); /* Clear scratchpad memory */

  while ((retval = READDIR (fd, addr))) { /* 0 indicates end-of-file */

    if (retval < 0) {
      write("rd_readdir: Directory read error! status: ", TRUE);
      itoa(status, 'd', retval);
      write(status, TRUE);

      write("Failed test 4 at mkdir read directory.", TRUE);
      context_switch();
    }

    write("Contents at addr: ", FALSE);
    write(addr, TRUE);
  }
#endif // TEST4

#ifdef TEST5
  /* ****TEST 5: 2 process test**** */
  thread_create(stacks[1], thread1);
  thread_create(stacks[2], thread2);
#endif // TEST5
  __asm__ __volatile__ ("sti");
  context_switch();
}
