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

#include <stdlib.h>     // malloc, free, exit, execvp, EXIT_SUCCESS
#include <stdio.h>      // getchar, perror, printf,, fileno, FILE
#include <string.h>     // strtok
#include <errno.h>      // errno
#include <sys/types.h>  // pid_t
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define MAX_DISTANCE 255
#define MAX_NODES 10
#define RL_BUFSIZE 1024
#define TOK_DELIM " "
#define get_edge(i, j) graph[num_nodes * i + j] // gets the edge between i and j
#define SHSIZE (num_nodes * 2) * (num_nodes * 2) + (num_nodes * 5)

//-------------------------------------------------
// Function prototypes ----------------------------
//-------------------------------------------------
char*
read_line(void);

int
find_paths(int node, int shmid);

void
permute(char* node, char* remaining, char* shm, int start, int end);

void
swap(char* x, char* y);

int
isValidPath(char* path);

//-------------------------------------------------

// Global Variables -------------------------------

char **graph;
int num_nodes = -1;

//-------------------------------------------------

int main() {
  int rows = 0; // how many rows, should be equal to num_nodes after all inputs is r
  int totalEntries = 0; // the total number of entries to the distance graph. should equal num_nodes * num_nodes
  char *line;
  char *distance;
  pid_t pid, wpid;
  int status;
  pid_t currpid;

  do {
    line = read_line();
    distance = strtok(line, TOK_DELIM);
    while (distance != NULL) {
      int c = atoi(distance);
      if (c > MAX_DISTANCE) { // make sure distance isn't greater than max distance.
        errno = EINVAL;
        perror("tsp_p: Entered distance greater than the MAX_DISTANCE (255).\n");
        exit(EXIT_FAILURE);
      }
      if ( (totalEntries%num_nodes) == rows) {
        // distance from i to i should be 0.
        if (c != 0) {
          errno = EINVAL;
          perror("tsp_p: Entered non-zero distance to itself.\n");
          exit(EXIT_FAILURE);
        }
      }
      graph[totalEntries] = distance;
      totalEntries++;
      distance = strtok(NULL, TOK_DELIM);
    }
    rows++;
  } while ( rows < num_nodes);
  graph[totalEntries] = NULL;

  if (totalEntries != (num_nodes*num_nodes)) {
    errno = EINVAL;
    perror("tsp_p: Incorrect adjacency matrix entered!\n");
    exit(EXIT_FAILURE);
  }
  int i;
  key_t key;
  int shmid;

  key = 9876;
  for (i = 0; i < num_nodes; i++) {
    // for each node, fork and run find_paths on the child. parent waits for all children to finish.
    shmid = shmget(key, SHSIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
      perror("shmget");
      exit(1);
    }

    pid = fork();
    if (pid == 0) {
      // child process
      find_paths(i, shmid);
      exit(EXIT_SUCCESS);
    } else if (pid < 0) {
      perror("tsp_p: fork error");
    } else {
      // parent process, wait for child to finish
      do {
        wpid = waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));

      char *shm;
      shm = shmat(shmid, NULL, 0);

      if (shm == (char *) -1) {
        perror("shmat");
        exit(1);
      }

      char *s;
      for (s = shm; *s != 0; s++)
        write(1, s, 1);
      shmdt(shm);
      shmctl(shmid, IPC_RMID, NULL);
      key++;
    }
  }


  return 0;
}

char *
read_line(void) {
  int first_time = 0; // flag for checking if num_nodes has been set
  int count_nodes = 0; // how many distance pairs are on this line
  int bufsize = RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (num_nodes == -1) {
    first_time = 1;
  }

  if (!buffer) {
    fprintf(stderr, "tsp_p: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // at new line set null terminator and return string
    if (c == EOF | c == '\n') {
      count_nodes++;
      if (first_time == 1) {
        // first time, record how many nodes were found.
        if (count_nodes > MAX_NODES) {
          perror("tsp_p: Too many nodes entered. Max Nodes is 10, change to add more.\n");
        }
        num_nodes = count_nodes;
        graph = malloc(sizeof(char *) * num_nodes * num_nodes); // num_nodes * num_nodes total char * entries
        first_time = 0;
        buffer[position] = '\0';
        return buffer;
      } else if (num_nodes == count_nodes) {
        buffer[position] = '\0';
        return buffer;
      } else {
        errno = EINVAL;
        perror("tsp_p: Inconsistent number of nodes entered.\n");
        exit(EXIT_FAILURE);
      }
    } else {
      if (c == ' ')
      {
        count_nodes++;
      }
      buffer[position] = c;
    }
    position++;

    // buffer full, expand buffer
    if (position >= bufsize) {
      bufsize += RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "tsp_p: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

int find_paths(int node, int shmid) {
  int i, j;
  char *shm;
  char buffer[2];
  char path [num_nodes*2];

  shm = shmat(shmid, NULL, 0);
  if (shm == (char *) -1) {
    perror("shmat");
    exit(1);
  }
  char* remaining = malloc(num_nodes);

  for (i = 0; i < num_nodes; i++) {
    if (i != node) {
      sprintf(buffer, "%d ", i);
      strcat(remaining, buffer);
    }
  }

  sprintf(buffer, "%d", node);

  remaining[num_nodes * 2 - 1] = '\0';

  permute(buffer, remaining, shm, 0, strlen(remaining) -1 );


  shmdt(shm);
  return 0;
}

void permute(char* node, char* remaining, char* shm, int start, int end) {
  int i;
  char* result = malloc(sizeof(remaining) *2 + 4);

  if (start == end | start == end - 1) {
    result[0] = *node;
    result[1] = ' ';

    strcat(result, remaining);
    strcat(result, node);
    char* check = malloc(strlen(result));
    memcpy(check, result,strlen(result));
    int weight = isValidPath(check);
    if (weight != -1) {
      char weightS[20];
      sprintf(weightS, "%d,", weight);
      result[strlen(result) - 1] = '\n';
      strcat(shm, weightS);
      strcat(shm, result);
      free(result);
    }
  } else {
    for (i = start; i <= end; i+=2) {
      swap((remaining + start), (remaining+i));
      permute(node, remaining, shm, start+2, end);
      swap((remaining + start), (remaining+i)); //backtrack
    }
  }
}

void swap(char* x, char* y) {
    char temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

int isValidPath(char* path) {
  int weight;
  weight = 0;
  char* prev = strtok(path, " ");
  char* next = strtok(NULL, " ");
  do {
    if (!strcmp(get_edge(atoi(prev), atoi(next)), "-1\0")) {
      return -1;
    } else {
      weight += atoi(get_edge(atoi(prev), atoi(next)));
    }
    prev = next;
  } while (next = strtok(NULL, " "));
  return weight;
}
