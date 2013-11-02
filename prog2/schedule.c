#include "schedule.h"
#define MAX_PROCS_AND_ARGS 10

int currentRunningProcIndex = 0;
int numberProcsToRun = 0;
long int quantum;
pid_t pids[MAX_PROCS_AND_ARGS]; //should be an array for all the pids being run
char *progs[MAX_PROCS][MAX_ARGS];

int main(int argc, char *argv[])
{
   int status, s;
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
   /* parse CLI */
   parse_cl(progs, argc, argv);
   //testPrint();

   for(s = 0; s < MAX_PROCS_AND_ARGS; s++) {
      pids[s] = 0;
   }

   //testPrint();
   for(s = 0; s < numberProcsToRun; s++) {
      forkChild(s);
   }

   for(s = 0; s < numberProcsToRun; s++) {
      //untraced?
      printf("current pid: %d\n", pids[s]);
      waitpid(pids[s], &status, WNOHANG);
   }

   //status = installHandler(SIGCHLD, handleChildEndingEarly);
   status = installHandler(SIGALRM, timesUp);
   scheduleProcs();

   while(numberProcsToRun > 0) {
      pause();//put this in a loop until all processes finish
   }

   for(s = 0; s < 10; s++) printf("pid: %d", pids[s]);

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
   //have to convert to seconds if longer than that many
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

void scheduleProcs() {
   int status;

   printf("in scheduleproc\n");

   //resume the paused process
   printf("Your quantum be dis: %d\n", quantum);
   setupTimer(quantum);//move this to before we exec the process
   kill(pids[currentRunningProcIndex], SIGCONT);
   waitpid(pids[currentRunningProcIndex], &status, WUNTRACED);
   handleChildEndingEarly(status);
}

void timesUp(int sigalrm) {
   int status;
   sigset_t old, new;

   sigemptyset(&old);
   sigemptyset(&new);
   sigaddset(&new, SIGCHLD);
   sigprocmask(SIG_BLOCK, &new, &old);

   printf("Quantum Expired\n");
   kill(pids[currentRunningProcIndex], SIGSTOP);
   waitpid(pids[currentRunningProcIndex], &status, WUNTRACED);
   //increment current process pointer
   currentRunningProcIndex++;
   printf("current running proc index times up: %d\n", currentRunningProcIndex);
   //testPrint();
   if(currentRunningProcIndex > numberProcsToRun - 1) {
      currentRunningProcIndex = 0;
   }
   //resume/exec the next process
   //kill(pids[currentRunningProcIndex], SIGCONT);
   sigprocmask(SIG_UNBLOCK, &new, &old);
   scheduleProcs();
}

void handleChildEndingEarly(int sigchld) {
   //block the SIGALRM for this run since we already ended
   int j;
   /*
   sigset_t old, new;

   sigemptyset(&old);
   sigemptyset(&new);
   sigaddset(&new, SIGALRM);
   sigprocmask(SIG_BLOCK, &new, &old);
   */

   printf("Child Ended Early\n");
   //remove current process from the queue and bump all other procs up
   for(j = currentRunningProcIndex; j < numberProcsToRun - 1; j++) {
      pids[j] = pids[j+1];//bump all the pids up
   }
   bumpProgs();
   testPrint();
   //run the next process
   numberProcsToRun--;
   if(numberProcsToRun == 0) raise(SIGINT);
   if(currentRunningProcIndex > numberProcsToRun - 1) {
      currentRunningProcIndex = 0;
   }
   printf("current running process handle child: %d\n", currentRunningProcIndex);
   /*
   sigprocmask(SIG_UNBLOCK, &new, &old);
   */
   scheduleProcs();
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
   //pid_t pid;
   printf("in forkChild()\n");

   pids[s] = fork();
   if(pids[s] < 0) {//change this to not just replace 0
      fprintf(stderr, "fork failed\n");
      exit(EXIT_FAILURE);
   }
   else {
      if(pids[s] == 0) {
         //execvp the command
         raise(SIGSTOP);
         execvp(progs[s][0], progs[s]);
      }
      else {
         printf("pid: %d\n", pids[s]);
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
