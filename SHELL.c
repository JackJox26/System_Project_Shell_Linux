#define MAX_JOBS 100
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h> 
#include <string.h>
#include "Global.h"
#include "Job.h"
#include "Process.h"
#include "Builtfunc.h"

/* recommandation il faut installer les readlines en installant avec // sudo apt install libreadline-dev*/
#define BOLDGREEN   "\033[1m\033[31m"      /* Bold Green */
#define reset       "\033[0m"
#define BOLDBLUE    "\033[1m\033[33m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */



//@author Etudiant Jacques LIU 22106045

void init_shell()
{

  /* See if we are running interactively.  */
  shell_terminal = STDIN_FILENO;
  shell_is_interactive = isatty (shell_terminal);

  if (shell_is_interactive)
    {
      /* Loop until we are in the foreground.  */
      while (tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp ()))
        kill (- shell_pgid, SIGTTIN);
      /* Ignore interactive and job-control signals.  */
      signal (SIGINT, SIG_IGN);
      signal (SIGQUIT, SIG_IGN);
      signal (SIGTSTP, SIG_IGN);
      signal (SIGTTIN, SIG_IGN);
      signal (SIGTTOU, SIG_IGN);
      signal (SIGCHLD, SIG_IGN);

      /* Put ourselves in our own process group.  */
      shell_pgid = getpid ();
      if (setpgid (shell_pgid, shell_pgid) < 0)
        {
          perror ("Couldn't put the shell in its own process group");
          exit (1);
        }

      /* Grab control of the terminal.  */
      tcsetpgrp (shell_terminal, shell_pgid);

      /* Save default terminal attributes for shell.  */
      tcgetattr (shell_terminal, &shell_tmodes);
    }
}



void BievenueScreen(){
        printf("\n\t================================================\n");
        printf("\t          Groupe :  Shell avec C \n");
        printf("\t                 Jacques LIU\n");
        printf("\t================================================\n");
        printf("\n\n");
}

void log_handle(int sig)
{
  FILE *pFile;
  pFile = fopen ("log.txt","a");
  if (pFile == NULL)
    {
      printf ("Error opening file!\n");
      perror("Error opening file");
    }
  else fprintf(pFile,"[LOG] child process terminated\n");
  fclose (pFile);
}


char *PromptCreation()
{
  char* prompt_string=malloc(sizeof(char)*10000);
  char hostname[1024];
        gethostname(hostname, 1024);
        char cwt[10000];
        if (getcwd(cwt, sizeof(cwt)) != NULL){
             sprintf(prompt_string,"%s%s@%s%s:%s%s%s$ ",BOLDMAGENTA,getenv("USER"),hostname,reset,BOLDWHITE, cwt,reset);
        }
        else{
            perror("erreur de prompatage");
            
        }
        return prompt_string;
}

void loopForShell()
{
  signal(SIGCHLD, log_handle);

  int bool_exit=1;
  do
  {
    char *line=NULL;
    char *line2=PromptCreation();
    line=readline(line2);
    add_history(line);
    char* help= line;
    if(strcmp(help, "exit\n") == 0 || strcmp(help, "quit") == 0 || strcmp(help, "leave") == 0)
    {
      bool_exit=0;
    }
    else if(strcmp(help, "") == 0 || strcmp(help, " ") == 0 || strcmp(help, "  ") == 0)
    {
      continue;
    }
    else
    {
      if (strchr(help, '&')!=NULL){
                  char* token=strtok(help,"&");
                  printf("%s\n",token);

                  launch_job(NewJob(token),0);
               }else{
            
                  launch_job(NewJob(help),1);
               }
    } 
  }
  while (bool_exit);
  }

int	main()
{
  BievenueScreen();
  init_shell();
  loopForShell();


    return 0;
}
