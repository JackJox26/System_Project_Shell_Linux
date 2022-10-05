#ifndef BUILTFUNC_H_
#define BUILTFUNC_H_
//Used for handling directory files
//For EXIT codes and error handling
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <libgen.h>

/* Change the current working directory */

//void cd(char** args);

/* List directory contents */
void ls(int argc, char *argv[]);
void _ls(const char *dir,int op_a,int op_l);
/* Print a helpful description of how to use the shell */
void help(void);
/* Exit the shell */
void exit_shell(void);
/* Print the current working directory */
void pwd(void);
/* Print the current process id */
char**
parse_command(char *line);

void cd(char* path);

void copyF(char* src, char* dest);

#endif // BUILTFUNC_H_
