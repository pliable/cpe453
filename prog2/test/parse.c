#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"


/*
   * Program
   *
   * @author Steve Choo
*/
int main(int argc, char *argv[]) {
   char *progs[MAX_PROCS][MAX_ARGS];

   if(argc > ABS_MAX) {
      fprintf(stderr, "Too many arguments, exiting...");
      exit(EXIT_FAILURE);
   }

   zero_out(progs);
   parse_cl(progs, argc, argv);
   test_print(progs);
   
   return 0;
}

void parse_cl(char *progs[MAX_PROCS][MAX_ARGS], int argc, char *argv[]) {
   int i, j, currArg, currProg = 0;

   for(i = 2; i < argc && currProg < MAX_PROCS; ) {
      currArg = 0;
      /* grabbing prog name */
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

void test_print(char *progs[MAX_PROCS][MAX_ARGS]) {
   int i, j;

   for(i = 0; i < MAX_PROCS; i++) {
      printf("Program: %s\n", progs[i][0]);

      for(j = 1; j < MAX_ARGS; j++) {
         if(progs[i][j] == NULL || (strcmp(progs[i][j], ":") == 0))
            break;

         printf("  Argument: %s\n", progs[i][j]);
      }
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
