//change the printfs to fprintfs to the html logfile
void *htmlHelper(void *sys, void *p, void * file) {
   monitor_data *system = (monitor_data *) sys, *pids = (monitor_data *)p; //casting a void * to a monitor_data * (may need **MD)
   char *htmlName = (char *)file;
   FILE *htmlFile = fopen(htmlName, "w");

   //write the header
   //write values into headure using fprintf w/ buncha %s

   //listactive
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
   }
   //listcompleted
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
   }
}
