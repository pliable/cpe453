#include "mond.h"

int main(int argc, char *argv[]) {
   /*

  cparser_t parser;
  char *config_file = NULL;
  int ch, debug = 0, n;
  cparser_result_t rc;

  memset(&parser, 0, sizeof(parser));

  parser.cfg.root = &cparser_root;
  parser.cfg.ch_complete = '\t';
  * 
   * Instead of making sure the terminal setting of the target and 
   * the host are the same. ch_erase and ch_del both are treated
   * as backspace.
   *
  parser.cfg.ch_erase = '\b';
  parser.cfg.ch_del = 127;
  parser.cfg.ch_help = '?';
  parser.cfg.flags = (debug ? CPARSER_FLAGS_DEBUG : 0);
  strcpy(parser.cfg.prompt, PROMPT);
  parser.cfg.fd = STDOUT_FILENO;
  cparser_io_config(&parser);

  if (CPARSER_OK != cparser_init(&parser.cfg, &parser)) {
      printf("Failed to initialize parser.\n");
      return -1;
  }
  
  // Start the parser
  cparser_run(&parser);

      */


   int checkSystemStats = 0, interval = 0, status;
   char procStat[25], procStatm[25];
   pid_t pid = 0;
   time_t t;
   char *executable, *curTime;
   FILE *logFile, *pidstatm, *pidstat;






   int commPoint = 0, i, n, defaultInterval = -1, monitorThreadID = 1, currentPidMon = 0;
   char command[7][35], input[256], *token, defaultLogfile[256];
   monitor_data pids[10];/* 10 for pid/executable monitoring */

   monitor_data system; /* System monitor thread information */
   strcpy(system.pidBeingMonitored, "system");

   monitor_data commandThread; /* "Data" for command thread */
   commandThread.monitorThreadID = 0;
   strcpy(commandThread.pidBeingMonitored, "command");
   time(&commandThread.whenStarted);
   commandThread.monitorInterval = 0;
   strcpy(commandThread.logfile ,"N/A");

   /* write and set cleanup functions for everythign */

   /* initialization */
   for(i = 0; i < 256; i++) {
      defaultLogfile[i] = '\0';
   }


   while(1) {
      /* initialize everything */
      for(i = 0; i < 7; i++) {
         for(n = 0; n < 35; n++) {
            command[i][n] = '\0';
         }
      }
      fgets(input, 256, stdin);
      token = strtok(input, " \n");
      while(token != NULL) {
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
            /* launch thread to monitor system shit. */
            system.monitorInterval = sysInterval;
            strcpy(system.logfile, sysLogfile);
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
         //do listactive stuff
      }
      if(strcmp(command[0], "listcompleted") == 0) {
         //do listcompleted stuff
      }
      if(strcmp(command[0], "remove") == 0) {
         //do remove stuff
      }
      if(strcmp(command[0], "kill") == 0) {
         //do kill stuff
      }
      if(strcmp(command[0], "exit") == 0) {
         //do exit stuff
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

void * systemMonitorHelper(void *ptr) {
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
   log = fopen(sys->logfile, "w");
   time(&t);
   ct = ctime(&t);
   ct[strlen(ct) - 1] = ']';
   fprintf(log, "[%s ", ct);
   fprintf(log, "System  ");

   getStatData(log);
   getMeminfoData(log);
   getLoadavgData(log);
   getDiskstatsData(log);
   fprintf(log, "\n");
   fclose(log);
   usleep(sys->monitorInterval);
   //release lock
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
   char line[256], *format;
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
   char line[256], *format;
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

/** 
 * Below are the "Action Functions" which respond to the specific commands
 * defined in the .cli file
 */

// Note: CLI Parser 0.5 does not support non-nested optional arguments, so the
//       'add' command had to be broken up into 9 distinct Action Functions.
//       For more details, check the manual under CLI Files 5.2

// Also Note: Optional parameters will be set to NULL if the corresponding 
//            optional value is not supplied at the CLI.  For example, see
//            cparser_cmd_add__s__f_filename__i_interval(cparser_context_t *context,
//                char **filename_ptr,
//                    int32_t *interval_ptr)  <-- interval_ptr is optional, and
//                                                can be null!

// Add system monitor using defaults
/*cparser_result_t cparser_cmd_add__s(cparser_context_t *context) {
  printf("Add system monitor using defaults\n");
}

// Add system monitor using set interval (optional filename)
cparser_result_t cparser_cmd_add__s__i_interval__f_filename(cparser_context_t *context,
    int32_t *interval_ptr,
    char **filename_ptr) {
  printf("Add system monitor using set interval (optional filename)\n");
}

// Add system monitor using set filename (optional interval)
cparser_result_t cparser_cmd_add__s__f_filename__i_interval(cparser_context_t *context,
    char **filename_ptr,
    int32_t *interval_ptr) {
  printf("Add system monitor using set filename (optional interval)\n");
}

// Add process monitor to pid using defaults
cparser_result_t cparser_cmd_add__p_pid(cparser_context_t *context,
    int32_t *pid_ptr) {
  printf("Add process monitor to pid using defaults\n");
}

// Add process monitor to pid using set interval (optional filename)
cparser_result_t cparser_cmd_add__p_pid__i_interval__f_filename(cparser_context_t *context,
    int32_t *pid_ptr,
    int32_t *interval_ptr,
    char **filename_ptr) {
  printf("Add process monitor to pid using set interval (optional filename)\n");
}

// Add process monitor to pid using set filename (optional interval)
cparser_result_t cparser_cmd_add__p_pid__f_filename__i_interval(cparser_context_t *context,
    int32_t *pid_ptr,
    char **filename_ptr,
    int32_t *interval_ptr) {
  printf("Add process monitor to pid using set filename (optional interval)\n");
}

// Exec and add process monitor using defaults
cparser_result_t cparser_cmd_add__e_executable(cparser_context_t *context,
    char **executable_ptr) {
  printf("Exec and add process monitor using defaults\n");
}

// Exec and add process monitor using set interval (optional filename)
cparser_result_t cparser_cmd_add__e_executable__i_interval__f_filename(cparser_context_t *context,
    char **executable_ptr,
    int32_t *interval_ptr,
    char **filename_ptr) {
  printf("Exec and add process monitor using set interval (optional filename)\n");
}

// Exec and add process monitor using set filename (optional interval)
cparser_result_t cparser_cmd_add__e_executable__f_filename__i_interval(cparser_context_t *context,
    char **executable_ptr,
    char **filename_ptr,
    int32_t *interval_ptr) {
  printf("Exec and add process monitor using set filename (optional interval)\n");
}

// Set the default interval (in units of microseconds)
cparser_result_t cparser_cmd_set_interval_interval(cparser_context_t *context,
    int32_t *interval_ptr) {
  printf("Set the default interval (in units of microseconds)\n");
}

// Set the default log file
cparser_result_t cparser_cmd_set_logfile_logFileName(cparser_context_t *context,
    char **logFileName_ptr) {
  printf("Set the default log file\n");
}

// Show information about active monitors
cparser_result_t cparser_cmd_listactive(cparser_context_t *context) {
  printf("Show information about active monitors\n");
}

// Show monitors which have been stopped, or for which the target process has
// stopped
cparser_result_t cparser_cmd_listcompleted(cparser_context_t *context) {
  printf("Show monitors which have been stopped, or for which the target process has stopped\n");
}

// Remove the system monitor thread
cparser_result_t cparser_cmd_remove__s(cparser_context_t *context) {
  printf("Remove the system monitor thread\n");
}

// Remove a specific process monitor thread
cparser_result_t cparser_cmd_remove__t_threadID(cparser_context_t *context,
    int32_t *threadID_ptr) {
  printf("Remove a specific process monitor thread\n");
}

// Terminate all threads associated with the process processID, then terminate
// the process
cparser_result_t cparser_cmd_kill_processID(cparser_context_t *context,
    int32_t *processID_ptr) {
  printf("Terminate all threads associated with the process processID, then terminate the process\n");
}

// List all available commands
cparser_result_t cparser_cmd_help_filter (cparser_context_t *context, char **filter) {
  assert(context);
  return cparser_help_cmd(context->parser, filter ? *filter : NULL);
} 
*/

/**
 * Exit the parser test program.
 */
/*cparser_result_t cparser_cmd_exit (cparser_context_t *context) {
  assert(context);
  return cparser_quit(context->parser);
}*/
