#ifndef _PRINT_H
#define _PRINT_H

// Initialize the terminal for writing
void terminal_initialize();
// Int to String (char *)
void itoa(char *buf, int base, int num);
// Write to terminal, adding a new line at the end if nl flag is set
void write(const char* data, bool nl);

void terminal_newline();

#endif
