/*
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * You should use this (very simple) starter code as a basis for
 * building your shell.  Please see the project handout for more
 * details.
 */

#ifndef _3600sh_h
#define _3600sh_h

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>

void do_exit();
void prompt();
void processLine();
int getTokens(char* line, char* argv[]);
void execute(int argc, char* argv[]);
int is_redirect(char* s);

#include <stdio.h>

/*
  Gets the next line of input from the console.
  Call free on the resulting char when done, to free up space.
*/
char* getline() {
  size_t size = 0; // max num of chars
  const size_t resize = 64; // "optimal resize"
  size_t len  = 0; // length of line
  char* line = NULL; // desired line of input

  // read until EOF or newline encountered
  do {
    size += resize;
    line = realloc(line, size); // initially malloc(NULL)
    fgets(line + len, resize, stdin);
    len = strlen(line);
  } while (!feof(stdin) && line[len - 1] != '\n');

  return line;
}

#endif 
