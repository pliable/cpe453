#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

#define MICRO_TO_MILLI 1000

#define MAX_PROCS 10
/* MAX_ARGS is 11 because program (position 0) + ten arguments (position 10) */
#define MAX_ARGS 11
#define ABS_MAX 102

void zero_out(char *progs[MAX_PROCS][MAX_ARGS]);
void parse_cl(char *progs[MAX_PROCS][MAX_ARGS], int argc, char *argv[]);
void handleChildEndingEarly(int pid);
void forkChild(int s);
int installHandler(int sig, void (*handler)(int sig));
void setupTimer(long numMS);
void timesUp(int pid);
void scheduleProcs();
void bumpProgs();
void testPrint();
