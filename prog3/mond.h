#define EXECUTABLE 1
#define INTERVAL 2
#define LOGFILE 3
#define SYSTEM "-s"
#define BASE 10
#define STAT "/proc/stat"
#define MEMINFO "/proc/meminfo"
#define LOADAVG "/proc/loadavg"
#define DISKSTATS "/proc/diskstats"
#define MAX_PIDS 10
#define BUFFER_SIZE 256

#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

typedef struct {
   int mutexIndex;
   char logfile[BUFFER_SIZE];
} logfileMutexKeyVal;

typedef struct {
   /* Need thread ID of pthread monitoring a pid */
   pthread_t monitorThreadID;
   int shorthandThreadID;
   FILE *logFP;
   char pidBeingMonitored[BUFFER_SIZE]; /* "system", "command", or pid */
   time_t whenStarted;
   time_t whenFinished;
   int monitorInterval;
   char logfile[BUFFER_SIZE];
} monitor_data;

void getStatData(FILE *logfile);
void getMeminfoData(FILE *logfile);
void getDiskstatsData(FILE *logfile);
void getLoadavgData(FILE *logfile);
void getPidStatData(FILE **logfile, FILE **pidstat);
void getPidStatmData(FILE **logfile, FILE **pidstatm);
void *systemMonitorHelper(void *sys);
