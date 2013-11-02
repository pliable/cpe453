#include <stdio.h>
#include <unistd.h>

/**
  program that sleeps for 9 secs, then quits
*/
int main(int argc, char *argv[]) {

   printf("hi i'm gonna sleep now night night\n");
   sleep(10);
   printf("hi i'm up now but bye\n");
   
   return 0;
}
