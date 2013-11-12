#include "mond.h"


pthread_mutex_t m[10];
logfileMutexKeyVal pairs[10];
int main(int argc, char *argv[]) {
   /* starting shorthandMonitorThreadID at 2 to reserve 1 for the command thread */
   int commPoint = 0, i, n, defaultInterval = -1, shorthandMonitorThreadID = 2, currentPidMon = 0,
       pairsIndex = 0, status;
   /* for printing purposes, remember that pthread_t is an unsigned long int */
   /* use these with ctime_r; can't use ctime back to back because 
      it uses a statically allocated buffer*/
   char ctime_buf[BUFFER_SIZE];
   char ctime_buf2[BUFFER_SIZE];
   void *ret_val;
   char command[7][35], input[BUFFER_SIZE], *token, defaultLogfile[BUFFER_SIZE];
   monitor_data pids[MAX_PIDS];/* 10 for pid/executable monitoring */


   /* initializing vars for future checking purposes */
   for(i = 0; i < MAX_PIDS; i++) {
      pids[i].shorthandThreadID = 0;
      pids[i].whenStarted = 0;
      pids[i].whenFinished = 0;
   }

   for(i = 0; i < 10; i++) {
      pthread_mutex_init(&m[i], NULL);  //does this actually work?
   }

   /******************* SYSTEM THREAD ***************************/

   monitor_data system; /* System monitor thread information */
   system.shorthandThreadID = 0;
   system.whenFinished = 0;
   strcpy(system.pidBeingMonitored, "system");

   /****************** COMMAND THREAD **************************/

   monitor_data commandThread; /* "Data" for command thread */
   commandThread.shorthandThreadID = 1;
   strcpy(commandThread.pidBeingMonitored, "command");
   time(&commandThread.whenStarted);
   commandThread.whenFinished = 0;
   commandThread.monitorInterval = 0;
   strcpy(commandThread.logfile ,"N/A");

   /* write and set cleanup functions for everythign */

   /* initialization */
   /* writing default log file name of "default.log" */
   strncpy(defaultLogfile, "default.log", BUFFER_SIZE);
   /*
   for(i = 0; i < BUFFER_SIZE; i++) {
      defaultLogfile[i] = '\0';
   }
   */


   while(1) {
      /* initialize everything */
      for(i = 0; i < 7; i++) {
         for(n = 0; n < 35; n++) {
            command[i][n] = '\0';
         }
      }
      printf("Command: ");
      fgets(input, BUFFER_SIZE, stdin);
      token = strtok(input, " \n");
      commPoint = 0;
      while(token != NULL) {
         if(commPoint > 6) {
            printf("Too many arguments\n");
            return -1;
         }
         sprintf(command[commPoint], "%s", token);
         commPoint++;
         token = strtok(NULL, " \n");
      }
         // for(i = 0; i < 7; i++) printf("c[i] %s\n", command[i]);
      if(strcmp(command[0], "add") == 0) {
         if(strcmp(command[1], "-s") == 0) { /* System statistics */
            int sysInterval = defaultInterval;
            char *sysLogfile = defaultLogfile;
            /* if system is being added again, resetting when finished as well */
            system.whenFinished = 0;
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

            /* Add the logfile and mutex to the array thang */
            strcpy(pairs[pairsIndex].logfile, sysLogfile);
            pairs[pairsIndex].mutexIndex = pairsIndex;
            pairsIndex++;

            /* launch thread to monitor system shit. */
            strcpy(system.pidBeingMonitored, "system");
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

            /* Add the logfile and mutex to the array thang */
            strcpy(pairs[pairsIndex].logfile, pidLogfile);
            pairs[pairsIndex].mutexIndex = pairsIndex;
            pairsIndex++;

            /* find first available pids slot */
            for(i = 0; i < MAX_PIDS; i++) {
               if(pids[i].shorthandThreadID == 0) {
                  break;
               }
            }

            /* i be the index we want now */

            /* fill pids[i] with info */
            strcpy(pids[i].pidBeingMonitored, command[2]);
            pids[i].monitorInterval = pidInterval;
            strcpy(pids[i].logfile, pidLogfile);
            pids[i].shorthandThreadID = shorthandMonitorThreadID++;
            time(&pids[i].whenStarted);
            /* launch thread to monitor PID shit. */
            pthread_create(&pids[i].monitorThreadID, NULL, &pidMonitorHelper, (void *)&pids[i]);

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
            pid_t new_pid = vfork();
            if(new_pid < 0) {
               perror("vfork");
               exit(EXIT_FAILURE);
            }

            /* parent stuff */
            if(new_pid) {
               /* find first available slot for pid table */
               for(i = 0; i < MAX_PIDS; i++) {
                  if(pids[i].shorthandThreadID == 0) {
                     break;
                  }
               }

               /* Add the logfile and mutex to the array thang */
               strcpy(pairs[pairsIndex].logfile, execLogfile);
               pairs[pairsIndex].mutexIndex = pairsIndex;
               pairsIndex++;

               /* converting long to string */
               int n = snprintf(NULL, 0, "%dl", new_pid);
               if(n < 0) {
                  printf("snprintf fail");
                  continue;
               }
               char buf[n + 1];
               int c = snprintf(buf, n + 1, "%dl", new_pid);
               if(c < 0) {
                  printf("snprintf fail again");
                  continue;
               }

               /* fill pids[i] with info */
               strcpy(pids[i].pidBeingMonitored, buf);
               pids[i].monitorInterval = execInterval;
               strcpy(pids[i].logfile, execLogfile);
               pids[i].shorthandThreadID = shorthandMonitorThreadID++;
               time(&pids[i].whenStarted);
               /* launch thread to monitor system shit. */
               pthread_create(&pids[i].monitorThreadID, NULL, &execMonitorHelper, (void *)&pids[i]);

               continue;

            } else { /* child stuff */
              if(execlp(command[2], command[2], (char *)NULL) < 0) {
                 perror("execlp");
                 exit(EXIT_FAILURE);
              }
            }

            continue;
         }
         else {//this else always getting triggered. wts
            printf("Usage: add <-s || -p pID || -e executable> [-i interval] [-f logfile]\n");
            continue;
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
            continue;
         }
      }

      if(strcmp(command[0], "listactive") == 0) {
         /* printing command thread */
         ctime_r(&commandThread.whenStarted, ctime_buf);
         ctime_buf[strlen(ctime_buf) - 1] = '\0';

         printf("Monitoring Thread ID: %8d | Type: %8s | Time Started: %s | Monitor Interval: %8d | Log File: %s\n",
                 commandThread.shorthandThreadID, commandThread.pidBeingMonitored, ctime_buf,
                 commandThread.monitorInterval, commandThread.logfile);

         /* printing system monitor */
         if(system.shorthandThreadID && (system.whenFinished == 0)) {
            /* getting rid of of \n */
            ctime_r(&system.whenStarted, ctime_buf);
            ctime_buf[strlen(ctime_buf) - 1] = '\0';

            printf("Monitoring Thread ID: %8d | Type: %8s | Time Started: %s | Monitor Interval: %8d | Log File: %s\n",
                    system.shorthandThreadID, system.pidBeingMonitored, ctime_buf,
                    system.monitorInterval, system.logfile);
         }

         /* printing monitor threads */
         for(i = 0; i < MAX_PIDS; i++) {
            if(pids[i].shorthandThreadID && (pids[i].whenFinished == 0)) {
               ctime_r(&pids[i].whenStarted, ctime_buf);
               ctime_buf[strlen(ctime_buf) - 1] = '\0';

               printf("Monitoring Thread ID: %8d | Type: %8s | Time Started: %s | Monitor Interval: %8d | Log File: %s\n",
                       pids[i].shorthandThreadID, pids[i].pidBeingMonitored, ctime_buf,
                       pids[i].monitorInterval, pids[i].logfile);
            } 
         }

         continue;
      }

      if(strcmp(command[0], "listcompleted") == 0) {
         /* printing system monitor */
         if(system.whenFinished) {
            /* getting rid of of \n */
            ctime_r(&system.whenStarted, ctime_buf);
            ctime_r(&system.whenFinished, ctime_buf2);
            ctime_buf[strlen(ctime_buf) - 1] = '\0';
            ctime_buf2[strlen(ctime_buf2) - 1] = '\0';

            printf("Monitoring Thread ID: %8d | Type: %8s | Time Started: %s | Time Finished: %s | Monitor Interval: %8d | Log File: %s\n",
                    system.shorthandThreadID, system.pidBeingMonitored, ctime_buf, ctime_buf2,
                    system.monitorInterval, system.logfile);
         }

         /* printing command thread */
         for(i = 0; i < MAX_PIDS; i++) {
            if(pids[i].whenFinished) {
               ctime_r(&pids[i].whenStarted, ctime_buf);
               ctime_r(&pids[i].whenFinished, ctime_buf2);
               ctime_buf[strlen(ctime_buf) - 1] = '\0';
               ctime_buf2[strlen(ctime_buf2) - 1] = '\0';

               printf("Monitoring Thread ID: %8d | Type: %8s | Time Started: %s | Time Completed: %s | Monitor Interval: %8d | Log File: %s\n",
                       pids[i].shorthandThreadID, pids[i].pidBeingMonitored,
                       ctime_buf, ctime_buf2, pids[i].monitorInterval, pids[i].logfile);
            } else {
               break;
            }
         }
         continue;
      }

      if(strcmp(command[0], "remove") == 0) {

         if(strcmp(command[1], "-s") == 0) {
            system.shorthandThreadID = 0;
            /* filling out when finished */
            time(&system.whenFinished);
            /* issue cancel */
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
            continue;
         }

         if(strcmp(command[1], "-t") == 0) {
            int sThreadID;

            if((sThreadID = strtol(command[2], NULL, 10)) <= 0) {
               fprintf(stderr, "Please indicate a tid to remove after the '-t'\n");
               continue;
            }

            /* searching for thread */
            for(i = 0; i < MAX_PIDS; i++) {
               if(pids[i].shorthandThreadID == sThreadID) {
                  break;
               }
            }

            /* if i is MAX_PIDS, thread couldn't be found */
            if(i == MAX_PIDS) {
               fprintf(stderr, "Thread could not be found, please try again\n");
               continue;
            }
               

            /* i is the index we want now */

            /* checking if anything else is writing to same log file, and if
             there isn't, close it */

            time(&pids[i].whenFinished);

            /* cancelling thread */
            if( (status = pthread_cancel(pids[i].monitorThreadID)) == ESRCH) {
               fprintf(stderr, "No thread could be found\n");
            }

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

            /* setting to zero to signify this slot is empty */
            pids[i].shorthandThreadID = 0;
            continue;
         } else {
            printf("Usage: remove <-s || -t threadID>\n");
            continue;
         }
      }
      
      if(strcmp(command[0], "kill") == 0) {
         for(i = 0; i < MAX_PIDS; i++) {
            /* kill all threads associated with process */
            if( strcmp(pids[i].pidBeingMonitored, command[1]) == 0) {
               pthread_cancel(pids[i].monitorThreadID);

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
         }

         long local_pid = strtol(command[1], NULL, 10);

         if(i == MAX_PIDS || local_pid == 0) {
            fprintf(stderr, "PID does not exist or is malformed\n");
            continue;
         }

         /* kill process */
         kill(local_pid, SIGKILL);
      }

      if(strcmp(command[0], "exit") == 0) {
         char ans = 'h';

         for(i = 0; i < MAX_PIDS; i++) {
            if(pids[i].shorthandThreadID) {
               printf("You still have threads actively monitoring. Do you really want to exit? (y/n) ");
               scanf("%c", &ans);
               break;
            }
         }

         if(system.shorthandThreadID && ans == 'h') {
            printf("You still have threads actively monitoring. Do you really want to exit? (y/n) ");
            scanf("%c", &ans);
         }

         if(ans == 'y' || i == MAX_PIDS) {
            /* closing threads */

            /* closing system monitor if open */
            if(system.shorthandThreadID) {
               /* closing system logfile */
               pthread_cancel(system.monitorThreadID);
               pthread_join(system.monitorThreadID, &ret_val);
            }

            /* closing monitor threads */
            for(i = 0; i < MAX_PIDS; i++) {
               if(pids[i].shorthandThreadID) {
                  /* closing log file */

                  /*intentionally ignoring ret value here because we 
                    want all thread to be killed anyway */
                  pthread_cancel(pids[i].monitorThreadID);
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
            }

            exit(EXIT_SUCCESS);
         } else {
            continue;
         }
      }
      else {
         printf("Not a valid command\n");
         continue;
      }
   }

   return 0;
}

void *execMonitorHelper(void *ptr) {
   time_t t;
   pid_t pid; 
   char *ct, procStat[25], procStatm[25];
   int y, whichMutexToUse = 0, status;
   FILE *pidstatm, *pidstat;
   monitor_data *sys = (monitor_data *) ptr;

   //printf("%02x\n", (unsigned)sys->monitorThreadID);
   //printf("%s\n", sys->pidBeingMonitored);
   //printf("%s\n", ctime(&sys->whenStarted));
   //printf("%d\n", sys->monitorInterval);
   //printf("%s\n", sys->logfile);

   pid = (pid_t)strtoimax(sys->pidBeingMonitored, NULL, 10);
   /* Match up the logfile with the mutexxx */
   for(y = 0; y < 10; y++) {
      if(strcmp(sys->logfile, pairs[y].logfile) == 0) {
         whichMutexToUse = pairs[y].mutexIndex;
         break;
      }
   }
   sprintf(procStat, "/proc/%d/stat", pid);
   sprintf(procStatm, "/proc/%d/statm", pid);
   //set cancel type
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
   //wait for the pid it is being monitored to end
   while((waitpid(pid, &status, WNOHANG)) == 0) {
   //while(1) {
      /* attain the lock */
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
      pthread_mutex_lock(&m[whichMutexToUse]);

      /* Open everything */
      sys->logFP = fopen(sys->logfile, "a");
      pidstatm = fopen(procStatm, "r");
      pidstat = fopen(procStat, "r");
      if(pidstat == NULL || pidstatm == NULL) {
         printf("Process no longer exists. Exiting process monitor thread.\n");
         fclose(sys->logFP);
         break;
      }

      /* Begin statistic tracking */
      time(&t);
      ct = ctime(&t);
      ct[strlen(ct) - 1] = ']';
      fprintf(sys->logFP, "[%s ", ct);
      fprintf(sys->logFP, "Process  ");
      getPidStatData(&sys->logFP, &pidstat);
      getPidStatmData(&sys->logFP, &pidstatm);//wtf how do pass FILE pointers in C?

      /* Close everything  */
      fclose(pidstatm);
      fclose(pidstat);
      fprintf(sys->logFP, "\n");
      fclose(sys->logFP);

      pthread_mutex_unlock(&m[whichMutexToUse]);
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      usleep(sys->monitorInterval);
   }
}

void *pidMonitorHelper(void *ptr) {
   time_t t;
   pid_t pid; 
   char *ct, procStat[25], procStatm[25];
   int y, whichMutexToUse = 0, status;
   FILE *pidstatm, *pidstat;
   monitor_data *sys = (monitor_data *) ptr;

   //printf("%02x\n", (unsigned)sys->monitorThreadID);
   //printf("%s\n", sys->pidBeingMonitored);
   //printf("%s\n", ctime(&sys->whenStarted));
   //printf("%d\n", sys->monitorInterval);
   //printf("%s\n", sys->logfile);

   pid = (pid_t)strtoimax(sys->pidBeingMonitored, NULL, 10);
   /* Match up the logfile with the mutexxx */
   for(y = 0; y < 10; y++) {
      if(strcmp(sys->logfile, pairs[y].logfile) == 0) {
         whichMutexToUse = pairs[y].mutexIndex;
         break;
      }
   }
   sprintf(procStat, "/proc/%d/stat", pid);
   sprintf(procStatm, "/proc/%d/statm", pid);
   //set cancel type
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
   //wait for the pid it is being monitored to end
   //while((waitpid(pid, &status, WNOHANG)) == 0) {
   while(1) {
      /* attain the lock */
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
      pthread_mutex_lock(&m[whichMutexToUse]);

      /* Open everything */
      sys->logFP = fopen(sys->logfile, "a");
      pidstatm = fopen(procStatm, "r");
      pidstat = fopen(procStat, "r");
      if(pidstat == NULL || pidstatm == NULL) {
         printf("Process no longer exists. Exiting process monitor thread.\n");
         fclose(sys->logFP);
         break;
      }

      /* Begin statistic tracking */
      time(&t);
      ct = ctime(&t);
      ct[strlen(ct) - 1] = ']';
      fprintf(sys->logFP, "[%s ", ct);
      fprintf(sys->logFP, "Process  ");
      getPidStatData(&sys->logFP, &pidstat);
      getPidStatmData(&sys->logFP, &pidstatm);//wtf how do pass FILE pointers in C?

      /* Close everything  */
      fclose(pidstatm);
      fclose(pidstat);
      fprintf(sys->logFP, "\n");
      fclose(sys->logFP);

      pthread_mutex_unlock(&m[whichMutexToUse]);
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      usleep(sys->monitorInterval);
   }
}

void *systemMonitorHelper(void *ptr) {
   time_t t;
   char *ct;
   int y, whichMutexToUse = 0;
   monitor_data *sys = (monitor_data *) ptr;
   //printf("%02x\n", (unsigned)sys->monitorThreadID);
   //printf("%s\n", sys->pidBeingMonitored);
   //printf("%s\n", ctime(&sys->whenStarted));
   //printf("%d\n", sys->monitorInterval);
   //printf("%s\n", sys->logfile);
   
   //sys monitor always runs until exit  (put a while(1) here)


   /* Match up the logfile with the mutexxx */
   for(y = 0; y < 10; y++) {
      if(strcmp(sys->logfile, pairs[y].logfile) == 0) {
         whichMutexToUse = pairs[y].mutexIndex;
         break;
      } 
   }
   //set cancel type
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
   while(1) {
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
      pthread_mutex_lock(&m[whichMutexToUse]);
      sys->logFP = fopen(sys->logfile, "a");

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

      pthread_mutex_unlock(&m[whichMutexToUse]);
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      usleep(sys->monitorInterval);
   }
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
