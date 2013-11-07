#define EXECUTABLE 1
#define INTERVAL 2
#define LOGFILE 3
#define SYSTEM "-s"
#define BASE 10
#define STAT "/proc/stat"
#define MEMINFO "/proc/meminfo"
#define LOADAVG "/proc/loadavg"
#define DISKSTATS "/proc/diskstats"

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
//#include "cparser.h"
//#include "cparser_token.h"
//#include "cparser_tree.h"

#define PROMPT "> "

typedef struct stuff {
   /* Need thread ID of pthread monitoring a pid */
   pthread_t monitorThreadID; 
   char pidBeingMonitored[256]; /* "system", "command", or pid */
   time_t whenStarted;
   int monitorInterval;
   char logfile[256];
} monitor_data;

void getStatData(FILE *logfile);
void getMeminfoData(FILE *logfile);
void getDiskstatsData(FILE *logfile);
void getLoadavgData(FILE *logfile);
void getPidStatData(FILE **logfile, FILE **pidstat);
void getPidStatmData(FILE **logfile, FILE **pidstatm);
void * systemMonitorHelper(void *sys);

