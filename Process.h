#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include<termios.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "Global.h"
#include "Job.h"
#include "Builtfunc.h"

// import job structure
typedef struct job job;
typedef struct process
{
  struct process *next;       /* next process in pipeline */
  char **argv;                /* for exec */
  pid_t pid;                  /* process ID */
  char completed;             /* true if process has completed */
  char stopped;               /* true if process has stopped */
  int status;                 /* reported status value */
} process;

void launch_process (process *p, pid_t pgid, int infile, int outfile, int errfile,int foreground);
int mark_process_status (pid_t pid, int status);
void update_status (void);

// a faire
int check_builtfunc_process(char** argv);
process *NewProcess(char *commande,job *j);


#endif //PROCESS_H