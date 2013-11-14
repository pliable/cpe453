//change the printfs to fprintfs to the html logfile
void *htmlHelper(void *sys, void *p, void * file) {
   monitor_data *system = (monitor_data *) sys, *pids = (monitor_data *)p; //casting a void * to a monitor_data * (may need **MD)
   char *htmlName = (char *)file;
   FILE *htmlFile = fopen(htmlName, "w");

   //write the header
   fprintf(htmlFile, "<html>\n<head>\n<title>System Monitor - Web extension by Kevin Stein and Steve Choo</title>\n<meta http-equiv=\"refresh\" content="2">\n</head>");
   fprintf(htmlFile, "<body>\n<h2>System Monitor - Web extension</h2>\n<p>by Kevin Stein and Steve Choo | CPE 453 Winter 2012 | <a href=\"http://www.csc.calpoly.edu/~foaad\">Prof. Foaad Khosmood</a></p>\n\n");
   fprintf(htmlFile, "<h3>Settings</h3>\n<ul>\n\t<li>webmon refresh rate = 1 second</li>\n\t<li>html refresh rate = 2 seconds</li>\n</ul>\n\n");
   //write values into headure using fprintf w/ buncha %s


   //listactive
   //write listactive header shit
   fprintf(htmlFile, "<h3>Active threads</h3>\n<table border=\"1\",cellpadding=\"2\">\n<tr>\n\t<td>thread ID</td>\n\t<td>process ID</td>\n\t<td>time</td>\n\t<td>interval (&#956sec)</td>\n\t<td>log file</td>\n</tr>\n\n");

   //actual code ffrom mond
   time_r(&commandThread.whenStarted, ctime_buf);
   ctime_buf[strlen(ctime_buf) - 1] = '\0';
   fprintf(htmlFile, "<tr>\n\t\t<td>%8d</td>\n\t\t<td>%8s</td>\n\t<td>%s</td>\n\t\t<td>%8d</td>\n\t\t<td>%s</td>\n</tr>\n\n", 
       commandThread.shorthandThreadID, commandThread.pidBeingMonitored, ctime_buf, commandThread.monitorInterval, commandThread.logfile);
   /*printf("Monitoring Thread ID: %8d | Type: %8s | Time Started: %s | Monitor Interval: %8d | Log File: %s\n",
           commandThread.shorthandThreadID, commandThread.pidBeingMonitored, ctime_buf,
           commandThread.monitorInterval, commandThread.logfile);*/

   /* printing system monitor */
   if(system.shorthandThreadID && (system.whenFinished == 0)) {
      /* getting rid of of \n */
      ctime_r(&system.whenStarted, ctime_buf);
      ctime_buf[strlen(ctime_buf) - 1] = '\0';

      fprintf(htmlFile, "<tr>\n\t\t<td>%8d</td>\n\t\t<td>%8s</td>\n\t<td>%s</td>\n\t\t<td>%8d</td>\n\t\t<td>%s</td>\n</tr>\n\n", 
       system.shorthandThreadID, system.pidBeingMonitored, ctime_buf, system.monitorInterval, system.logfile);
   }
      

      /*printf("Monitoring Thread ID: %8d | Type: %8s | Time Started: %s | Monitor Interval: %8d | Log File: %s\n",
              system.shorthandThreadID, system.pidBeingMonitored, ctime_buf,
              system.monitorInterval, system.logfile);
      }*/

      /* printing monitor threads */
      for(i = 0; i < MAX_PIDS; i++) {
         if(pids[i].shorthandThreadID && (pids[i].whenFinished == 0)) {
            ctime_r(&pids[i].whenStarted, ctime_buf);
            ctime_buf[strlen(ctime_buf) - 1] = '\0';

            fprintf(htmlFile, "<tr>\n\t\t<td>%8d</td>\n\t\t<td>%8s</td>\n\t<td>%s</td>\n\t\t<td>%8d</td>\n\t\t<td>%s</td>\n</tr>\n\n", 
             pids[i].shorthandThreadID, pids[i].pidBeingMonitored, ctime_buf, pids[i].monitorInterval, pids[i].logfile);

            /*printf("Monitoring Thread ID: %8d | Type: %8s | Time Started: %s | Monitor Interval: %8d | Log File: %s\n",
                    pids[i].shorthandThreadID, pids[i].pidBeingMonitored, ctime_buf,
                    pids[i].monitorInterval, pids[i].logfile);*/
         } 
      }
         

   //listcompleted

   //write html header
   fprintf(htmlFile, "<h3>Active threads</h3>\n<table border=\"1\",cellpadding=\"2\">\n<tr>\n\t<td>thread ID</td>\n\t<td>process ID</td>\n\t<td>time</td>\n\t<td>interval   (&#956sec)</td>\n\t<td>log file</td>\n\t<td>time finished</td>\n</tr>\n\n");

   //actual mond code 
   if(system.whenFinished) {
      /* getting rid of of \n */
      ctime_r(&system.whenStarted, ctime_buf);
      ctime_r(&system.whenFinished, ctime_buf2);
      ctime_buf[strlen(ctime_buf) - 1] = '\0';
      ctime_buf2[strlen(ctime_buf2) - 1] = '\0';

      fprintf(htmlFile, "<tr>\n\t\t<td>%8d</td>\n\t\t<td>%8s</td>\n\t<td>%s</td>\n\t\t<td>%8d</td>\n\t\t<td>%s</td>\n\t<td>Wed Feb 22 16:19:00 PST 2012</td>\n</tr>\n\n", 
       system.shorthandThreadID, system.pidBeingMonitored, ctime_buf, system.monitorInterval, system.logfile, ctime_buf2);

      /*printf("Monitoring Thread ID: %8d | Type: %8s | Time Started: %s | Time Finished: %s | Monitor Interval: %8d | Log File: %s\n",
              system.shorthandThreadID, system.pidBeingMonitored, ctime_buf, ctime_buf2,
              system.monitorInterval, system.logfile);*/
   }

   /* printing command thread */
   for(i = 0; i < MAX_PIDS; i++) {
      if(pids[i].whenFinished) {
         ctime_r(&pids[i].whenStarted, ctime_buf);
         ctime_r(&pids[i].whenFinished, ctime_buf2);
         ctime_buf[strlen(ctime_buf) - 1] = '\0';
         ctime_buf2[strlen(ctime_buf2) - 1] = '\0';

      fprintf(htmlFile, "<tr>\n\t\t<td>%8d</td>\n\t\t<td>%8s</td>\n\t<td>%s</td>\n\t\t<td>%8d</td>\n\t\t<td>%s</td>\n\t<td>Wed Feb 22 16:19:00 PST 2012</td>\n</tr>\n\n", 
       pids[i].shorthandThreadID, pids[i].pidBeingMonitored, ctime_buf, pids[i].monitorInterval, pids[i].logfile, ctime_buf2);

         /*printf("Monitoring Thread ID: %8d | Type: %8s | Time Started: %s | Time Completed: %s | Monitor Interval: %8d | Log File: %s\n",
                 pids[i].shorthandThreadID, pids[i].pidBeingMonitored,
                 ctime_buf, ctime_buf2, pids[i].monitorInterval, pids[i].logfile);*/
      } else {
         break;
      }
   }
   fclose(htmlFile);
}
