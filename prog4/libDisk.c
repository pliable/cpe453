#include "fs.h"

int openDisk(char *filename, int nBytes) {
   int disk, c;
   int numBlocks = nBytes/BLOCKSIZE;
   disk = open(filename, O_RDWR);/* Put numBytes in entry table for this disk */
   if(disk < 0) {
      return -1;
   }
   for(c = 0; c < nBytes; c++) {
      write(disk, 0, 1);
   }

   return disk;
}

int readBlock(int disk, int bNum, void *block) {
   int offset = bNum*BLOCKSIZE, status;
   void buffer[BLOCKSIZE];
   /* cHECK to make sure offset < allocated disk bytes */
   status = lseek(disk, offset, SEEK_SET);
   if(status < 0) {
      return -1;
   }
   status = read(disk, buffer, BLOCKSIZE);
   if(status < 0) {
      return -1;
   }

   return 0;
}

int writeBlock(int disk, int bNum, void *block) {
   int offset = bNum*BLOCKSIZE, status;
   /* cHECK to make sure offset < allocated disk bytes */
   status = lseek(disk, offset, SEEK_SET);
   if(status < 0) {
      return -1;
   }
   status = write(disk, block, BLOCKSIZE);
   if(status < 0) {
      return -1;
   }

   return 0;
}
