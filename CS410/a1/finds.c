#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

char *pathName = NULL;
int fFlag = 0;
char *fExt = NULL;
int lFlag = 0;
char *string = NULL;

#define FTW_F    1      // file other than directory 
#define FTW_D    2      // directory 
#define FTW_DNR  3      // directory that can't be read 
#define FTW_NS   4      // file that we can't stat 

#ifdef PATH_MAX
static long pathMax = PATH_MAX;
#else
static long pathMax = 0;
#endif

static long posix_version = 0;
static long xsi_version = 0;

#define PATH_MAX_GUESS 1024

static char *fullPath;
static size_t pathSize;

static int myFTW();
int checkExtension(char *);

typedef int FileHandler(char *, const struct stat *, int);
static FileHandler fileHandler;

struct node {
  char* path;
  struct node * next;
}typedef node;

struct node * visited;

void insert(char* path) {
  node* temp;
  char* copy = malloc(strlen(path));
  strcpy(copy, path);
  temp = (node*) malloc(sizeof(node));
  temp->path = copy;
  if (visited == NULL) {
    visited = temp;
    visited->next = NULL;
  } else {
    temp->next = visited;
    visited = temp;
  }
}

char * path_alloc(size_t *pathLen) { 
  char   *ptr;
  size_t size;

  if (posix_version == 0) {
    posix_version = sysconf(_SC_VERSION);
  }

  if (xsi_version == 0) {
    xsi_version = sysconf(_SC_XOPEN_VERSION);
  }

  if (pathMax == 0) { 
    errno = 0;
    if ((pathMax = pathconf("/", _PC_PATH_MAX)) < 0) {
      if (errno == 0) {
        pathMax = PATH_MAX_GUESS;
      } else {
        exit(1);
      }
    } else {
      pathMax++;
    }
  }

  if ((posix_version < 200112L) && (xsi_version < 4)) {
    size = pathMax + 1;
  } else {
    size = pathMax;
  }

  if ((ptr = malloc(size)) == NULL) {
    exit(1);
  }

  if (pathLen != NULL) {
    *pathLen = size;
  }
  return (ptr);
}

int checkExtension(char* path) {
  int n = strlen(path);
  if (path[n-2] == '.') {
    if (path[n-1] == *fExt) {
      return 1;
    }
  }
  return 0;
}


static int doPath(char* path) {
  struct stat     statbuf;
  struct dirent   *dirp;
  DIR             *dp;
  int             ret, n;

  if (lstat(path, &statbuf) < 0) /* stat error */
    return(fileHandler(path, &statbuf, FTW_NS));
  if (S_ISDIR(statbuf.st_mode) == 0) /* not a directory */
    return(fileHandler(path, &statbuf, FTW_F));

  if ((ret = fileHandler(path, &statbuf, FTW_D)) != 0)
    return(ret);

  n = strlen(path);
  if (n + NAME_MAX + 2 > pathSize) {    
    pathSize *= 2;
    if ((path = realloc(path, pathSize)) == NULL) {
      exit(1);
    }
  }
  path[n++] = '/';
  path[n] = 0;

  if ((dp = opendir(path)) == NULL) {
    return(fileHandler(path, &statbuf, FTW_DNR));
  }

  while ((dirp = readdir(dp)) != NULL) {
    if (strcmp(dirp->d_name, ".") == 0  || strcmp(dirp->d_name, "..") == 0) {
      continue;                 
    }
    strcpy(&path[n], dirp->d_name);   
    if ((ret = doPath(path)) != 0) { 
      break;    
    }
  }
  path[n-1] = 0;
  if (closedir(dp) < 0) {
    exit(1);
  }
  return (ret);
}

static int myFTW(char* path) {
  path = path_alloc(&pathSize);    
  if (pathSize < strlen(pathName)) {
    pathSize = strlen(pathName) * 2;
    if ((path = realloc(path, pathSize)) == NULL ) {
      exit(1);
    }
  }
  strcpy(path, pathName);
  return (doPath(path));
}

int parseCmdLineArgs(int argc, const char** argv) {
  int index;
  int c;
  opterr = 0;
  while ((c = getopt (argc, argv, "s:f:lp:")) != -1) {
    switch (c)
    {
      case 'p':
        pathName = optarg;
        break;

      case 'f':
        fFlag = 1;
        if (*optarg != 'c' && *optarg != 'h' && *optarg != 'S') {
          printf("Invalid file extension.\n");
          return 1;
        }
        fExt = optarg;
        break;

      case 'l':
        lFlag = 1;
        break;

      case 's':
        string = optarg;
        break;

      case '?':
        if (optopt == 'p')
          printf("Option -%c requires an argument.\n", optopt);
        else if (optopt == 'f')
          printf("Option -%c requires an argument.\n", optopt);
        else if (optopt == 's')
          printf("Option -%c requires an argument.\n", optopt);
        else if (isprint(optopt))
          fprintf("Unknown flag '-%c'.\n", optopt);
        else
          printf("Unknown flag char '\\x#%x'.\n", optopt);
        return 1;

      default:
        abort();
    }
  }

  if (pathName == NULL) {
    printf("-p requires an argument.");
    return 1;
  }
  if (string == NULL) {
    printf("-s requires an argument.");
    return 1;
  }
  printf("command line argument results:\n");
  printf("pathName = %s\nfFlag = %d\nfExt = %s\nlFlag = %d\nstring = %s\n\n", pathName, fFlag, fExt, lFlag, string);

  for (index = optind; index < argc; index++)
    printf("Non-option argument %s\n", argv[index]);

  return 0;
}

int regex(char* string, char* path) {
  FILE *file;
  char *line = NULL;
  size_t length = 0;

  if ( (file = fopen(path, "r")) == NULL) {
    printf("Error opening file %s.\n", path);
  } else {
    while ((getline(&line, &length, file)) != -1) {
      if (regexLineParser(string, line, length) == 1) {
        printf("Found line in %s Line is:%s", path, line);
      }
      length = 0;
    }
  }
}

int regexLineParser(char* string, char* line, size_t length) {
  int i;
  int r = 0;
  int regexSize = strlen(string);
  int questionFlag = 0;
  int starFlag = 0;
  for(i =0; i < length; i++) {
    if ( r >= regexSize) {
      return 1;
    }
    if ( !(isalpha(string[r]) || isdigit(string[r])) ) {
      printf("%d is not a valid regex character.\n", string[r]);
    }
    if (r + 1 < regexSize &&  string[r+1] == '*') {
      if (string[r] == '?') {
        printf("Invalid regex string entered.");
        exit(1);
      } else {
        // skip the current character and the special character.
        // set star flag.
        r = r + 2;
        starFlag = 1;
      }
    } else if (r + 1 < regexSize &&  string[r+1] == '?') {
      if (string[r] == '*') {
        printf("Invalid regex string entered.");
        exit(1);
      } else {
        // skip the current character and the special character.
        // set question flag.
        r = r + 2;
        questionFlag = 1;
      }
    } else if (string[r] == '.') { //any character can be skipped
      r++;
      starFlag = 0;
      questionFlag = 0;
    } else if (line[i] == string[r]) { //normal characters covered
      r++;
      // reset flags, a new character was found that matches the regex.
      starFlag = 0;
      questionFlag = 0;
    } else if (questionFlag == 1 && line[i] == string[r-2]) {
      questionFlag = 0;
    } else if (starFlag == 1 && line[i] == string[r-2]) {
      // don't change flag, keep it in case of repeats.
    } else {
      r = 0;
    }
  }
  return 0;
}

int alreadyVisited(char* symLinkPath) {
  if (visited == NULL) {
    return 0;
  }
  struct node * cur = visited;
  do {
    if (strcmp(cur->path, symLinkPath) == 0) {
      return 1;
    }
    cur = cur->next;
  } while (cur != NULL);
  return 0;
}

int fileHandler(char* path, const struct stat *statptr, int type){
  switch(type) {
    case FTW_NS:
      printf("Error on path:%s\n", path);
      break;
    case FTW_D:
      insert(path);
      break;
    case FTW_DNR:
      printf("Can't read directory:%s\n", path);
      break;
    case FTW_F:
      if (lFlag == 1) {
        if (S_ISREG(statptr->st_mode) != 0) {
          if (fFlag == 1) {
            if (checkExtension(path) == 1) {
              regex(string, path);
            }
          } else {
            regex(string, path);
          }
        } else if (S_ISLNK(statptr->st_mode) != 0) {
          char* symLinkPath = malloc(pathMax);
          if( (readlink(path, symLinkPath, pathMax)) != -1){
            char* realPath = canonicalize_file_name(symLinkPath);
            if (alreadyVisited(realPath) == 0) {
              doPath(realPath);
            } else {
              // do nothing already visited
            }
          } else {
            printf("Error reading sym link.\n");
          }
        }
      } else {
        if (S_ISREG(statptr->st_mode) != 0) {
          if (fFlag == 1) {
            if (checkExtension(path) == 1) {
              regex(string, path);
            }
          } else {
            regex(string, path);
          }
        } else {
          printf("%s is not a regular file.\n", path);
        }
      }
      break;
  }

  return 0;
}

int main(int argc, const char** argv) {
  int ret;
  visited = malloc( sizeof(struct node) );
  if (parseCmdLineArgs(argc, argv) == 1)
    return 1;
  ret = myFTW(pathName);
  if (ret == 1) {
    return 1;
  } else {
    return 0;
  }
}
