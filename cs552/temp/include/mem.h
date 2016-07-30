#ifndef MEM_H
#define MEM_H

// ------------------------------------
// DEFINES
// ------------------------------------
#define DIR 0
#define REG 1
#define MAX_FILE_SIZE 1067008
#define MAX_BLOCKS_PER_INODE 4168
#define MEM_SIZE 2097152
// Total inodes and blocks
#define TOTAL_INODES 1024
#define TOTAL_BLOCKS 8192
#define SUPERBLOCK_POS 0
#define INODE_ARRAY_START 1
#define INODE_ARRAY_END 256
#define BITMAP_START 257
#define BITMAP_END 260
#define FREE_BLOCK_START 261
#define UNUSEDBLOCK 0
#define FILENAME_LENGTH 14
#define EMPTY_DIR_ENTRY 1024    // 1024 is past the range of valid inodes so its okay to use as a flag
#define FREE 0
#define IN_USE 1
#define FAIL -1
#define SUCCEED 0
#define EMPTY_FDT_ENTRY 1024    // 1024 is past the range of valid inodes so its okay to use as a flag
#define DIR_ENTRIES_PER_BLOCK 16

// ------------------------------------
// DATA TYPES
// ------------------------------------
// INODE structure
// Size - 64 Bytes
typedef struct inode {
  uint32_t limit;     // Size of all currently allocated blocks
  uint32_t size;            // Size of file/directory
  struct block* location[10];   // Location of blocks
  uint32_t nOpened;         // Number of processes where this file is open (need for unlinking)
  uint8_t type;           // DIR || REG
  byte_t status;
  byte_t padding[10];    // Pad to 64 bytes
  // TODO: place filename here???
} inode_t;

// SUPERBLOCK structure
// Size - 256 Bytes
typedef struct superblock {
  int vInodes;
  int vBlocks;
  int lInodes;        // inodes left
  int lBlocks;        // Blocks left
  byte padding[240];
} superblock_t;

// DIRECTORY structure
// Size - 16 Bytes
typedef struct dir {
  uint16_t inode;    // inode position in inode array
  char filename[FILENAME_LENGTH];
} dir_t;

typedef struct block {
  union {
    byte_t placeholder[256];              // 256Bytes - for uninitialized blocks
    superblock_t meta;                    // 256Bytes
    struct inode iNodeArray[4];           // 256Bytes - 4 inodes in each block of the inode array
    byte_t bitmap[256];                   // 256Byes
    dir_t entries[DIR_ENTRIES_PER_BLOCK]; // 256Bytes - 16 dir entries fit into a block
    char file[256];                       // 256Bytes
    struct block* blocks[64];             // 256 Bytes
  } data;                                 // 252Btes - Only access the field that corresponds to the block type
} block_t;


// ------------------------------------
// FUNCTION PROTOTYPES
// ------------------------------------
// Setup
void set_fdt(fd*);
void init_mem();

// File operations
int rd_creat(char *);
int rd_mkdir(char *);
int rd_open(char *);
int rd_close(int);
int rd_read(int, char*, int);
int rd_write(int, char*, int);
int rd_lseek(int, int);
int rd_unlink(char*);
int rd_readdir(int, char*);

// Test operations
void rd_create(int, int, int);

//void rd_read(int);

#endif
