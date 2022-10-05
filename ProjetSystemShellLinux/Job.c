#include "Job.h"


job *first_job = NULL;
pid_t shell_pgid;
struct termios shell_tmodes;
int shell_terminal;
int shell_is_interactive;



job *find_job (pid_t pgid)
{
  job *j;

  for (j = first_job; j; j = j->next)
    if (j->pgid == pgid)
      return j;
  return NULL;
}


int job_is_stopped (job *j)
{
  process *p;

  for (p = j->first_process; p; p = p->next)
    if (!p->completed && !p->stopped)
      return 0;
  return 1;
}


int job_is_completed (job *j)
{
  process *p;

  for (p = j->first_process; p; p = p->next)
    if (!p->completed)
      return 0;
  return 1;
}




void wait_for_job (job *j)
{
  int status;
  pid_t pid;

  do
    pid = waitpid (j->pgid, &status, WUNTRACED);
  while (!mark_process_status (pid, status)
         && !job_is_stopped (j)
         && !job_is_completed (j));
}

void format_job_info (job *j, const char *status)
{
  fprintf (stderr, "%ld (%s): %s\n", (long)j->pgid, status, j->command);
}

void do_job_notification ()
{
  job *j, *jlast, *jnext;

  /* Update status information for child processes.  */
  update_status ();

  jlast = NULL;
  for (j = first_job; j; j = jnext)
    {
      jnext = j->next;

      /* If all processes have completed, tell the user the job has
         completed and delete it from the list of active jobs.  */
      if (job_is_completed (j)) {
        format_job_info (j, "completed");
        if (jlast)
          jlast->next = jnext;
        else
          first_job = jnext;
        free_job (j);
      }

      /* Notify the user about stopped jobs,
         marking them so that we won’t do this more than once.  */
      else if (job_is_stopped (j) && !j->notified) {
        format_job_info (j, "stopped");
        j->notified = 1;
        jlast = j;
      }

      /* Don’t say anything about jobs that are still running.  */
      else
        jlast = j;
    }
}

void launch_job (job *j, int foreground)
{
  process *p;
  pid_t pid;
  int mypipe[2], infile, outfile;

  infile = j->stdin;
  for (p = j->first_process; p; p = p->next)
    {
      /* Set up pipes, if necessary.  */
      if (p->next)
        {
          if (pipe (mypipe) < 0)
            {
              perror ("pipe");
              exit (1);
            }
          outfile = mypipe[1];

        }
      else
        outfile = j->stdout;
    if(!check_builtfunc_process(p->argv)){
        /* Fork the child processes.  */
        pid = fork ();
        if (pid == 0)
          /* This is the child process.  */
          launch_process (p, j->pgid, infile,
                          outfile, j->stderr, foreground);
        else if (pid < 0)
          {
            /* The fork failed.  */
            perror ("fork");
            exit (1);
          }
        else
          {
            /* This is the parent process.  */
            p->pid = pid;
            if (shell_is_interactive)
              {
                if (!j->pgid)
                  j->pgid = pid;
                setpgid (pid, j->pgid);
              }
          }
      }
      /* Clean up after pipes.  */
      if (infile != j->stdin)
        close (infile);
      if (outfile != j->stdout)
        close (outfile);
      infile = mypipe[0];
    }

  //format_job_info (j, "launched");
  if (!shell_is_interactive)
    wait_for_job (j);
  else if (foreground)
    put_job_in_foreground (j, 0);
  else
    put_job_in_background (j, 0);
}


job* NewJob(char *command){
  job *j = (job *)malloc(sizeof(job));
  j->command = command;
  j->stdin = STDIN_FILENO;
  j->stdout = STDOUT_FILENO;
  j->stderr = STDERR_FILENO;
 
  // decouper la command en commande separer par | dans un tableau de char
  // allocate memory for the array of commands
  char **argv = (char **)malloc(sizeof(char *) * 100);
  
  char *token = strtok(command, "|");
  int i=0;
  while(token != NULL){
    argv[i]=token;
    token = strtok(NULL, "|");
    i++;
  }

  //creation du processus dans le job avec le tableau de commande
 process *p=NULL;
  p=NewProcess(argv[0],j);
  j->first_process= p ;

  for(int i=1;i<100;i++){
    if(argv[i]!=NULL){
     p->next=NewProcess(argv[i],j);      
          p=p->next;
    }
  }

  return j;

}



void free_job (job *j)
{
  process *p, *next;

  for (p = j->first_process; p; p = next)
    {
      next = p->next;
      free (p);
    }
  free (j);
}

job *get_first_job (void)
{
  return first_job;
}

void put_job_in_foreground (job *j, int cont)
{

  /* Put the job into the foreground.  */
  tcsetpgrp (shell_terminal, j->pgid);


  /* Send the job a continue signal, if necessary.  */
  if (cont)
    {
      tcsetattr (shell_terminal, TCSADRAIN, &j->tmodes);
      if (kill (- j->pgid, SIGCONT) < 0)
        perror ("kill (SIGCONT)");
    }


  /* Wait for it to report.  */
  wait_for_job (j);

  /* Put the shell back in the foreground.  */
  tcsetpgrp (shell_terminal, shell_pgid);

  /* Restore the shell’s terminal modes.  */
  tcgetattr (shell_terminal, &j->tmodes);
  tcsetattr (shell_terminal, TCSADRAIN, &shell_tmodes);
}

void put_job_in_background (job *j, int cont)
{
  /* Send the job a continue signal, if necessary.  */
  if (cont)
    if (kill (-j->pgid, SIGCONT) < 0)
      perror ("kill (SIGCONT)");
}
void mark_job_as_running (job *j)
{
  process *p;

  for (p = j->first_process; p; p = p->next)
    p->stopped = 0;
  j->notified = 0;
}


/* Continue the job J.  */
void continue_job (job *j, int foreground)
{
  mark_job_as_running (j);
  if (foreground)
    put_job_in_foreground (j, 1);
  else
    put_job_in_background (j, 1);
}
