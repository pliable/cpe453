#include "libDisk.h"

int openDisk(char *filename, int nBytes) {
   int disk, c, offset;
   int numBlocks = nBytes/BLOCKSIZE;
   char data[2] = {MAGIC, FIRSTBLOCKOFFSET};
   disk = open(filename, O_RDWR | O_CREAT | O_TRUNC);/* Put numBytes in entry table for this disk */
   if(disk < 0) {
      return -1;
   }
   for(c = 0; c < nBytes; c++) {
      write(disk, 0, 1);
   }
   lseek(disk, 0, SEEK_SET);//error check yo
   //superblock setup
   write(disk, "1", 1);
   write(disk, data[0], 1);
   write(disk, data[1], 1);
   write(disk, 0, 1);
   //format the blocks on the disk
   for(c = 1; c < numBlocks; c++) {//start at block 1 cos block 0 is the super
      lseek(disk, BLOCKSIZE*c, SEEK_SET);
      write(disk, "4", 1);
      write(disk, data[0], 1);
      if(numBlocks - c == 1) {//we are on the final block, so there is no next
         write(disk, 0, 1);
      }
      else {
         offset = c*BLOCKSIZE;
         char addr[1];
         sprintf(addr, "%x", offset);
         write(disk, addr, 1);
      }
      write(disk, 0, 1);
   }

   return disk;
}

int readBlock(int disk, int bNum, void *block) {
   int offset = bNum*BLOCKSIZE, status;
   unsigned char buffer[BLOCKSIZE];
   /* cHECK to make sure offset < allocated disk bytes */
   status = lseek(disk, offset, SEEK_SET);
   if(status < 0) {
      return -1;
   }
   //status = read(disk, buffer, BLOCKSIZE);
   status = read(disk, block, BLOCKSIZE);
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
