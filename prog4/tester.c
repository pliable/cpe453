#include "libDisk.c"

int main() {
   int r = openDisk("asdf", 256), status;
   void *a = "asdfasdf\0";
   printf("%d\n", r);
   status = writeBlock(r, 0, a);
   printf("%d\n", status);
   status = readBlock(r, 0, a);
   printf("%d\n%s\n", status, a);

   close(r);
   return 0;
}
