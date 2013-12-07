#include "libTinyFS.h"

int main() {
   int status = 42, status2 = 41, status3 = 40;

   status2 = tfs_mkfs("asdf", DEFAULT_DISK_SIZE);
   //status = tfs_mount("asdf");
   //status3 = tfs_mount("asdf");

   printf("STATUS tfs_mount: %d\n STATUS tfs_mkfs: %d\n STATUS3 tfs_mount: %d\n", status, status2, status3);

   /*
   int r = openDisk("asdf", 10240), status;
   void *a = "asdfasdf\0", *b;
//   printf("%d\n", r);
   status = writeBlock(r, 0, a);
//   printf("%d\n", status);
   b = malloc(256);
   status = readBlock(r, 0, b);
   printf("%d\n%s\n", 123234782578, (char*)b);
   
   
   free(b);
   close(r);
   */
   return 0;
}
