#include "types.h"
#include "print.h"
#include "mem.h"

extern block_t FILE_SYSTEM[TOTAL_BLOCKS];

fd* current_fdt;

// Helper functions
void print_inode(int);

bool available_inodes();
bool available_blocks();

int path(char *, unsigned int *);
int verify(char*, int, bool);
void find_and_remove(int, char*);
bool equals(char *, char*);  // string comparison
int curr_loc, next_loc, curr_indirection, next_indirection, curr_doubleIndirection, next_doubleIndirection;
inode_t* loop;
int inode_loop(inode_t*, block_t**, int);

dir_t* find_open_entry(int, char*);
int find_open_block(void);
int allocate_block(int, int);
int find_open_inode(void);
void create_dir_entry(dir_t*, int, char*, int);
void create_empty_inode(int, uint8_t);
void initialize_block(int, int);

void set_fdt(fd* dest) {
  __asm__ __volatile__ ("\tpushf\n\tcli");
  current_fdt = dest;
  __asm__ __volatile__ ("popf");
}

int rd_creat(char *pathname) {
  __asm__ __volatile__ ("cli");

  // check superblock for available blocks
  char *filename;

  if (!available_inodes()) {
    __asm__ __volatile__ ("sti");
    return -1;
  }
  int dir_inode = path(pathname, &filename);

  if (dir_inode == -1) {
    write("The directory does not exist!", TRUE);
    __asm__ __volatile__ ("sti");
    return -1;
  }

  dir_t* dirEntry;  // The entry where we will store the new file

  dirEntry = find_open_entry(dir_inode, filename);
  if ( dirEntry == -1 ) {
    write("The name is not available!", TRUE);
    __asm__ __volatile__ ("sti");
    return -1;
  }

  if ( dirEntry == 0 ) {
    write("No space available!", TRUE);
    __asm__ __volatile__ ("sti");
    return -1;
  }

  int inode = find_open_inode();

  create_dir_entry(dirEntry, inode, filename, dir_inode);
  create_empty_inode(inode, REG);

  __asm__ __volatile__ ("sti");
  return 0;
}

int rd_mkdir(char *pathname) {
  __asm__ __volatile__ ("cli");

  // check superblock for available blocks
  char *filename;

  if (!available_inodes()) return -1;

  int dir_inode = path(pathname, &filename);

  if (dir_inode == -1) {
    write("The directory does not exist!", TRUE);
    __asm__ __volatile__ ("sti");
    return -1;
  }

  dir_t* dirEntry;  // The entry where we will store the new file

  dirEntry = find_open_entry(dir_inode, filename);
  if ( dirEntry == -1 ) {
    write("The name is not available!", TRUE);
    __asm__ __volatile__ ("sti");
    return -1;
  }

  if ( dirEntry == 0 ) {
    write("No space available!", TRUE);
    __asm__ __volatile__ ("sti");
    return -1;
  }

  int inode = find_open_inode();

  create_dir_entry(dirEntry, inode, filename, dir_inode);
  create_empty_inode(inode, DIR);

  __asm__ __volatile__ ("sti");
  return 0;
}

int rd_open(char *pathname) {
  __asm__ __volatile__ ("cli");

  char *filename;
  int dir_inode;
  int inode;
  if (*pathname == '/' && *(pathname + 1) == 0) {
    inode = 0;
    dir_inode = -2;
  } else {
    dir_inode = path(pathname, &filename);
  }

  if (dir_inode == FAIL) {
    write("The path does not exist!", TRUE);
    __asm__ __volatile__ ("sti");
    return -1;
  }

  // Make sure the file exists && keep track of its inode, return -1 otherwise
  if ( dir_inode != -2) {
    inode = verify(filename, dir_inode, FALSE);
  }
  if (inode == FAIL) {
    __asm__ __volatile__ ("sti");
    return FAIL;
  }

  // Mark file inode as opened (nOpened++)
  FILE_SYSTEM[inode / 4 + 1].data.iNodeArray[inode % 4].nOpened++;

  // find available stop in current_fdt
  int i, pos;
  pos = -1;
  for ( i = 0; i < FDT_LENGTH; i++ ) {
    if ((current_fdt + i) -> inode == EMPTY_FDT_ENTRY) {
      pos = i;
      // put into current_fdt the file/dir inode && offset (0)
      (current_fdt + pos) -> inode = inode;
      (current_fdt + pos) -> offset = 0;
      break;
    }
  }

  if (pos == -1) {
    write("The fdt is full!", TRUE);
  }

  __asm__ __volatile__ ("sti");
  // return the fd_t pos
  return pos;
}

int rd_close(int fd) {
  __asm__ __volatile__ ("cli");
  int result;
  if ((current_fdt+fd) -> inode == EMPTY_FDT_ENTRY) {
    write("Invalid fd.", TRUE);
    result = FAIL;
  } else {
    int inode = (current_fdt+fd) -> inode;
    (current_fdt+fd) -> inode = EMPTY_FDT_ENTRY;
    (current_fdt+fd) -> offset = 0;
    FILE_SYSTEM[inode / 4 + 1].data.iNodeArray[inode % 4].nOpened--;
    result = SUCCEED;
  }

  __asm__ __volatile__ ("sti");
  return result;
}

int rd_read(int fd, char* buf, int bytes) {
  __asm__ __volatile__ ("cli");
  int result;
  int inode = (current_fdt) -> inode;
  // Check for valid file descriptor
  if ( inode == EMPTY_FDT_ENTRY ) {
    write("Invalid fd.", TRUE);
    result = FAIL;
  } else {
    inode_t* ptr = &FILE_SYSTEM[inode / 4 + 1].data.iNodeArray[inode % 4];

    // Make sure the inode is not a directory
    if ( ptr -> type == DIR ) {
      write("Can't use rd_read on a directory.", TRUE);
      result = FAIL;
    } else {
      // Now we can start writing to the blocks
      int read = 0; // How many bytes we write

      // First, get to the block that our current offset is in
      int offset = (current_fdt) -> offset;
      int initial_offset = offset;
      int block_offset = offset / 256;  // How many blocks we must go into to reach offset
      block_t* block_ptr;
      int block = inode_loop(ptr, &block_ptr, -1);

      // Get to our offset
      int i = 0;
      for (i = 0; i < block_offset; i++ ) {
        block = inode_loop(ptr, &block_ptr, block); // this returns unusued, what if last block used has space?
      }

      // Continue read from the block as long as we haven't reached the end
      // of the file, or all the necessary bytes are read
      while ( read != bytes && (read + initial_offset) < ptr -> size ) { //}&& block_ptr -> data.file[offset % 252] != '\0') {
        // Write a char into the block, incrementing both offset and written
        buf[read++] = block_ptr -> data.file[offset++ % 256];
        // If we reached our limit for the block, we must allocate a new block
        if ( offset % 256 == 0 ) {
          // Set our block pointer to this new block
          block = inode_loop(ptr, &block_ptr, block);
        }
      }
      // Update our fdt entry
      (current_fdt) -> offset = offset;
      // Return the amount written to
      result = read;
    }
  }
  __asm__ __volatile__ ("sti");
  return result;
}

int rd_write(int fd, char* buf, int bytes) {
  __asm__ __volatile__ ("cli");
  int result;
  int inode = (current_fdt) -> inode;
  // Check for valid file descriptor
  if ( inode == EMPTY_FDT_ENTRY ) {
    write("Invalid fd.", TRUE);
    result = FAIL;
  } else {
    inode_t* ptr = &FILE_SYSTEM[inode / 4 + 1].data.iNodeArray[inode % 4];

    // Make sure the inode is not a directory
    if ( ptr -> type == DIR || ptr -> size == MAX_FILE_SIZE ) {
      write("Can't use rd_write on a directory.", TRUE);
      result = FAIL;
    } else {
      // Now we can start writing to the blocks
      int written = 0; // How many bytes we write

      // First, get to the block that our current offset is in
      int offset = (current_fdt) -> offset;
      int initial_offset = offset;
      int block_offset = offset / 256;  // How many blocks we must go into to reach offset
      block_t* block_ptr = 0;
      int block = -1;
      // Get to our offset
      int i = 0;
      for (i = 0; i < block_offset; i++ ) {
        block = inode_loop(ptr, &block_ptr, block); // this returns unusued, what if last block used has space?
      }
      // Continue writing into the block as long as we haven't reached our max
      // file size and the specified number of writes hasn't been written
      while ( ptr -> size < MAX_FILE_SIZE && written < bytes ) {
        // If we reached our limit for the block, we must allocate a new block
        if (ptr -> size == ptr -> limit ) {
          int blocknum = find_open_block();
          if (blocknum == -1) {
            return 0;
          }
          int err = allocate_block(inode, blocknum);
          if ( err == -1 ) {
            return -1;
          }
          initialize_block(inode, blocknum);

          block = inode_loop(ptr, &block_ptr, block);
        }

        // Write a char into the block, incrementing both offset and written
        block_ptr -> data.file[offset++ % 256] = buf[written++];

        // Increase file size
        ptr -> size++;
      }
      // Update our fdt entry
      (current_fdt) -> offset = offset;
      // Return the amount written to
      result = written;
    }
  }
  __asm__ __volatile__ ("sti");
  return result;
}

int rd_lseek(int fd, int offset) {
  __asm__ __volatile__ ("cli");
  int result;
  int inode = (current_fdt+fd) -> inode;
  if (inode == EMPTY_FDT_ENTRY) {
    write("Invalid fd.", TRUE);
    result = FAIL;
  } else {
    inode_t* ptr = &FILE_SYSTEM[inode / 4 + 1].data.iNodeArray[inode % 4];
    if ( ptr -> type == DIR ) {
      write("Can't use rd_lseek on a directory.", TRUE);
      result = FAIL;
    } else {
      if ( offset > ptr -> size ) {
        offset = ptr -> size;
      }
      (current_fdt+fd) -> offset = offset;
    }
  }
  __asm__ __volatile__ ("sti");
  return result;
}

int rd_unlink(char* pathname) {
  __asm__ __volatile__ ("cli");
  int result;
  char* filename;
  int parent_inode = path(pathname, &filename);
  if (parent_inode == FAIL) {
    write("The path does not exist!", TRUE);
    result = FAIL;
  } else {
    // Make sure the file exists && keep track of its inode, return -1 otherwise
    int inode = verify(filename, parent_inode, FALSE);
    if (inode == FAIL) {
      result = FAIL;
    } else {
      inode_t* ptr = &FILE_SYSTEM[inode / 4 + 1].data.iNodeArray[inode % 4];

      // If we have a file that is opened, cannot unlinking
      if ( ptr -> nOpened != 0 ) {
        write("Cannot unlink an open file.", TRUE);
        result = FAIL;
      } else {
        if (ptr -> type == DIR && ptr -> size != 0) {
          write("Cannot unlink a directory that is not empty.", TRUE);
          result = FAIL;
          // free all of its blocks
          // zero out inode
          // set status to free
        } else {
          block_t* block_ptr;
          int block = inode_loop(ptr, &block_ptr, -1);
          int start = &FILE_SYSTEM;
          int blocknum;
          while (block != -1) {
            blocknum = (int) (block_ptr - start) / 256;
            free_block(blocknum);
            block = inode_loop(ptr, &block_ptr, block);
          }
          ptr -> type = 0;
          ptr -> limit = 0;
          ptr -> size = 0;
          int l;
          for ( l = 0; l < 10; l++ ) {
            ptr -> location[l] = UNUSEDBLOCK;
          }
          ptr -> nOpened = 0;
          ptr -> status = FREE;
          FILE_SYSTEM[SUPERBLOCK_POS].data.meta.lInodes++;
          find_and_remove(parent_inode, filename);
        }
      }
    }
  }

  __asm__ __volatile__ ("sti");
  return result;
}

int rd_readdir(int fd, char* buf) {
  __asm__ __volatile__ ("cli");
  int result;
  int inode = (current_fdt) -> inode;
  // Check for valid file descriptor
  if ( inode == EMPTY_FDT_ENTRY ) {
    write("Invalid fd.", TRUE);
    result = FAIL;
  } else {
    inode_t* ptr = &FILE_SYSTEM[inode / 4 + 1].data.iNodeArray[inode % 4];

    // Make sure the inode is a directory
    if ( ptr -> type == REG ) {
      write("Can't use rd_readdir on a directory.", TRUE);
      result = FAIL;
    } else {
      // Now we can start reading entries
      int offset = (current_fdt) -> offset;

      // First, make sure we haven't already read everything // the size isn't zero
      if ( ptr -> size == 0 || offset * 16 == (ptr -> limit) ) {
        result = 0;
      } else {
        // Get to the block that our curren offset is in
        int block_offset = offset / DIR_ENTRIES_PER_BLOCK;
        block_t* block_ptr;
        int block = inode_loop(ptr, &block_ptr, -1);
        // Get to our offset
        int i;
        for ( i = 0; i < block_offset; i++ ) {
          block = inode_loop(ptr, &block_ptr, block);
        }

        while (block_ptr -> data.entries[offset % DIR_ENTRIES_PER_BLOCK].inode == EMPTY_DIR_ENTRY) {
          offset++;
          if ( offset % DIR_ENTRIES_PER_BLOCK == 0 ) {
            block = inode_loop(ptr, &block_ptr, block);
          }
        }
        // Read one entry from the directory
        for ( i = 0; i < DIR_ENTRIES_PER_BLOCK; i++ ) {
          if ( block_ptr -> data.entries[offset % DIR_ENTRIES_PER_BLOCK].filename[i] != '\0' ) {
            buf[i] = block_ptr -> data.entries[offset % DIR_ENTRIES_PER_BLOCK].filename[i];
          } else {
            buf[i] = ' ';
            itoa(&buf[i+1], 'd', block_ptr -> data.entries[offset % DIR_ENTRIES_PER_BLOCK].inode);
            break;
          }
        }
        offset++;

        (current_fdt) -> offset = offset;
        result = 1;
      }
    }
  }
  __asm__ __volatile__ ("sti");
  return result;
}

void init_mem() {
  int i;
  int j;
  // Set up superblock
  i = SUPERBLOCK_POS;
  FILE_SYSTEM[i].data.meta.vInodes = TOTAL_INODES;
  FILE_SYSTEM[i].data.meta.vBlocks = TOTAL_BLOCKS;
  FILE_SYSTEM[i].data.meta.lInodes = TOTAL_INODES - 1;
  FILE_SYSTEM[i].data.meta.lBlocks = TOTAL_BLOCKS;

  // Set up iNodeArray
  for (i = INODE_ARRAY_START; i <= INODE_ARRAY_END; i++) {
    for ( j = 0; j < 4; j++ ) {
      FILE_SYSTEM[i].data.iNodeArray[j].status = FREE;
    }
  }

  // Put the root in inode 0
  FILE_SYSTEM[INODE_ARRAY_START].data.iNodeArray[0].type = DIR;
  FILE_SYSTEM[INODE_ARRAY_START].data.iNodeArray[0].nOpened = 0;
  FILE_SYSTEM[INODE_ARRAY_START].data.iNodeArray[0].size = 0;
  FILE_SYSTEM[INODE_ARRAY_START].data.iNodeArray[0].limit = 256;
  FILE_SYSTEM[INODE_ARRAY_START].data.iNodeArray[0].status = IN_USE;
  FILE_SYSTEM[INODE_ARRAY_START].data.iNodeArray[0].location[0] = &FILE_SYSTEM[FREE_BLOCK_START];

  // Don't think we need to do this explicitally since its in the bss
  int l;
  for ( l = 1; l < 10; l++ ) {
    FILE_SYSTEM[INODE_ARRAY_START].data.iNodeArray[0].location[l] = UNUSEDBLOCK;
  }

  // Set up bitmap
  for (i = BITMAP_START; i <= BITMAP_END; i++) {
    for (j = 0; j < 256; j++) {
      FILE_SYSTEM[i].data.bitmap[j] = ~0xFF;
    }
  }
  // First 261 blocks are taken
  for ( i = 0; i < 261; i++ ) {
    find_open_block();
  }

  // Set up first block to be directory block
  for (i = 0; i < DIR_ENTRIES_PER_BLOCK; i ++) {
    FILE_SYSTEM[FREE_BLOCK_START].data.entries[i].inode =  EMPTY_DIR_ENTRY;
  }
  int blocknum = find_open_block()  ;
}

void print_inode(int inode) {
  char memstr[25];
  int block = (inode / 4) + 1; // Offset by one because of superblock
  int offset = inode % 4;
  write("Inode ", FALSE);
  itoa(memstr, 'd', inode);
  write(memstr, FALSE);
  write(" located at ", FALSE);
  itoa(memstr, 'd', &FILE_SYSTEM[block].data.iNodeArray[offset]);
  write(memstr, FALSE);
  write(". Size is - ", FALSE);
  itoa(memstr, 'd', FILE_SYSTEM[block].data.iNodeArray[offset].size);
  write(memstr, FALSE);
  write(" limit is - ", FALSE);
  itoa(memstr, 'd', FILE_SYSTEM[block].data.iNodeArray[offset].limit);
  write(memstr, FALSE);
  write(" Location is ", FALSE);
  itoa(memstr, 'd', FILE_SYSTEM[block].data.iNodeArray[offset].location[0]);
  write(memstr, FALSE);
  write(" Type is ", FALSE);
  itoa(memstr, 'd', FILE_SYSTEM[block].data.iNodeArray[offset].type);
  write(memstr, FALSE);
  write(" Status is ", FALSE);
  itoa(memstr, 'd', FILE_SYSTEM[block].data.iNodeArray[offset].status);
  write(memstr, TRUE);
}

bool available_inodes() {
  if (FILE_SYSTEM[SUPERBLOCK_POS].data.meta.lInodes == 0) {
    return FALSE;
  }

  return TRUE;
}

bool available_blocks() {
  if (FILE_SYSTEM[SUPERBLOCK_POS].data.meta.lBlocks == 0) {
    return FALSE;
  }

  return TRUE;
}

// Find the directory a pathname falls under, and return the directories inode.
// Point filename to start of filename
// return -1 if failure
int path(char *pathname, unsigned int *filename_addr) {
  if (*pathname != '/') {
    write("Paths must start with root directory \'/\'.", TRUE);
    return FAIL;
  }

  if (*(pathname + 1) == 0) {
    write("Cannot create or delete \'/\'.", TRUE);
    return FAIL;
  }

  char *start, *finish;
  start = finish = pathname + 1;
  int parent_inode = 0;      // Parent directory inode always starts at 0 (root)

  for ( ; *finish != 0; finish++) {
    if (*finish == '/') {
      *finish = 0;      // temporarily end string

      // verify that start - finish (which will be start of dir name to the 0 we
      // just placed) is a directory under the current parent directory
      parent_inode = verify(start, parent_inode, TRUE);
      if (parent_inode == -1) {
        write("Invalid path.", TRUE);
        return FAIL;
      }

      *finish = '/';     // undo change
      start = ++finish;  // Set start to start of next directory/file
    }
  }

  *filename_addr = start;
  return parent_inode;
}

// Verify that the directory exists
// If it does, return its inode else -1
int verify(char *dirname, int t, bool requireDir) {
  // Find block and offset of the inode
  int block = (t / 4) + 1; // Offset by one because of superblock
  int offset = t % 4;

  int size = FILE_SYSTEM[block].data.iNodeArray[offset].size;

  if (FILE_SYSTEM[block].data.iNodeArray[offset].type != DIR || size == 0) {
    write("Given parent inode represents a file not a directory.", TRUE);
    return FAIL;
  }

  // point to first directory
  inode_t* inode = &FILE_SYSTEM[block].data.iNodeArray[offset];
  block_t* block_ptr;
  int fuck = inode_loop(inode, &block_ptr, -1);
  dir_t* dirEntry;

  while ( fuck != -1) {
    dirEntry = block_ptr -> data.entries;
    int i;
    // As long as there
    for ( i = 0; i < DIR_ENTRIES_PER_BLOCK; i++ ) {
      if ( (dirEntry + i) -> inode == EMPTY_DIR_ENTRY ) continue;
      if (equals(dirname, (dirEntry + i) -> filename)) {
        // Make sure it is an directory
        if (requireDir) {
          if (FILE_SYSTEM[block].data.iNodeArray[offset].type == DIR) {
            return (dirEntry + i) -> inode;
          } else {
            write(dirname, FALSE);
            write(" is a file not a directory!", TRUE);
            return FAIL;
          }
        } else {
          return (dirEntry + i) -> inode;
        }
      }
      size -= 16;
      if ( size <= 0 ) {
        write("The target is not present in this directory.", TRUE);
        return FAIL;
      }
    }
    fuck = inode_loop(inode, &block_ptr, fuck);
  }

  return FAIL;
}

bool equals(char* str1, char* str2) {
  int i = 0;
  for ( ; i < FILENAME_LENGTH ; i++ ) {
    if (str1[i] == 0 && str2[i] == 0) {
      break;
    }
    if (str1[i] != str2[i]) {
      return FALSE;
    }
  }
  return TRUE;
}

int curr_block, blockCount;

/*
 * Loop through all blocks in an inode
 * Return -1 if done looping
 * An inode has anywhere from 0 - 4168 blocks. Using curr_block, inode_loop
 * finds the next block in the specified inode, and points block_ptr to it. It
 * then increments curr_block and returns it.
 */

int inode_loop(inode_t* inode, block_t** ptr, int last_block) {
  if ( last_block == inode -> limit / 256 ) return -1;
  curr_block = last_block + 1;
  if ( curr_block >= 0 && curr_block <= 7) {
    *ptr = inode -> location[curr_block];
    return curr_block;
  } else if ( curr_block >= 8 && curr_block <= 71) {
    *ptr = inode -> location[8] -> data.blocks[curr_block - 8];
    return curr_block;
  } else {  // 72 - 4167
    int indirection = (curr_block - 72) / 64;
    int doubleIndirection = (curr_block - 72) % 64;
    *ptr = inode -> location[9] -> data.blocks[indirection] -> data.blocks[doubleIndirection];
    return curr_block;
  }
}

// Loop through blocks in inode && return an open entry
dir_t* find_open_entry(int inode, char* filename) {
  block_t* block_ptr;
  dir_t* dirEntry;
  dir_t* fin;
  fin = 0;

  if (FILE_SYSTEM[(inode / 4) + 1].data.iNodeArray[inode % 4].size == FILE_SYSTEM[(inode / 4) + 1].data.iNodeArray[inode % 4].limit) {
    int blocknum = find_open_block();
    if (blocknum == -1) {
      return 0;
    }
    int err = allocate_block(inode, blocknum);
    if ( err == -1 ) {
      return err;
    }
    initialize_block(inode, blocknum);
  }

  inode_t* ptr = &FILE_SYSTEM[(inode / 4) + 1].data.iNodeArray[inode % 4];
  int block = inode_loop(ptr, &block_ptr, -1);

  while ( block != -1 ) {
    dirEntry = block_ptr -> data.entries;
    int i;
    for ( i = 0; i < DIR_ENTRIES_PER_BLOCK; i++ ) {
      if ( dirEntry[i].inode == EMPTY_DIR_ENTRY ) {
        fin = &dirEntry[i];
      };
      if ( equals(filename, (dirEntry+i)->filename) ) {
        return -1;
      }
    }
    block = inode_loop(ptr, &block_ptr, block);
  }
  // We haven't found any
  return fin;
}

void find_and_remove(int dir_inode, char* filename) {
  block_t* block_ptr;
  dir_t* dirEntry;

  inode_t* inode = &FILE_SYSTEM[(dir_inode / 4) + 1].data.iNodeArray[dir_inode % 4];
  int block = inode_loop(inode, &block_ptr, -1);

  while ( block != -1 ) {
    dirEntry = block_ptr -> data.entries;
    int i;
    for ( i = 0; i < DIR_ENTRIES_PER_BLOCK; i++ ) {
      if ( equals(filename, (dirEntry+i)->filename) ) {
        (dirEntry+i) -> inode = EMPTY_DIR_ENTRY;
        (dirEntry+i) -> filename[0] = '\0';
        return;
      }
    }
    block = inode_loop(inode, &block_ptr, block);
  }
}

int find_open_block() {
  if (!available_blocks()) {
    write("All blocks have already been allocated.", TRUE);
    return -1;
  }
  byte_t* bitmap;
  uint8_t pos;
  int i;
  int j;
  int k;
  for ( i = BITMAP_START; i <= BITMAP_END; i++ ) {
    for ( j = 0; j < 256; j++ ) {
      for ( k = 0 ; k < 8; k++ ) {
        bitmap = &FILE_SYSTEM[i].data.bitmap[j];
        pos = (*bitmap  >> k) & 0x1;
        if ( pos == 0 ) {
          *bitmap |= (0x1 << k);
          FILE_SYSTEM[SUPERBLOCK_POS].data.meta.lBlocks--;
          return (2048 * (i - BITMAP_START) + 8 * j + k);
        }
      }
    }
  }
}

void free_block(int blocknum) {
  int i = blocknum / 2048;
  blocknum = blocknum - i * 2048;
  i = i + BITMAP_START;
  int j = blocknum /8;
  blocknum = blocknum - j * 8;
  int k = blocknum;
  byte_t* bitmap;
  bitmap = &FILE_SYSTEM[i].data.bitmap[j];
  *bitmap ^= (0x1 << k);
  FILE_SYSTEM[SUPERBLOCK_POS].data.meta.lBlocks++;
}

int init_indirection_block() {
  int indirection = find_open_block();
  if (indirection == -1) return -1;
  int i;
  for ( i = 0; i < 64; i++ ) {
    FILE_SYSTEM[indirection].data.blocks[i] = UNUSEDBLOCK;
  }
  return indirection;
}

int allocate_block(int inode, int blocknum) {
  int i;
  inode_t* edit;
  edit = &FILE_SYSTEM[(inode / 4) + 1].data.iNodeArray[inode % 4];

  int next_block = edit -> limit / 256;     // 1 block per 256 bytes

  if ( next_block < 8 ) {
    // this is the basic case.
    edit -> location[next_block] = &FILE_SYSTEM[blocknum];
  } else if ( next_block < 72 ) {
    int indirection = next_block - 8;

    // indirection == 0, meaning we haven't initialized the indirection block
    if ( indirection == 0 ) {
      int indirectionBlock = init_indirection_block();
      if (indirectionBlock == -1) {
        return -1;
      }
      edit -> location[8] = &FILE_SYSTEM[indirectionBlock];

      FILE_SYSTEM[indirectionBlock].data.blocks[0] = &FILE_SYSTEM[blocknum];
    } else {
      edit -> location[8] -> data.blocks[indirection] = &FILE_SYSTEM[blocknum];
    }
  } else if ( next_block < MAX_BLOCKS_PER_INODE ) {
    int indirection = (next_block - 72) / 64;
    int doubleIndirection = (next_block - 72) % 64;

    // We haven't allocated either indirection or double indirection block
    if ( indirection == 0 && doubleIndirection == 0 ) {
      // first insertion into double indirect
      int indirectionBlock = init_indirection_block();
      if (indirectionBlock == -1) {
        return -1;
      }
      int doubleIndirectionBlock = init_indirection_block();
      if (doubleIndirectionBlock == -1) {
        free_block(indirectionBlock);
        return -1;
      }

      edit-> location[9] = &FILE_SYSTEM[indirectionBlock];
      FILE_SYSTEM[indirectionBlock].data.blocks[0] = &FILE_SYSTEM[doubleIndirectionBlock];
      FILE_SYSTEM[doubleIndirectionBlock].data.blocks[0] = &FILE_SYSTEM[blocknum];
    } else if ( doubleIndirection == 0 ) { // We are at the start of a double indirection block
      int doubleIndirectionBlock = init_indirection_block();
      if (doubleIndirectionBlock == -1) {
        return -1;
      }
      edit -> location[9] -> data.blocks[indirection] = &FILE_SYSTEM[doubleIndirectionBlock];
      FILE_SYSTEM[doubleIndirectionBlock].data.blocks[0] = &FILE_SYSTEM[blocknum];
    } else {
      edit -> location[9] -> data.blocks[indirection] -> data.blocks[doubleIndirection] = &FILE_SYSTEM[blocknum];
    }
  } else {
    // next block is beyond the max number of blocks we can give an inode
    return -1;
  }

  FILE_SYSTEM[(inode / 4) + 1].data.iNodeArray[inode % 4].limit += 256;
  return 0;
}

void initialize_block(int inode, int blocknum) {
  int i;
  switch (FILE_SYSTEM[(inode / 4) + 1].data.iNodeArray[inode % 4].type) {
    case DIR:
      for (i = 0; i < DIR_ENTRIES_PER_BLOCK; i ++) {
        FILE_SYSTEM[blocknum].data.entries[i].inode =  EMPTY_DIR_ENTRY;
      }
      break;
    case REG:
      FILE_SYSTEM[blocknum].data.file[0] = '\0';
      break;
  }
}

int find_open_inode() {
  int pos = 0;
  int i = INODE_ARRAY_START;
  inode_t* inode;
  while (1) {
    inode = &FILE_SYSTEM[i].data.iNodeArray[pos];
    if (inode -> status == FREE) break;
    pos++;
    if ( pos == 4 ) {
      pos = 0;
      i++;
    }
  }

  return 4 * i - 4 + pos;
}

void create_dir_entry(dir_t* dirEntry, int inode, char* filename, int parent_dir) {
  dirEntry -> inode = inode;
  int i;
  for ( i = 0; i < FILENAME_LENGTH; i++ ) {
    if (*(filename+i) == '\0' || *(filename+i) == 0) {
      dirEntry -> filename[i] = '\0';
      break;
    }
    dirEntry -> filename[i] = *(filename+i);
  }

  FILE_SYSTEM[(parent_dir / 4) + 1].data.iNodeArray[parent_dir % 4].size += 16;
}

void create_empty_inode(int pos, uint8_t type) {
  inode_t* inode = &FILE_SYSTEM[(pos / 4) + 1].data.iNodeArray[pos % 4];
  inode -> status = IN_USE;
  inode -> type = type;
  inode -> size = 0;
  inode -> nOpened = 0;
  inode -> limit = 0;
  int i;
  for ( i = 0; i < 10; i++ ) {
    inode -> location[i] = UNUSEDBLOCK;
  }
  FILE_SYSTEM[SUPERBLOCK_POS].data.meta.lInodes--;
}
