#ifndef GLOBAl_H
#define GLOBAl_H
#include <sys/types.h>
#include <termios.h>

extern pid_t shell_pgid;
extern struct termios shell_tmodes;
extern int shell_terminal;
extern int shell_is_interactive;

#endif //GLOBAl_H