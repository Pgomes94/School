//==============================================================================
//==============================================================================
// Michael Gerakis
// Patrick Gomes
// Zhenjie Jason Ruan
//
// Assignment 2
// CS410
//==============================================================================
//==============================================================================
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define RL_BUFSIZE 1024
#define TOK_DELIM ",\n"

//-------------------------------------------------
// Function prototypes ----------------------------
//-------------------------------------------------
int
getPathsCount(char*);

char*
read_input(void);

void
orderPaths(int);
//-------------------------------------------------

//-------------------------------------------------
// Global Variables -------------------------------
char** paths;
char** weights;
int* pathsOrdered;
//-------------------------------------------------

int main() {

  char* token;
  int num_paths = 0;
  int first = 1;
  char* input;

  input = read_input();

  int paths_size = getPathsCount(input);

  paths   = malloc(sizeof(char *) * paths_size);
  weights = malloc(sizeof(char *) * paths_size);

  token = strtok(input, TOK_DELIM);
  while (token != NULL) {
    if(first) {             // getting first token from line, always weight
      weights[num_paths] = token;
      first = 0;
    } else {
      paths[num_paths] = token;
      first = 1;
      num_paths++;
    }

    token = strtok(NULL, TOK_DELIM);
  }

  orderPaths(num_paths);

  int i;
  for (i = 0; paths[i] != NULL; i++) {
    printf("%s, %s\n", weights[pathsOrdered[i]], paths[pathsOrdered[i]]);
  }
}

int getPathsCount(char* input) {
  int i;
  int pathsCount = 0;
  for (i = 0; input[i] != '\0'; i++) {
    if (input[i] == '\n')
      pathsCount++;
  }
  return pathsCount;
}

char* read_input() {
  int bufsize = RL_BUFSIZE;
  int position = 0;
  char* buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "tspsort: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while(1) {

    //Read a character
    c = getchar();

    // at new line set null terminator and return string
    if (c == EOF) {
      return buffer;
    }

    buffer[position] = c;

    position++;

    // if buffer full, expand
    if (position >= bufsize) {
      bufsize += RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "tspsort: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

void orderPaths(int num_paths) {
  pathsOrdered = malloc(sizeof(int) * num_paths);
  int i;
  for (i = 0; i < num_paths; i++) {
    pathsOrdered[i] = i;
  }

  int j;
  int temp;
  for (i = 0; i < num_paths; i++) {
    j = i;
    while (j > 0 && atoi(weights[pathsOrdered[j-1]]) > atoi(weights[pathsOrdered[j]])) {
      temp = pathsOrdered[j];
      pathsOrdered[j] = pathsOrdered[j-1];
      pathsOrdered[j-1] = temp;

      j = j - 1;
    }
  }
}
