#include "libDisk.h"
/* for SYS_ERR, first parameter is system call, second is string name of system call */
#define SYS_ERR(eno, sys_call) { if(eno < 0) { perror(sys_call); exit(EXIT_FAILURE); } }

int openDisk(char *filename, int nBytes) {
   int disk, c, offset;
   int numBlocks = nBytes/BLOCKSIZE;
   char data[2] = {MAGIC, FIRSTBLOCKOFFSET};
   superblock sb;
   formatted_block fb;

   SYS_ERR(disk = open(filename, O_RDWR | O_CREAT | O_TRUNC), "open");/* Put numBytes in entry table for this disk */

   SYS_ERR(write(disk, 0, nBytes), "write");
   /*
   for(c = 0; c < nBytes; c++) {
      SYS_ERR(write(disk, 0, 1), "write");
   }
   */
   SYS_ERR(lseek(disk, 0, SEEK_SET), "lseek");//error check yo
   //superblock setup

   sb.type = 1;
   sb.magic = MAGIC;
   sb.blockOffset = FIRSTBLOCKOFFSET;
   sb.finalByte = 0;

   //Writing superblock to disk
   SYS_ERR(write(disk, &sb, sizeof(sb)), "write");
   
   /*
   SYS_ERR(write(disk, "1", 1), "write");
   SYS_ERR(write(disk, data[0], 1), "write");
   SYS_ERR(write(disk, data[1], 1), "write");
   SYS_ERR(write(disk, 0, 1), "write");
   */

   //format the blocks on the disk

   fb.type = 4;
   fb.magic = MAGIC;
   fb.finalByte = 0;

   for(c = 1; c < numBlocks; c++) {//start at block 1 cos block 0 is the super
      SYS_ERR(lseek(disk, BLOCKSIZE*c, SEEK_SET));
      SYS_ERR(write(disk, &fb, sizeof(fb)), "write");
      /*
      SYS_ERR(write(disk, "4", 1), "write");
      SYS_ERR(write(disk, '0x45', 1), "write");
      */
      if(numBlocks - c == 1) {//we are on the final block, so there is no next
         SYS_ERR(write(disk, 0, 1), "write");
      }
      else {
         offset = c*BLOCKSIZE;
         //why array size 1?
         char addr[1];
         sprintf(addr, "%x", offset);
         SYS_ERR(write(disk, addr, 1), "write");
      }
      //This will always execute in the loop...what about final block write?
      SYS_ERR(write(disk, 0, 1), "write");
   }

   return disk;
}

int readBlock(int disk, int bNum, void *block) {
   int offset = bNum*BLOCKSIZE, status;
   /* cHECK to make sure offset < allocated disk bytes */
   SYS_ERR(status = lseek(disk, offset, SEEK_SET), "lseek");
   SYS_ERR(status = read(disk, block, BLOCKSIZE), "read");

   /* this won't be reached if error occurs */
   return status;
}

int writeBlock(int disk, int bNum, void *block) {
   int offset = bNum*BLOCKSIZE, status;
   /* cHECK to make sure offset < allocated disk bytes */
   SYS_ERR(status = lseek(disk, offset, SEEK_SET), "lseek");
   SYS_ERR(status = write(disk, block, BLOCKSIZE), "write");

   /* this won't be reached if error occurs */
   return status;
}
