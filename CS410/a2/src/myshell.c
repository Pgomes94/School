//==============================================================================
//==============================================================================
// Michael Gerakis
// Patrick Gomes
// Zhenjie Jason Ruan
//
// Assignment 2
// CS410
// A basic shell implementation.
// Sources:
// http://stephen-brennan.com/2015/01/16/write-a-shell-in-c/
//==============================================================================
//==============================================================================


#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid()
#include <unistd.h>     // fork, exec, pid_t
#include <stdlib.h>     // malloc, free, exit, execvp, EXIT_SUCCESS
#include <stdio.h>      // getchar, perror, printf,, fileno, FILE
#include <string.h>     // strcmp, strtok
#include <termios.h>    // struct termios
#include <signal.h>     // signal handling


#define MYSHELL_RL_BUFSIZE 1024
#define MYSHELL_TOK_BUFSIZE 64
#define MYSHELL_TOK_DELIM " \t\r\n\a"
#define MYSHELL_CMD_SIZE 3


//-------------------------------------------------
// Function prototypes ----------------------------
//-------------------------------------------------
void
sig_handler(int sig, siginfo_t* siginfo, void* context);

void
myshell_loop(void);

char *
myshell_read_line(void);

char **
myshell_split_line(char* string, char* delim);

int
myshell_execute(char **args);

//-------------------------------------------------

//-------------------------------------------------
// Global Variables -------------------------------
pid_t* runningProcesses = NULL; // only stores processes running in the foreground.
int countProccesses = 0; // only counts processes in the foreground.
int backgroundProcessEnded = 0; // flag for whether or not a backgroundProcessEnded recently
//-------------------------------------------------

int
main(int argc, char** argv) {

  struct sigaction sigIntHandler; // struct for catching and handling SIGINT signal
  struct sigaction sigChldHandler; // struct for catching and handling SIGCHLD signal

  sigIntHandler.sa_sigaction = &sig_handler; // use sighandler to handle signals
  sigIntHandler.sa_flags = SA_RESTART; // restart instead of ending

  sigChldHandler.sa_sigaction = &sig_handler; // use sighandler to handle signals
  sigChldHandler.sa_flags = SA_SIGINFO; // use sa_sigaction instead of sa_handler

  sigaction(SIGINT, &sigIntHandler, NULL); // catches SIGINT
  sigaction(SIGCHLD, &sigChldHandler, NULL); // catches SIGCHLD

  myshell_loop();

  free(runningProcesses); // myshell is done, free space from malloc

  return EXIT_SUCCESS;
}

void sig_handler(int sig, siginfo_t* siginfo, void* context) {
  if (sig == SIGINT) {
    // kill all foreground processes running.
    int i;
    for (i = 0; i < countProccesses; i++) {
      kill(runningProcesses[i], SIGKILL);
    }
    printf("\n");
  } else if (sig == SIGCHLD) {
    // notifies parent a child ended.
    // number of processes running decreases.
    int i;
    int found = 0;
    for (i = 0; i < countProccesses; i++) {
      // check that the pid is a foreground process, adjust count numbers then.
      if (runningProcesses[i] == siginfo->si_pid) {
        countProccesses--;
        found = 1;
      }
    }
    if (found == 0) {
      // no foreground process found, must be background. set flag to catch EOF coming up.
      backgroundProcessEnded = 1;
      ("Background process with pid: %d has finished!\n", siginfo->si_pid);
    }
  }
}

void
myshell_loop(void) {
  char **cmds;
  char **args;
  int status;
  char *line;

  status = 1;

  do {
    // check if input is from terminal or file.
    if (isatty(fileno(stdin)))
      printf("myshell> ");
    line = myshell_read_line(); // reads line character by character
    cmds = myshell_split_line(line, ";"); // parse into commands
    int i;
    if (cmds[0] != '\0') {
      for (i = 0; cmds[i] != NULL; i++) {
        if (status) { // make sure status is good after each command runs
          args = myshell_split_line(cmds[i], MYSHELL_TOK_DELIM); // parse each command
          status = myshell_execute(args); // execute one command at a time
        } else {
          break;
        }
      }
      free(line);
      free(args);
      free(cmds);
    }
  } while(status); // if after all commands run the status is still good, read next line
}

char *
myshell_read_line(void) {
  int bufsize = MYSHELL_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "myshell: allocation error\n");
    free(runningProcesses); // myshell is done, free space from malloc
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // background processes send and EOF when they are finished. catch this and don't end if EOF comes from a background process.
    if (c == EOF && backgroundProcessEnded == 0) {
      printf("\n");
      free(runningProcesses); // myshell is done, free space from malloc
      exit(1);
    } else if (c == EOF && backgroundProcessEnded == 1) {
      backgroundProcessEnded = 0;
      return NULL;
    }

    // at new line set null terminator and return string
    if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // buffer full, expand buffer
    if (position >= bufsize) {
      bufsize += MYSHELL_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "myshell: allocation error\n");
        free(runningProcesses); // myshell is done, free space from malloc
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **
myshell_split_line(char* string, char* delim) {
  int bufsize = MYSHELL_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "myshell: allocation error\n");
    free(runningProcesses); // myshell is done, free space from malloc
    exit(EXIT_FAILURE);
  }

  token = strtok(string, delim); // breaks string into tokens, removes occurences of delim from string
  while (token != NULL) {
    tokens[position] = token;
    position++;

    // tokens full, expand tokens
    if (position >= bufsize) {
      bufsize += MYSHELL_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "myshell: allocation error\n");
        free(runningProcesses); // myshell is done, free space from malloc
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, delim); // get next token
  }
  tokens[position] = NULL;
  return tokens;
}

int
myshell_execute(char** args) {
  if (args[0] == NULL) {
    return 1;
  }

  // Flags
  int    stdoutRedirectFlag   = 0;
  int    stdinRedirectFlag    = 0;
  int    stderrRedirectFlag   = 0;
  int    backgroundExecFlag   = 0;
  int    pipeFlag             = 0;
  // Redirection file
  FILE*  outFile;
  FILE*  inFile;
  FILE*  errFile;
  // Iteration variables
  int    i;
  // New argument list (for redirection uses)
  char *newArgs;
  int argsMemLength = 0;

  // Find the char size of the args array
  // Make sure to add size for spaces
  for (i = 0; args[i] != NULL; i++) {
    argsMemLength += strlen(args[i]);   // count the args[j]
    argsMemLength += 1;                 // space
  }

  // Convert args ARRAY into a newArgs STRING
  newArgs = malloc(sizeof(char) * argsMemLength);
  newArgs[0] = '\0';
  for (i = 0; args[i] != NULL; i++ ) {
    strcat(newArgs, args[i]);
    strcat(newArgs, " \0");
  }

  int end = strlen(newArgs) - 1;
  int argsPos = 0;
  for (i = 0; i < end; i++) {
    if (newArgs[i] == ' ')
      argsPos++;
    if (newArgs[i] == '>' || (newArgs[i] == '1' && i < end - 1 && newArgs[i] == '>')) {
      if (stdoutRedirectFlag)
        fclose(outFile);
      stdoutRedirectFlag = 1;
      outFile = fopen(args[argsPos+1], "w");
      newArgs[i] = '\0';
    }
    if (newArgs[i] == '<') {
      if (stdinRedirectFlag)
        fclose(inFile);
      stdinRedirectFlag = 1;
      inFile = fopen(args[argsPos+1], "r");
      newArgs[i] = '\0';
    }
    if (i < end - 1 && (newArgs[i] == '2' && newArgs[i+1] == '>')) {
      if (stderrRedirectFlag)
        fclose(errFile);
      stderrRedirectFlag = 1;
      errFile = fopen(args[argsPos+1], "w");
      newArgs[i] = '\0';
    }
    if (newArgs[i] == '&') {
      backgroundExecFlag = 1;
      newArgs[i] = '\0';
    }
    if (i < end - 1 && (newArgs[i] == '&' && newArgs[i+1] == '>')) {
      if (stdoutRedirectFlag)
        fclose(outFile);
      if (stderrRedirectFlag)
        fclose(errFile);
      stdoutRedirectFlag = 1;
      stderrRedirectFlag = 1;
      outFile = fopen(args[argsPos+1], "w");
      errFile = fopen(args[argsPos+1], "w");
      newArgs[i] = '\0';
    }
    if (newArgs[i] == '|') {
      if (stdoutRedirectFlag)
        fclose(outFile);
      if (stdinRedirectFlag)
        fclose(inFile);
      if (stderrRedirectFlag)
        fclose(errFile);
      pipeFlag = 1;
    }
  }

  pid_t pid, wpid;
  int status;
  pid = fork();
  if (pid == 0) {
    // Child process

    if (pipeFlag) {
      newArgs[0] = '\0';
      for (i = 0; args[i] != NULL; i++ ) {
        strcat(newArgs, args[i]);
        strcat(newArgs, " \0");
      }
      FILE* fp;
      int PATH_MAX = 1024;
      char path[PATH_MAX];
      fp = popen(newArgs, "r");
      while(fgets(path, PATH_MAX, fp) != NULL)
        printf("%s", path);
      pclose(fp);
      free(newArgs);
      exit(1);
    }

    for (i = 0; args[i] != NULL; i++) {
      if (strcmp(args[i], ">\0") == 0 || strcmp(args[i], "1>\0") == 0) {
        args[i] = '\0';
      } else if (strcmp(args[i], "<\0") == 0) {
        args[i] = '\0';
      } else if (strcmp(args[i], "2>\0") == 0) {
        args[i] = '\0';
      } else if ((strcmp(args[i], "&\0") == 0)) {
        args[i] = '\0';
      } else if (strcmp(args[i], "&>\0") == 0) {
        args[i] = '\0';
      }
    }

    // First check if flags are set and make appropiate
    // I/O redirect
    if(stdoutRedirectFlag == 1) {
      dup2(fileno(outFile), STDOUT_FILENO);
      fclose(outFile);
    }
    if (stdinRedirectFlag == 1) {
      dup2(fileno(inFile), STDIN_FILENO);
      fclose(inFile);
    }
    if (stderrRedirectFlag == 1) {
      dup2(fileno(errFile), STDERR_FILENO);
      fclose(errFile);
    }

    // execute the command, exit with 0 if error, with 1 if successful
    if (execvp(args[0], args) == -1) {
      perror("myshell");
      exit(0);
    } else {
      exit(1);
    }
  } else if (pid < 0) {
    perror("myshell");
  } else {
    // Parent process
    if (backgroundExecFlag == 0) { // waits only if backgroundExecFlag is not set
      // store pid of the running processes in the foreground.
      countProccesses++;
      runningProcesses = (pid_t*) realloc (runningProcesses, countProccesses * sizeof(pid_t)); // reallocate space for number of running processes
      runningProcesses[countProccesses - 1] = pid;
      do {
        wpid = waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
  }
  return 1;
}
