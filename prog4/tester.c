#include "libDisk.c"

int main() {
   int r = openDisk("asdf", 10240), status;
   void *a = "asdfasdf\0", *b;
//   printf("%d\n", r);
   //status = writeBlock(r, 0, a);
//   printf("%d\n", status);
   b = malloc(256);
   //status = readBlock(r, 0, b);
   printf("%d\n%s\n", 123234782578, (char*)b);
   
   
   free(b);
   close(r);
   return 0;
}
