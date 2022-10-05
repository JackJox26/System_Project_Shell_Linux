#include "Process.h"
#define MAX_ARGS 100
void launch_process (process *p, pid_t pgid,int infile, int outfile, int errfile,int foreground)
{
  pid_t pid;

  if (shell_is_interactive)
    {
      /* Put the process into the process group and give the process group
         the terminal, if appropriate.
         This has to be done both by the shell and in the individual
         child processes because of potential race conditions.  */
      pid = getpid ();
      if (pgid == 0) pgid = pid;
      setpgid (pid, pgid);
      if (foreground)
        tcsetpgrp (shell_terminal, pgid);

      /* Set the handling for job control signals back to the default.  */
      signal (SIGINT, SIG_DFL);
      signal (SIGQUIT, SIG_DFL);
      signal (SIGTSTP, SIG_DFL);
      signal (SIGTTIN, SIG_DFL);
      signal (SIGTTOU, SIG_DFL);
      signal (SIGCHLD, SIG_DFL);
    }

  /* Set the standard input/output channels of the new process.  */
  if (infile != STDIN_FILENO)
    {
      dup2 (infile, STDIN_FILENO);
      close (infile);
    }
  if (outfile != STDOUT_FILENO)
    {
      dup2 (outfile, STDOUT_FILENO);
      close (outfile);
    }
  if (errfile != STDERR_FILENO)
    {
      dup2 (errfile, STDERR_FILENO);
      close (errfile);
    }

  /* Exec the new process.  Make sure we exit.  */
  execvp (p->argv[0], p->argv);
  perror ("execvp");
  exit (1);
}


int mark_process_status (pid_t pid, int status)
{
  job *j;
  process *p;
  if (pid > 0)
    {
      /* Update the record for the process.  */
      for (j = get_first_job (); j; j = j->next)
        for (p = j->first_process; p; p = p->next)
          if (p->pid == pid)
            {
                // printf("[%d] %d %s",j->jid,j->pid,j->cmdline);
              p->status = status;
              if (WIFSTOPPED (status))
                p->stopped = 1;
              else
                {
                  p->completed = 1;
                  if (WIFSIGNALED (status))
                    fprintf (stderr, "%d: Terminated by signal %d.\n",
                             (int) pid, WTERMSIG (p->status));
                }
              return 0;
             }
      fprintf (stderr, "No child process %d.\n", pid);
      return -1;
    }

  else if (pid == 0 || errno == ECHILD)
    /* No processes ready to report.  */
    return -1;
  else {
    /* Other weird errors.  */
    perror ("waitpid");
    return -1;
  }
}


void update_status (void)
{
  int status;
  pid_t pid;

  do
    pid = waitpid (WAIT_ANY, &status, WUNTRACED|WNOHANG);
  while (!mark_process_status (pid, status));
}

process *NewProcess(char *commande,job *j){
  // decompose la commande en arguments
  
  char **argv=(char **)malloc(sizeof(char *) * 100);
  
  int argc = 0;
  char *token = strtok(commande, " ");
  while(token != NULL){
    argv[argc] = token;

   if(strcmp(argv[argc],"<") == 0){
    // on a trouve un redirection d'entree
     //on recupere le nom du fichier
      token = strtok(NULL, " ");
      if(token == NULL){
        fprintf(stderr, "Erreur : il manque le nom du fichier d'entree\n");
        return NULL;
      }
     // on ouvre le fichier
      int fd = open(token, O_RDONLY);
       if(fd == -1){
        fprintf(stderr, "Erreur : impossible d'ouvrir le fichier d'entree\n");
       return NULL;
      }
    j->stdin=fd;
      
     }// rechanger redirection des sorties
    else if(strcmp(argv[argc],">") == 0){
      // on a trouve un redirection de sortie
      //on recupere le nom du fichier
      token = strtok(NULL, " ");
      if(token == NULL){
        fprintf(stderr, "Erreur : il manque le nom du fichier de sortie\n");
        return NULL;
      }
      // on ouvre le fichier
      int fd = open(token, O_WRONLY|O_CREAT|O_TRUNC, 0666);
      if(fd == -1){
        fprintf(stderr, "Erreur : impossible d'ouvrir le fichier de sortie\n");
        return NULL;
      }
      j->stdout=fd;
    }
     else{
      argc++;
    }
      
   
    token = strtok(NULL, " ");
  }
  argv[argc] = NULL;
  
  // création du processus
  process *p = (process *)malloc(sizeof(process));
  p->argv = argv; 
   p->next = NULL;
  return p;
}

int check_builtfunc_process(char** argv)
{   
    int argc = 0;
    char** av = argv;
    while (*(av++)) argc++;
    if (argv == NULL) return 1;
    if (!strcmp(argv[0],"cd")) { cd(argv[1]); return 1; }
    if (!strcmp(argv[0],"exit")) { exit(0); return 1; }
    if (strcmp(argv[0],"ls")==0) { ls(argc,argv);return 1;}
    if (strcmp(argv[0],"copy")==0) { copyF(argv[1],argv[2]); return 1;}
  
    return 0;
}