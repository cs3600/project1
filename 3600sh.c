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
// by running a command or printing an error statement
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
    size_t argSize = strlen(line);
    char* argv[argSize]; // TODO more efficient allocation of argv size
    getTokens(line, argv);

    // exit or logout, do clean exit
    if (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "logout") == 0) {
      do_exit();
    }
    // execute the program if the first arg is not null
    else if (argv[0]) {
      execute(argv);
    }
  }

  // check for end of input
  if (feof(stdin)) {
    do_exit();
  }

  // free line buf after malloc  
  free(line);
}

// Function that tokenizes a line of input into an array.
// @param line
//     The line to tokenize
// @param argv
//     Pointer to an array of strings; the result is stored in
//     this array
void getTokens(char* line, char* argv[]) {
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
        // escape character
        if (*line == '\\') {
          memmove(&line[0], &line[1], strlen(line));
          // handle '\t'
          if (*line == 't') {
            *line = '\t';
          }
          // check if valid escape
          else if (*line != '\\' && *line != ' ' && *line != '&') {
            printf("Error: Unrecognized escape sequence.\n");
          }
        }
        line++;
      }
      // terminate the string
      *line++ = '\0';
      // add to array of commands
      *argv++ = word;
    }
    else {
      break;
    }
  }
  // null terminate the array to use exec
  *argv = '\0';
}

// Function that executes the commands in argv.
// at the end of program execution.
// @param argv
//     The commands to execute
void execute(char* argv[]) {
  // fork a child process
  pid_t pid = fork();
  // could not fork
  if (pid == -1) {
    printf("Error: Could not fork child process.\n");
  }
  // child process, execute and make daddy proud
  else if (pid == 0) {
    // exit status
    int status;
    // execute the command, check for error
    if ((status = execvp(argv[0], argv)) < 0) {
      // check for permission errors
      if (errno == EACCES) {
        printf("%s", "Error: Permission Denied.\n");
      }
      else {
        printf("%s", "Error: Command not found.\n");
      }
      exit(-1);
    }
  }
  // parent process, wait for your child
  else {
    waitpid(pid, NULL, 0);
  }
}

// Function which exits, printing the necessary message
void do_exit() {
  printf("So long and thanks for all the fish!\n");
  // Wait for all children to exit, then exit
  while (wait(NULL) > 0) {}
  exit(0);
}
