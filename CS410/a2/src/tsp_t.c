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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // strtok
#include <ucontext.h>
#include <errno.h>      // errno

#define MAX_THREADS 10
#define RL_BUFSIZE 1024
#define TOK_DELIM " "
#define get_edge(i, j) graph[num_nodes * i + j] // gets the edge between i and j
#define MAX_DISTANCE 255
#define SHSIZE (num_nodes * 2) * (num_nodes * 2) + (num_nodes * 5)

// Global Variables -------------------------------

char **graph;
int num_nodes = -1;
char* shm;

//-------------------------------------------------

/*my thread struct contains info about
my thread*/
typedef struct
{
	ucontext_t context; /*Stores the context of current thread*/
	int active; /*1 if current thread is active, 0 otherwise*/
	void* stack;
} my_thread;

/*queue for the thread*/
my_thread threadList[ MAX_THREADS ];
/*index of current executing thread*/
int currentThreadIndex = -1;
/*Number of currently active threads*/
int numThreads = 0;
/*flag shows if we are in thread or main process*/
int inThread = 0;

/*main process context*/
ucontext_t mainContext;

/*initialize, deactivate all threads*/
void initThreads()
{
	int i;
	for (i = 0; i < MAX_THREADS; ++i)
	{
		threadList[i].active = 0;
	}

	return;
}

/*create thread*/
void my_thr_create(void(*func)(int), int thr_id)
{
	if(thr_id >= MAX_THREADS)
		fprintf(stderr, "%s\n", "Reached thread limit");

	getcontext(&threadList[thr_id].context);

	my_thread *currentThread = &threadList[thr_id];
	/*allocate new stack*/
	currentThread->context.uc_link = NULL;
	currentThread->stack = malloc(SIGSTKSZ);
	if ( currentThread->stack == 0 ){
		fprintf(stderr, "%s\n", "can not allocate stack for new thread");
		exit(1);
	}
	currentThread->context.uc_stack.ss_sp = currentThread->stack;
	currentThread->context.uc_stack.ss_size = SIGSTKSZ;
	currentThread->context.uc_stack.ss_flags = 0;

	makecontext(&currentThread->context, func, 1, thr_id);
	++ numThreads;

}

/*switch between main and child thread*/
void my_thr_switch()
{
	if (inThread){
		//fprintf(stdout, "Switching from Thread %d to Main\n", currentThreadIndex);
		swapcontext(&threadList[currentThreadIndex].context, &mainContext);
	}
	/*we are in main, start a new thread*/
	else{
		if (numThreads == 0) return;

		currentThreadIndex = (currentThreadIndex + 1) % numThreads;

		//printf("starting thread %d\n", currentThreadIndex);
		inThread = 1;
		swapcontext(&mainContext, &threadList[currentThreadIndex]);
		inThread = 0;
		//printf("Thread %d switched back to main\n", currentThreadIndex);

		/*clean up the finish thread*/
		if (threadList[currentThreadIndex].active == 0)
		{
			//printf("Thread %d is finished, cleaning up\n", currentThreadIndex);
			free(threadList[currentThreadIndex].stack);

			--numThreads;
			/*swap last entry with current cleaned entry to prevent
			* hole in the thread queue
			*/
			if (currentThreadIndex != numThreads)
			{
				threadList[currentThreadIndex] = threadList[numThreads];
			}
			/*deavtivated swaped thread*/
			threadList[numThreads].active = 0;
		}
	}
	return;
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

void swap(char* x, char* y) {
    char temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

void permute(char* node, char* remaining, int start, int end) {
  int i;
  char* result = malloc(strlen(remaining) * 2 + 4);
	for (i = 0; result[i] != 0; i++) {
		result[i] = NULL;
	}
  if (start == end | start == end - 1) {
    result[0] = *node;
    result[1] = ' ';

    strcat(result, remaining);
    strcat(result, node);
    char* check = malloc(strlen(result));
    memcpy(check, result, strlen(result));
    int weight = isValidPath(check);
    if (weight != -1) {
      char weightS[20];
      sprintf(weightS, "%d,", weight);
			result[strlen(result)-1] = '\n';
      strcat(shm, weightS);
      strcat(shm, result);
			free(result);
    }
  } else {
    for (i = start; i <= end; i+=2) {
      swap((remaining + start), (remaining+i));
      permute(node, remaining, start+2, end);
      swap((remaining + start), (remaining+i)); //backtrack
    }
  }
}

int find_paths(int node) {
  int i, j;
  char buffer[2];
  char path [num_nodes*2];

  char* remaining = malloc(num_nodes);

  for (i = 0; i < num_nodes; i++) {
    if (i != node) {
      sprintf(buffer, "%d ", i);
      strcat(remaining, buffer);
    }
  }

  sprintf(buffer, "%d", node);

  remaining[num_nodes * 2 - 1] = '\0';

  permute(buffer, remaining, 0, strlen(remaining) -1 );

  return 0;
}

void threadFunction(int thr_id)
{
	threadList[currentThreadIndex].active = 1;
	find_paths(thr_id);
	//printf("now in Thread %d currentThreadIndex: %d\n", thr_id, currentThreadIndex);
	threadList[currentThreadIndex].active = 0;
	my_thr_switch();
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

int main(int argc, char *argv[]) {

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

	shm = malloc(sizeof(char) * SHSIZE);
	int i;

	getcontext(&mainContext);

	for (i = 0; i < num_nodes; ++i)
	{
		my_thr_create(&threadFunction, i);
	}

	while(!(inThread || numThreads == 0)) {
	    my_thr_switch();
	}

	char *s;
	for (s = shm; *s != 0; s++)
		write(1, s, 1);


	return 0;
}
