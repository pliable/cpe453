#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {
   int times = strtol(argv[1], NULL, 10);
   int a, k;
   for(a = 0; a < times; a++) {
      for(k = 0; k < times; k++) {
         printf("\t");
      }
      printf("%d\n", times);
      sleep(1);
   }

   return 0;
}
