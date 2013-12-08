#include "libDisk.h"
#include "tinyFS.h"

int diskSize = 0;
int openDisk(char *filename, int nBytes) {
   int disk;//to make sure bytes is a multiple of the blocksize
   struct stat checker;

   if(lstat(filename, &checker) < 0) {/* Create file if it does not exist */
      SYS_ERR((disk = (open(filename, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))), "open");
   }
   else {
      if(nBytes == 0) { /* If the contents should not be overwritten */
         SYS_ERR((disk = (open(filename, O_RDWR))), "open");
      }
      else { /* If contents can be overwritten */
         SYS_ERR((disk = (open(filename, O_RDWR))), "open");
      }
   }

   SYS_ERR(lseek(disk, nBytes, SEEK_SET), "lseek");

   return disk;
}

int readBlock(int disk, int bNum, void *block) {
   int offset = bNum*BLOCKSIZE, status = 0;
   /* cHECK to make sure offset < allocated disk bytes */
   SYS_ERR((status = lseek(disk, offset, SEEK_SET)), "lseek");
   SYS_ERR((status = read(disk, block, BLOCKSIZE)), "read");

   /* this won't be reached if error occurs */
   return status;
}

int writeBlock(int disk, int bNum, void *block) {
   int offset = bNum*BLOCKSIZE, status = 0;
   /* cHECK to make sure offset < allocated disk bytes */
   SYS_ERR((status = lseek(disk, offset, SEEK_SET)), "lseek");
   SYS_ERR((status = write(disk, block, BLOCKSIZE)), "write");

   /* this won't be reached if error occurs */
   return status;
}
