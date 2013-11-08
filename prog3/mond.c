#include "mond.h"


pthread_mutex_t m[11];
int main(int argc, char *argv[]) {

   int checkSystemStats = 0, interval = 0, status;
   char procStat[25], procStatm[25];
   pid_t pid = 0;
   time_t t;
   char *executable, *curTime;
   FILE *logFile, *pidstatm, *pidstat;






   int commPoint = 0, i, n, defaultInterval = -1, shorthandMonitorThreadID = 1, currentPidMon = 0;
   /* for printing purposes, remember that pthread_t is an unsigned long int */
   pthread_t tid;
   void *ret_val;
   char command[7][35], input[BUFFER_SIZE], *token, defaultLogfile[BUFFER_SIZE];
   monitor_data pids[MAX_PIDS];/* 10 for pid/executable monitoring */


   /* initializing shorthandThreadID for future checking purposes */
   for(i = 0; i < MAX_PIDS; i++) {
      pids[i].shorthandThreadID = 0;
   }

   for(i = 0; i < 11; i++) {
      pthread_mutex_init(&m[i], NULL);  //does this actually work?
   }

   /******************* SYSTEM THREAD ***************************/

   monitor_data system; /* System monitor thread information */
   strcpy(system.pidBeingMonitored, "system");

   /****************** COMMAND THREAD **************************/

   monitor_data commandThread; /* "Data" for command thread */
   commandThread.shorthandThreadID = 0;
   strcpy(commandThread.pidBeingMonitored, "command");
   time(&commandThread.whenStarted);
   commandThread.monitorInterval = 0;
   strcpy(commandThread.logfile ,"N/A");

   /* write and set cleanup functions for everythign */

   /* initialization */
   for(i = 0; i < BUFFER_SIZE; i++) {
      defaultLogfile[i] = '\0';
   }


   while(1) {
      /* initialize everything */
      for(i = 0; i < 7; i++) {
         for(n = 0; n < 35; n++) {
            command[i][n] = '\0';
         }
      }
      fgets(input, BUFFER_SIZE, stdin);
      token = strtok(input, " \n");
      while(token != NULL) {
         //This sprintf might break
         sprintf(command[commPoint], token);
         commPoint++;//this is not incrementing wts
         //schoo note: changed above to commPoint++ to see if that
         //makes a difference and increments?
         if(commPoint > 6) {
            printf("Too many arguments\n");
            return -1;
         }
         token = strtok(NULL, " \n");
      }
         // for(i = 0; i < 7; i++) printf("c[i] %s\n", command[i]);
      commPoint = 0;
      if(strcmp(command[0], "add") == 0) {
         if(strcmp(command[1], "-s") == 0) { /* System statistics */
            int sysInterval = defaultInterval;
            char *sysLogfile = defaultLogfile;
            if(strcmp(command[2], "-i") == 0) { /* interval given */
               if((sysInterval = strtol(command[3], NULL, 10)) <= 0) {
                  printf("Please put an actual number for the interval after '-i'.\n");
                  continue;
               }
            }
            if(strcmp(command[2], "-f") == 0) { /* file */
               if(strlen(command[3]) == 0) {
                  printf("Please include a log file to write to after '-f'.\n");
                  continue;
               }
               sysLogfile = command[3];
            }
            if(strcmp(command[4], "-f") == 0) { /* file */
               if(strlen(command[5]) == 0) {
                  printf("Please include a log file to write to after '-f'.\n");
                  continue;
               }
               sysLogfile = command[5];
            }
            /* Check defaults to make sure values were given if no defaults yet */
            if(sysInterval <= 0) {
               printf("A default interval was not set, supply a value for an interval\n");
               continue;
            }
            if(strlen(sysLogfile) <= 0) {
               printf("A default logfile was not set, supply a value for a logfile\n");
               continue;
            }
            if(system.shorthandThreadID != 0) {
               printf("There is already a thread monitoring system statistics.\n");
               continue;
            }

            /* launch thread to monitor system shit. */
            system.monitorInterval = sysInterval;
            strcpy(system.logfile, sysLogfile);
            system.shorthandThreadID = shorthandMonitorThreadID;
            shorthandMonitorThreadID++;
            time(&system.whenStarted); /* get rid of extra \n */
            pthread_create(&system.monitorThreadID, NULL, &systemMonitorHelper, (void *) &system);

            continue;
         }
         if(strcmp(command[1], "-p") == 0) { /* PID to observe */
            int pidInterval = defaultInterval, pid;
            char *pidLogfile = defaultLogfile;
            if((pid = strtol(command[2], NULL, 10)) <= 0) {
               printf("Please indicate a pid to monitor after the '-p'\n");
               continue;
            }
            if(strcmp(command[3], "-i") == 0) { /* interval given */
               if((pidInterval = strtol(command[4], NULL, 10)) <= 0) {
                  printf("Please put an actual number for the interval after '-i'.\n");
                  continue;
               }
            }
            if(strcmp(command[3], "-f") == 0) { /* file */
               if(strlen(command[4]) == 0) {
                  printf("Please include a log file to write to after '-f'.\n");
                  continue;
               }
               pidLogfile = command[4];
            }
            if(strcmp(command[5], "-f") == 0) { /* file */
               if(strlen(command[6]) == 0) {
                  printf("Please include a log file to write to\n");
                  continue;
               }
               pidLogfile = command[6];
            }
            /* Check defaults to make sure values were given if no defaults yet */
            if(pidInterval <= 0) {
               printf("A default interval was not set, supply a value for an interval\n");
               continue;
            }
            if(strlen(pidLogfile) <= 0) {
               printf("A default logfile was not set, supply a value for a logfile\n");
               continue;
            }
            /* launch thread to monitor that pid file */
            continue;
         }

         if(strcmp(command[1], "-e") == 0) { /* new executable to run */
            int execInterval = defaultInterval;
            char *execLogfile = defaultLogfile;
            if(strcmp(command[2], "-i") == 0 || strcmp(command[2], "-f") == 0) {
               printf("Please include an executable to run after the '-e'\n");
               continue;
            }
            if(strcmp(command[3], "-i") == 0) { /* interval given */
               if((execInterval = strtol(command[4], NULL, 10)) <= 0) {
                  printf("Please put an actual number for the interval after '-i'.\n");
                  continue;
               }
            }
            if(strcmp(command[3], "-f") == 0) { /* file */
               if(strlen(command[4]) == 0) {
                  printf("Please include a log file to write to after '-f'.\n");
                  continue;
               }
               execLogfile = command[4];
            }
            if(strcmp(command[5], "-f") == 0) { /* file */
               if(strlen(command[6]) == 0) {
                  printf("Please include a log file to write to after '-f'.\n");
                  continue;
               }
               execLogfile = command[6];
            }
            /* Check defaults to make sure values were given if no defaults yet */
            if(execInterval <= 0) {
               printf("A default interval was not set, supply a value for an interval\n");
               continue;
            }
            if(strlen(execLogfile) <= 0) {
               printf("A default logfile was not set, supply a value for a logfile\n");
               continue;
            }
            /* launch the new executable */
            continue;
         }
         else {//this else always getting triggered. wts
            printf("Usage: add <-s || -p pID || -e executable> [-i interval] [-f logfile]\n");
            return -1;
         }
      }

      if(strcmp(command[0], "set") == 0) {
         if(strcmp(command[1], "interval") == 0) {
            defaultInterval = strtol(command[2], NULL, 10);
            if(defaultInterval <= 0) {
               printf("Need to input a number for the interval\n");
            }
            continue;
         }
         if(strcmp(command[1], "logfile") == 0) {
            if(strlen(command[2]) == 0) {
               printf("Please include a log file after 'logfile'.\n");
               continue;
            }
            strcpy(defaultLogfile, command[2]);
            continue;
         }
         else {
            printf("Usage: set <interval numberInMicroseconds || logfile logfileName>\n");
            return -1;
         }
      }

      if(strcmp(command[0], "listactive") == 0) {
         for(i = 0; i < MAX_PIDS; i++) {
            if(pids[i].shorthandThreadID != 0) {
               printf("Monitoring Thread ID: %d8 | Type: %s8 | Time Started: %s8 | Monitor Interval: %d8 | Log File: %s\n",
                       pids[i].shorthandThreadID, pids[i].pidBeingMonitored, ctime(&pids[i].whenStarted),
                       pids[i].monitorInterval, pids[i].logfile);
            } else {
               break;
            }
            continue;
         }
      }

      if(strcmp(command[0], "listcompleted") == 0) {
         for(i = 0; i < MAX_PIDS; i++) {
            if(pids[i].shorthandThreadID != 0 && pids[i].whenFinished) {
               printf("Monitoring Thread ID: %d8 | Type: %s8 | Time Started: %s8 | Time Completed: %s8 | Monitor Interval: %d8 | Log File: %s\n",
                       pids[i].shorthandThreadID, pids[i].pidBeingMonitored,
                       ctime(&pids[i].whenStarted), ctime(&pids[i].whenFinished),
                       pids[i].monitorInterval, pids[i].logfile);
            } else {
               break;
            }
            continue;
         }
      }

      if(strcmp(command[0], "remove") == 0) {
         int status;
         /* filling out when finished */
         time(&system.whenFinished);

         if(strcmp(command[1], "-s") == 0) {
            /* issue cancel */
            system.shorthandThreadID = 0;
            if( (status = pthread_cancel(system.monitorThreadID)) == ESRCH) {
               fprintf(stderr, "No thread could be found\n");
               continue;
            }

            /* wait for thread to terminate */
            if( (status = pthread_join(system.monitorThreadID, &ret_val)) != 0) {
               switch(status) {
                  case EDEADLK:
                     fprintf(stderr, "Deadlock detected\n");
                     break;
                  case EINVAL:
                     fprintf(stderr, "Thread not joinable or another thread is waiting to join\n");
                     break;
                  case ESRCH:
                     fprintf(stderr, "No thread could be found\n");
                     break;
               }
               continue;
            }
         }

         if(strcmp(command[1], "-t") == 0) {
            if((tid = strtol(command[2], NULL, 10)) <= 0) {
               printf("Please indicate a tid to remove after the '-t'\n");
               continue;
            }

            /* cancelling thread */
            if( (status = pthread_cancel(tid)) == ESRCH) {
               fprintf(stderr, "No thread could be found\n");
            }

            /* wait for thread to terminate */
            if( (status = pthread_join(tid, &ret_val)) != 0) {
               switch(status) {
                  case EDEADLK:
                     fprintf(stderr, "Deadlock detected\n");
                     break;
                  case EINVAL:
                     fprintf(stderr, "Thread not joinable or another thread is waiting to join\n");
                     break;
                  case ESRCH:
                     fprintf(stderr, "No thread could be found\n");
                     break;
               }
               continue;
            }
         }
      }
      
      if(strcmp(command[0], "kill") == 0) {
         //do kill stuff
      }

      if(strcmp(command[0], "exit") == 0) {
         char ans;
         for(i = 0; i < MAX_PIDS; i++) {
            if(pids[i].shorthandThreadID) {
               printf("You still have threads actively monitoring. Do you really want to exit? (y/n) ");
               scanf("%c", &ans);

               if(ans == 'y') {
                  /* closing system logfile */
                  fclose(system.logFP);
                  /* closing command logfile */
                  fclose(commandThread.logFP);

                  for(i = 0; i < MAX_PIDS; i++) {
                     /* ignoring ret vals since closing anyway */
                     fclose(pids[i].logFP);
                  }

                  /* closing threads */

                  pthread_cancel(system.monitorThreadID);
                  /*check here too maybe*/
                  pthread_join(system.monitorThreadID, &ret_val);
                  
                  pthread_cancel(commandThread.monitorThreadID);
                  /*check here too maybe*/
                  pthread_join(commandThread.monitorThreadID, &ret_val);

                  for(i = 0; i < MAX_PIDS; i++) {
                     /*intentionally ignoring ret value here because we 
                       want all thread to be killed anyway */
                     pthread_cancel(pids[i].monitorThreadID);
                  }

                  /* waiting for closed threads */

                  for(i = 0; i < MAX_PIDS; i++) {
                     if( (status = pthread_join(pids[i].monitorThreadID, &ret_val)) != 0) {
                        switch(status) {
                           case EDEADLK:
                              fprintf(stderr, "Deadlock detected\n");
                              break;
                           case EINVAL:
                              fprintf(stderr, "Thread not joinable or another thread is waiting to join\n");
                              break;
                           case ESRCH:
                              fprintf(stderr, "No thread could be found\n");
                              break;
                        }
                     }
                  }

                  exit(EXIT_SUCCESS);
               } else {
                  continue;
               }
            }
         }
      }
      else {
         printf("Not a valid command\n");
         return -1;
      }
   }






   pid = fork();
   if(pid == 0) {
      execlp(executable, executable, NULL);
   }
   else {
      /* Format the dirs for the pid specific system stats  */
      sprintf(procStat, "/proc/%d/stat", pid);
      sprintf(procStatm, "/proc/%d/statm", pid);
      /* The main loop to read the system stats every <interval> ms */
      while((waitpid(pid, &status, WNOHANG)) == 0) { 
         /* Open all the files we need */
         pidstatm = fopen(procStatm, "r");
         pidstat = fopen(procStat, "r");
         if(pidstat == NULL || pidstatm == NULL) {
            break;
         }
         
         /* get the current time */
         time(&t);
         curTime = ctime(&t);
         curTime[strlen(curTime) - 1] = ']';

         /* Collect our data */
         if(checkSystemStats) {
            fprintf(logFile, "[%s ", curTime);
            fprintf(logFile, "System  ");

            getStatData(logFile);
            getMeminfoData(logFile);
            getLoadavgData(logFile);
            getDiskstatsData(logFile);
            fprintf(logFile, "\n");
         }
         fprintf(logFile, "[%s ", curTime);
         fprintf(logFile, "Process(%d)  ", pid);

         getPidStatData(&logFile, &pidstat);
         getPidStatmData(&logFile, &pidstatm);//wtf how do pass FILE pointers in C?

         /* Close everything  */
         fclose(pidstatm);
         fclose(pidstat);
         fprintf(logFile, "\n");
         usleep(interval);
      }
   }
   fclose(logFile);

   return 0;
}

void *systemMonitorHelper(void *ptr) {
   FILE *log;
   time_t t;
   char *ct;
   monitor_data *sys = (monitor_data *) ptr;
   //printf("%02x\n", (unsigned)sys->monitorThreadID);
   //printf("%s\n", sys->pidBeingMonitored);
   //printf("%s\n", ctime(&sys->whenStarted));
   //printf("%d\n", sys->monitorInterval);
   //printf("%s\n", sys->logfile);
   
   //sys monitor always runs until exit  (put a while(1) here)

   //acquire lock
  // pthread_mutex_lock(&mutex);
   sys->logFP = fopen(sys->logfile, "w");
   time(&t);
   ct = ctime(&t);
   ct[strlen(ct) - 1] = ']';
   fprintf(sys->logFP, "[%s ", ct);
   fprintf(sys->logFP, "System  ");

   getStatData(sys->logFP);
   getMeminfoData(sys->logFP);
   getLoadavgData(sys->logFP);
   getDiskstatsData(sys->logFP);
   fprintf(sys->logFP, "\n");
   fclose(sys->logFP);
   usleep(sys->monitorInterval);
   //release lock
  // pthread_mutex_unlock(&mutex);
}

void getStatData(FILE *logfile) {
   char *line = NULL;
   size_t size = 0;
   char *tmp;
   FILE *stat;

   stat = fopen(STAT, "r");

   /* get cpu line */
   getline(&line, &size, stat);
   fprintf(logfile, "[PROCESS] ");
   /* skip "cpu" tok */
   strtok(line, " ");
   fprintf(logfile, "cpuusermode ");
   fprintf(logfile, "%s", strtok(NULL, " "));
   /* skip nice */
   strtok(NULL, " ");
   fprintf(logfile, " cpusystemmode ");
   fprintf(logfile, "%s", strtok(NULL, " "));
   fprintf(logfile, " idletaskrunning ");
   fprintf(logfile, "%s", strtok(NULL, " "));
   fprintf(logfile, " iowaittime ");
   fprintf(logfile, "%s", strtok(NULL, " "));
   fprintf(logfile, " irqservicetime ");
   fprintf(logfile, "%s", strtok(NULL, " "));
   fprintf(logfile, " softirqservicetime ");
   fprintf(logfile, "%s", strtok(NULL, " "));

   /* skipping over individual cpus */
   getline(&line, &size, stat);
   while(strcmp(strtok(line, " "), "intr") != 0) {
      getline(&line, &size, stat);
   }
   fprintf(logfile, " intr ");
   fprintf(logfile, "%s", strtok(NULL, " "));

   /* grab ctxt */
   getline(&line, &size, stat);

   fprintf(logfile, " ctxt ");
   /* skip over ctxt */
   strtok(line, " ");

   /* getting rid of jerk \n */
   tmp = strtok(NULL, " ");
   tmp[strlen(tmp) - 1] = '\0';
   fprintf(logfile, "%s", tmp);

   /*skip over btime*/
   getline(&line, &size, stat);

   /* grab processes */
   getline(&line, &size, stat);
   /* skip "processes" tok */
   strtok(line, " ");

   fprintf(logfile, " forks ");

   /* getting rid of jerk \n */
   tmp = strtok(NULL, " ");
   tmp[strlen(tmp) - 1] = '\0';
   
   fprintf(logfile, "%s", tmp);

   /* grab procs_running */
   getline(&line, &size, stat);
   /* skip "procs_running" tok */
   strtok(line, " ");

   fprintf(logfile, " runnable ");

   /* getting rid of jerk \n */
   tmp = strtok(NULL, " ");
   tmp[strlen(tmp) - 1] = '\0';
   fprintf(logfile, "%s", tmp);

   /* grab procs_blocked */
   getline(&line, &size, stat);
   /* skip over "procs_blocked" tok */
   strtok(line, " ");

   fprintf(logfile, " blocked ");

   /* getting rid of jerk \n */
   tmp = strtok(NULL, " ");
   tmp[strlen(tmp) - 1] = '\0';
   fprintf(logfile, "%s", tmp);

   fclose(stat);
}

void getMeminfoData(FILE *logfile) {
   char *line = NULL;
   size_t size = 0;
   FILE *meminfo;

   meminfo = fopen(MEMINFO, "r");
   fprintf(logfile, " [MEMORY] ");

   /* get MemTotal line */
   getline(&line, &size, meminfo);
   /* skip over "MemTotal" tok */
   strtok(line, " ");
   fprintf(logfile, "memtotal ");
   fprintf(logfile, "%s", strtok(NULL, " "));

   /* get MemFree line */
   getline(&line, &size, meminfo);
   /* skip over "MemFree" tok */
   strtok(line, " ");
   fprintf(logfile, " memfree ");
   fprintf(logfile, "%s", strtok(NULL, " "));

   /* skip buffers line */
   getline(&line, &size, meminfo);

   /* get cached line */
   getline(&line, &size, meminfo);
   strtok(line, " ");
   fprintf(logfile, " cached ");
   fprintf(logfile, "%s", strtok(NULL, " "));

   /* get swapcached line */
   getline(&line, &size, meminfo);
   strtok(line, " ");
   fprintf(logfile, " swapcached ");
   fprintf(logfile, "%s", strtok(NULL, " "));

   /* get active line */
   getline(&line, &size, meminfo);
   strtok(line, " ");
   fprintf(logfile, " active ");
   fprintf(logfile, "%s", strtok(NULL, " "));

   /* get inactive line */
   getline(&line, &size, meminfo);
   strtok(line, " ");
   fprintf(logfile, " inactive ");
   fprintf(logfile, "%s", strtok(NULL, " "));

   fclose(meminfo);
}

void getDiskstatsData(FILE *logfile) {
   char *line = NULL;
   size_t size = 0;
   FILE *diskstats;
   diskstats = fopen(DISKSTATS, "r");

   /* skipping over unnecessary junk */
   while(1) {
      /* grab line */
      getline(&line, &size, diskstats);
      /* skip first token */
      strtok(line, " ");
      /* skip second token */
      strtok(NULL, " ");

      if(!strcmp(strtok(NULL, " "), "sda")) {
         break;
      }
   }

   /* line should be sda at this point */
   fprintf(logfile, " [DISKSTATS(sda)] ");
   fprintf(logfile, "totalnoreads ");
   fprintf(logfile, "%s", strtok(NULL, " "));
   /* skip over reads merged */
   strtok(NULL, " ");
   fprintf(logfile, " totalsectorsread ");
   fprintf(logfile, "%s", strtok(NULL, " "));
   fprintf(logfile, " nomsread ");
   fprintf(logfile, "%s", strtok(NULL, " "));
   fprintf(logfile, " totalnowrites ");
   fprintf(logfile, "%s", strtok(NULL, " "));
   /* skip over writes merged */
   strtok(NULL, " ");
   fprintf(logfile, " nosectorswritten ");
   fprintf(logfile, "%s", strtok(NULL, " "));
   fprintf(logfile, " nomswritten ");
   fprintf(logfile, "%s", strtok(NULL, " "));
   
   fclose(diskstats);
}

void getLoadavgData(FILE *logfile) {
   char *line = NULL;
   size_t size = 0;
   FILE *loadavg; 

   loadavg = fopen(LOADAVG, "r");

   getline(&line, &size, loadavg);

   fprintf(logfile, " [LOADAVG] ");
   fprintf(logfile, "1min ");
   fprintf(logfile, "%s", strtok(line, " "));
   fprintf(logfile, " 5min ");
   fprintf(logfile, "%s", strtok(NULL, " "));
   fprintf(logfile, " 15min ");
   fprintf(logfile, "%s", strtok(NULL, " "));
   fclose(loadavg);
}

void pidhelper (int pid, int interval) { 
      //wait for the pid it is being monitored to end
      //open the proc files
      //call pidstatdata and pidstatmdata
      //sleep
}

void getPidStatData(FILE **logfile, FILE **pidstat) {
   //rss here prints a space afer
   char line[BUFFER_SIZE], *format;
   int field = 0;
   
   fprintf(*logfile, "[STAT] ");
   fgets(line, sizeof(line), *pidstat);
   format = strtok(line, " \n");
   while(format != NULL) {
      if(field == 1) {
         fprintf(*logfile, "executable %s ", format);  
      } else if(field == 2) {
         fprintf(*logfile, "stat %s ", format);  
      } else if(field == 10) {
         fprintf(*logfile, "minorfaults %s ", format);  
      } else if(field == 12) {
         fprintf(*logfile, "majorfaults %s ", format);  
      } else if(field == 14) {
         fprintf(*logfile, "usermodetime %s ", format);  
      } else if(field == 15) {
         fprintf(*logfile, "kernelmodetime %s ", format);
      } else if(field == 18) {
         fprintf(*logfile, "priority %s ", format);  
      } else if(field == 19) {
         fprintf(*logfile, "nice %s ", format);  
      } else if(field == 20) {
         fprintf(*logfile, "nothreads %s ", format);  
      } else if(field == 23) {
         fprintf(*logfile, "vsize %s ", format);  
      } else if(field == 24) {
         fprintf(*logfile, "rss %s ", format);  
      }
      ++field;
      if(field > 24) {
         break;
      }
      format = strtok(NULL, " \n");
   }
}

void getPidStatmData(FILE **logfile, FILE **pidstatm) {
   char line[BUFFER_SIZE], *format;
   int field = 0;
   
   fprintf(*logfile, "[STATM] ");
   fgets(line, sizeof(line), *pidstatm);
   format = strtok(line, " \n");
   while(format != NULL) {
      if(field == 0) {
         fprintf(*logfile, "program %s ", format);
      } else if(field == 1) {
         fprintf(*logfile, "residentset %s ", format);
      } else if(field == 2) {
         fprintf(*logfile, "share %s ", format);
      } else if(field == 3) {
         fprintf(*logfile, "text %s ", format);
      } else if(field == 5) {
         fprintf(*logfile, "data %s", format);
      }
      ++field;
      if(field > 5) {
         break;
      }
      format = strtok(NULL, " \n");
   }
}
