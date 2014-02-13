#include "schedule.h"
#define MAX_PROCS_AND_ARGS 10

int currentRunningProcIndex = 0;
int numberProcsToRun = 0;
int nasty = 0;
long int quantum;
pid_t pids[MAX_PROCS_AND_ARGS]; /*should be an array for all the pids being run*/
char *progs[MAX_PROCS][MAX_ARGS];
struct itimerval val;

int main(int argc, char *argv[])
{
   int status, s;
   int numSecs;
   int j;
   /* an array of an array of pointers, for CLI args */

   if(argc > ABS_MAX) {
      fprintf(stderr, "Too many arguments, exiting...");
      exit(EXIT_FAILURE);
   }
   quantum = strtol(argv[1], NULL, 10);
   if(quantum <= 0) {
      fprintf(stderr, "Quantum is too low\n");
      return -1;
   }

   zero_out(progs);
   parse_cl(progs, argc, argv);

   for(s = 0; s < MAX_PROCS_AND_ARGS; s++) {
      pids[s] = 0;
   }

   status = installHandler(SIGALRM, timesUp);
   for(s = 0; s < numberProcsToRun; s++) {
      forkChild(s);
   }

   for(s = 0; s < numberProcsToRun; s++) {
      waitpid(pids[s], &status, WNOHANG);
   }

   while(numberProcsToRun > 0) {
      /*have to convert to seconds if longer than that many*/
      numSecs = quantum / MICRO_TO_MILLI;

      val.it_interval.tv_sec = 0;
      val.it_interval.tv_usec = 0;
      val.it_value.tv_sec = numSecs;
      val.it_value.tv_usec = (quantum%MICRO_TO_MILLI)*MICRO_TO_MILLI;
      j = setitimer(ITIMER_REAL, &val, NULL);
      kill(pids[currentRunningProcIndex], SIGCONT);
      waitpid(pids[currentRunningProcIndex], &status, WUNTRACED);

      if(nasty == 1) {/*TIMER WENT OFF*/
         currentRunningProcIndex++;
         if(currentRunningProcIndex > numberProcsToRun - 1) {
            currentRunningProcIndex = 0;
         }
         nasty = 0;
      } else if (nasty == 0) {/*CHILD COMMITTED SUICIDE BEFORE IT'S TIME*/
         /*remove current process from the queue and bump all other procs up*/
         for(j = currentRunningProcIndex; j < numberProcsToRun - 1; j++) {
            pids[j] = pids[j+1];/*bump all the pids up*/
         }
         bumpProgs();
         /*run the next process*/
         numberProcsToRun--;
         if(currentRunningProcIndex > numberProcsToRun - 1) {
            currentRunningProcIndex = 0;
         }
      }
   }
   

   while(numberProcsToRun > 0) {
      pause();/*put this in a loop until all processes finish*/
   }

   wait(&status);

   return 0;
}

void parse_cl(char *progs[MAX_PROCS][MAX_ARGS], int argc, char *argv[]) {
   int i, j, currArg, currProg = 0;

   for(i = 2; i < argc && currProg < MAX_PROCS; ) {
      currArg = 0;
      /* grabbing prog name */
      numberProcsToRun++;
      if(numberProcsToRun > 10) {
         fprintf(stderr, "Too many arguments for a program, exiting\n");
         exit(EXIT_FAILURE);
      }
      progs[currProg][currArg++] = argv[i++];

      for(j = 1; j <= MAX_ARGS && i < argc; j++) {
         if( strcmp(argv[i], ":") == 0)
            break;

         progs[currProg][currArg++] = argv[i++];
      }

      if(j > MAX_ARGS) {
         fprintf(stderr, "Too many arguments for a program, exiting\n");
         exit(EXIT_FAILURE);
      }

      i++;
      currProg++;
   }
}

void zero_out(char *progs[MAX_PROCS][MAX_ARGS]) {
   int i, j;

   for(i = 0; i < MAX_PROCS; i++) {
      for(j = 0; j < MAX_ARGS; j++) {
         progs[i][j] = NULL;
      }
   }
}

void setupTimer(long numMS) {
   /*have to convert to seconds if longer than that many*/
   struct itimerval val;
   int numSecs = numMS / MICRO_TO_MILLI;

   val.it_interval.tv_sec = 0;
   val.it_interval.tv_usec = 0;
   val.it_value.tv_sec = numSecs;
   val.it_value.tv_usec = (numMS%MICRO_TO_MILLI)*MICRO_TO_MILLI ;
   setitimer(ITIMER_REAL, &val, 0);
}

int installHandler(int sig, void (*handler)(int sig)) {
   struct sigaction sa;

   sa.sa_handler = handler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;

   if(sigaction(sig, &sa, NULL) < 0) {
      return -1;
   }

   return 0;
}

void timesUp(int sigalrm) {
   int status;
   sigset_t old, new;

   sigemptyset(&old);
   sigemptyset(&new);
   sigaddset(&new, SIGCHLD);
   sigprocmask(SIG_BLOCK, &new, &old);

   kill(pids[currentRunningProcIndex], SIGSTOP);
   waitpid(pids[currentRunningProcIndex], &status, WUNTRACED);

   val.it_interval.tv_sec = 0;
   val.it_interval.tv_usec = 0;
   val.it_value.tv_sec = 0;
   val.it_value.tv_usec = 0;
   nasty = 1;

   sigalrm = ITS_OVER_NINE_THOUSAND;

   sigprocmask(SIG_UNBLOCK, &new, &old);
}

void bumpProgs() {
   int i, j;

   for(i = currentRunningProcIndex; i < MAX_PROCS - 1; i++) {
      for(j = 0; j < MAX_ARGS; j++) {
         progs[i][j] = progs[i+1][j];
      }
   }
}

void forkChild(int s) {
   int status;

   pids[s] = fork();
   if(pids[s] < 0) {/*change this to not just replace 0*/
      fprintf(stderr, "fork failed\n");
      exit(EXIT_FAILURE);
   }
   else {
      if(pids[s] == 0) {
         /*execvp the command*/
         raise(SIGSTOP);
         execvp(progs[s][0], progs[s]);
      }
      else {
         waitpid(pids[s], &status, WUNTRACED);
      }
   }
}

void testPrint() {
   int i, j;

   for(i = 0; i < MAX_PROCS; i++) {
      if(progs[i][0] != NULL) {
         printf("Program: %s\n", progs[i][0]);
      }

      for(j = 1; j < MAX_ARGS; j++) {
         if(progs[i][j] == NULL || strcmp(progs[i][j], ":") == 0) {
            break;
         }
         printf("Argument %s\n", progs[i][j]);
      }
   }
}
