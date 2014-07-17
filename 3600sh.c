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
    // Read in null terminated line of input
    processLine(); 
    // You should probably remove this; right now, it
    // just exits
    do_exit();
  }

  return 0;
}

// Function that prints a prompt to the console.
// The prompt is of the form:
// '[user]@[host]:[cwd]> '
// where cwd is the current working directory.
void prompt() {
  // hostname buf
  char hostname[HOST_NAME_MAX];
  // cwd buf, uses malloc
  char* cwd = getcwd(NULL, 0);
  // load hostname into buf
  gethostname(hostname, HOST_NAME_MAX);
  // print console prompt
  printf("%s@%s:%s> ", getlogin(), hostname, cwd);
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

  // check error or EOF for stdin
  if (ferror(stdin) || feof(stdin)) {
    printf("%s\n", "Error reading input."); // TODO comply with project guidelines
  }

  // process input
  else {
    // get array of command tokens
    size_t argSize = strlen(line);
    char* argv[argSize]; // TODO more efficient allocation of argv size
    getTokens(line, argv);
    size_t i; // TODO remove
    for (i = 0; i < argSize; i++) {
      printf("Token: %s\n", argv[i]);
    }
    //execute(argv);
  }

  // free line buf after malloc  
  free(line);
}

// Function that tokenizes a line of input into an array.
// @param line
//     The line to tokenize
// @param argv
//     Pointer to an array of char pointers; the result is stored in
//     this array
void getTokens(char* line, char* argv[]) {

// TODO the end line pointer check again instead of the '\0' check

  // loop until line is fully read
  while (*line != '\0') {
    // replace blank space making tokens null terminated strings
    while (*line == ' ' || *line == '\t' || *line == '\n') {
      *line++ = '\0';
    }
    // encountered non-white space, store its address, increment pointer
    *argv++ = line;
    printf
    // skip non-white space
    while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n') {
      line++;
    }
  }
  // null terminate the array to use exec
  *argv = '\0';
}

// Function that executes the commands in argv
// @param argv
//     The commands to execute
void execute(char* argv[]) {
  // fork a child process
  pid_t pid = fork();
  // could not fork
  if (pid == -1) {
    printf("Error: could not fork child process."); // TODO comply with project guidelines
  }
  // child process, execute and daddy proud
  else if (pid == 0) {
    // execute the command, check for error
    if (execvp(argv[0], argv) < 0) {
      printf("%s", "Error: could not execute."); // TODO comply with project guidelines
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
