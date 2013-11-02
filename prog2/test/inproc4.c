#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
int main(int argc, char *argv[]) {
   int fp;
   fp = open(argv[1], O_RDWR, O_CREAT);
   int y = write(fp, argv[2], strlen(argv[2]));
   close(fp);

   return 0;
}
