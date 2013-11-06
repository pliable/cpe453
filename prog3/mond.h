#define EXECUTABLE 1
#define INTERVAL 2
#define LOGFILE 3
#define SYSTEM "-s"
#define BASE 10
#define STAT "/proc/stat"
#define MEMINFO "/proc/meminfo"
#define LOADAVG "/proc/loadavg"
#define DISKSTATS "/proc/diskstats"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

void getStatData(FILE *logfile);
void getMeminfoData(FILE *logfile);
void getDiskstatsData(FILE *logfile);
void getLoadavgData(FILE *logfile);
void getPidStatData(FILE **logfile, FILE **pidstat);
void getPidStatmData(FILE **logfile, FILE **pidstatm);

typedef struct stuff {
   /* Need thread ID of pthread monitoring a pid */
   int monitorThreadID; 
   char pidBeingMonitored[256]; /* "system", "command", or pid */
   time_t whenStarted;
   int monitorInterval;
   char *logfile;
} monitor_data;
