#include "fs.h"

int openDisk(char *filename, int nBytes) {
   int disk;
   int numBlocks = nBytes/BLOCKSIZE;
   disk = open(filename, O_RDWR);/* Put numBytes in entry table for this disk */
   
}

int readBlock(int disk, int bNum, void *block) {
   int offset = bNum*BLOCKSIZE;
   void buffer[BLOCKSIZE];
   /* cHECK to make sure offset < allocated disk bytes */
   lseek(disk, offset, SEEK_SET);
   read(disk, buffer, BLOCKSIZE);

   return 0;
}

int writeBlock(int disk, int bNum, void *block) {
   
}
