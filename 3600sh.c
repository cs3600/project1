/**
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * You should use this (very simple) starter code as a basis for 
 * building your shell.  Please see the project handout for more
 * details.
 */

#include "3600sh.h"

#define USE(x) (x) = (x)
// Obtained through "getconf HOST_NAME_MAX"
// Is there a cleaner way of doing this? HARDCODED TODO
// sysconf(HOST_NAME_MAX)
#define HOST_NAME_MAX 64

int main(int argc, char* argv[]) {
  // Code which sets stdout to be unbuffered
  // This is necessary for testing; do not change these lines
  USE(argc);
  USE(argv);
  setvbuf(stdout, NULL, _IONBF, 0); 
  
  // Main loop that reads a command and executes it
  while (1) {
    // Display prompt
    prompt();
    // Read in line of input
    processLine(); 
  }

  return 0;
}

// Function that prints a prompt to the console.
// The prompt is of the form:
// '[user]@[host]:[cwd]> '
// where cwd is the current working directory.
void prompt() {
  // gets the username
  char* user = "???";
  struct passwd* pw = getpwuid(getuid());
  if (pw) {
    user = pw->pw_name;
  }

  // gets the hostname
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);

  // gets cwd, uses malloc
  char* cwd = getcwd(NULL, 0);

  // print console prompt
  if (pw && cwd) {
    printf("%s@%s:%s> ", user, hostname, cwd);
  }

  // flush output buffer to console
  fflush(stdout);
  // free cwd buf after malloc
  free(cwd);
}

// Function that reads in a line of input and responds
// by running a command or printing an error statement.
void processLine() {
  // read input from console
  char* line = getline();

  // exit on input error
  if (ferror(stdin)) {
    printf("Error: reading input.");
    do_exit();
  }

  // process input (single newline and empty string ignored)
  else if (line[0] != '\n' && strcmp(line, "")) {
    // get array of command tokens
    int argc = strlen(line);
    char* argv[argc];
    // check tokens are well formatted
    if ((argc = getTokens(line, argv)) > 0) {
      // exit or logout, do clean exit
      if (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "logout") == 0) {
        do_exit();
      }
      // execute the program if the first arg is not null
      else if (argv[0]) {
        execute(argc, argv);
      }
    }
  }

  // free line buf after malloc  
  free(line);

  // check for end of input
  if (feof(stdin)) {
    do_exit();
  }
}

// Function that tokenizes a line of input into an array
// of command tokens. This function permits only the 
// following escape sequences as valid '\t', '\&', '\ ',
// and '\\'. Any invalid sequence prints an error.
// This function also only permits background '&' 
// character can be specified only once, at the end
// of the input line. The '&' character can be separated 
// from the final token, but it is not required.
// Valid backgrounding examples:
//
//  ls__&
//  ls_-l&__
//
// Invalid:
//
// ls & /
// ls & &
//
// @param line
//     The line to tokenize
// @param argv
//     Pointer to an array of strings; the result is stored in
//     this array
// @return
//     -1 if invalid escape sequence is encountered. 
//     -2 if background character syntax is invalid.
//     Otherwise, the size of the resulting array is returned.
int getTokens(char* line, char* argv[]) {
  // size of resulting array
  int argc = 0;
  // has the background character '&' been encountered?
  int bg = 0;
  // is background character '&' attached to the previous token?
  int isbgattached = 0;

  // loop until string termination reached
  while (*line != '\0') {
    // eat white space
    while (*line == ' ' || *line == '\t' || *line == '\n') {
      line++;
    }
    // check that the end of the string hasn't been reached
    if (*line != '\0') {
      // pointer to start of the word
      char* word = line;
      // skip non-white space
      while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n') {

        // background check ;)
        if (!bg && *line == '&') {
          bg = 1;
          // drop the attached & from the token
          if (*word != '&') {
            isbgattached = 1;
            memmove(&line[0], &line[1], strlen(line));
            continue;
          }
        }
        // already backgrounded before, invalid syntax
        else if (bg) {
          printf("Error: Invalid syntax.\n");
          return -2;
        }
        // escape character check
        else if (*line == '\\') {
          memmove(&line[0], &line[1], strlen(line));
          // handle '\t'
          if (*line == 't') {
            *line = '\t';
          }
          // check if valid escape
          else if (*line != '\\' && *line != ' ' && *line != '&') {
            printf("Error: Unrecognized escape sequence.\n");
            return -1;
          }
        }
        line++;
      }
      // terminate new string token, if not the end of original input string
      if (*line != '\0') {
        *line++ = '\0';
      }
      // add to array of commands, keep track of size
      *argv++ = word;
       argc++;
      // add new '&' token if it was attached to the
      // previous token.
      if (isbgattached) {
        *argv++ = "&";
         argc++;
      }
    }
    else {
      break;
    }
  }
  // null terminate the array to use exec
  *argv = '\0';
  return argc;
}

// Function that executes the commands in argv.
// @param argv
//     The commands to execute. This array
//     must be null terminated.
void execute(int argc, char* argv[]) {
  // background command
  int bg = strcmp(argv[argc - 1], "&") == 0;
  // fork a child process
  pid_t pid = fork();
  // could not fork
  if (pid == -1) {
    printf("Error: Could not fork child process.\n");
  }
  // child process, execute and make daddy proud
  else if (pid == 0) {

    // ignore '&' token
    if(bg) {
      // background the process
      argv[argc - 1] = NULL;
    }

    // loop and check for redirects
    int i, infd = -1, outfd = -1, errfd = -1;
    for (i = 0; i < argc; i++) {
      // check element is not null
      if (argv[i]) {
        int isredirect = is_redirect(argv[i]);
        // check for valid redirection
        if (isredirect && argv[i + 1]) {
          
          // check valid redirect arg, can't be & or a redirect
          if (is_redirect(argv[i + 1]) || strcmp(argv[i + 1], "&") == 0) {
            printf("Error: Invalid syntax.\n");
            exit(-1);
          }

          // arguments after the first redirection file must be either
          // another redirection or the & character
          if (argv[i + 2] && !(is_redirect(argv[i + 2]) || strcmp(argv[i + 2], "&") == 0)) {
            printf("Error: Invalid syntax.\n");
            exit(-1);
          }

          // redirect stdin
          if (strcmp(argv[i], "<") == 0) {
            // close stdin fd
            close(0);
            // open new file descriptor for reading only
            if ((infd = open(argv[i + 1], O_RDONLY)) == -1) {
              printf("Error: Unable to open redirection file.\n");
              exit(-1);
            }
            // null out args
            argv[i] = NULL;
            argv[i + 1] = NULL;
          }

          // redirect stdout
          else if (strcmp(argv[i], ">") == 0) {
            // close stout fd
            close(1);
            // open new file descriptor for writing, create if not exists
            // if a new file is created, user will have read/write permissions
            if ((outfd = open(argv[i + 1], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
              printf("Error: Unable to open redirection file.\n");
              exit(-1);
            }
            // null out args
            argv[i] = NULL;
            argv[i + 1] = NULL;
          }

          // redirect stderr
          else {
            // close stout fd
            close(2);
            // open new file descriptor for writing, create if not exists
            // if a new file is created, user will have read/write permissions
            if ((errfd = open(argv[i + 1], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
              printf("Error: Unable to open redirection file.\n");
              exit(-1);
            }
            // null out args
            argv[i] = NULL;
            argv[i + 1] = NULL;
          }

        }
        // no arg passed to redirection
        else if (isredirect) {
          printf("Error: Invalid syntax.\n");
          exit(-1);
        }
      }
    }

    // exit status
    int status;
    // execute the command, check for error
    if ((status = execvp(argv[0], argv)) < 0) {
      // check for permission errors
      if (errno == EACCES) {
        printf("%s", "Error: Permission denied.\n");
      }
      else {
        printf("%s", "Error: Command not found.\n");
      }
      exit(-1);
    }

    // close new file descriptors
    if (infd != -1) {
      close(infd);
    }
    if (outfd != -1) {
      close(outfd);
    }
    if (errfd != -1) {
      close(errfd);
    }
  }

  // parent process
  else {
    // wait if not backgrounded
    if (!bg) {
      waitpid(pid, NULL, 0);
    }
  }
}

// Checks if the given string is a redirection character.
// @param s
//     the string to check
// @return
// 1 if true, 0 if false.
//
int is_redirect(char* s) {
  return strcmp("<", s) == 0 ||
      strcmp(">", s) == 0 ||
      strcmp("2>", s) == 0;
}

// Function which exits, printing the necessary message
void do_exit() {
  printf("So long and thanks for all the fish!\n");
  // Wait for all children to exit, then exit
  while (wait(NULL) > 0) {}
  exit(0);
}
