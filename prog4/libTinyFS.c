#include "libTinyFS.h"

fileDescriptor globalFP = 0;
struct node head;

int tfs_mkfs(char *filename, int nBytes) {
   int disk, c, offset;
   int numBlocks = nBytes/BLOCKSIZE;
   int lengthOfBitVector = numBlocks / 8;
   superblock sb;
   formatted_block fb;

   disk = openDisk(filename, nBytes);

   for(c = 0; c < nBytes; c++) {
      SYS_ERR(write(disk, "\0", 1), "write");
   }

   SYS_ERR(lseek(disk, 0, SEEK_SET), "lseek");//error check yo
   //superblock setup

   sb.type = 1;
   sb.magic = MAGIC;
   sb.byteOffset = sizeof(superblock) + lengthOfBitVector;
   sb.finalByte = 0;

   //Writing superblock to disk
   SYS_ERR(write(disk, &sb, sizeof(sb)), "write");
   
   //format the blocks on the disk

   fb.type = 4;
   fb.magic = MAGIC;
   fb.finalByte = 0;

   for(c = 1; c < numBlocks; c++) {//start at block 1 cos block 0 is the super
      SYS_ERR(lseek(disk, BLOCKSIZE*c, SEEK_SET), "lseek");

      if(c + 1 > numBlocks) {//we are on the final block, so there is no next
         fb.blockAddress = "\0";
      }
      else {
         fb.blockAddress = c + 1;
         offset = c*BLOCKSIZE;
      }
      //This will always execute in the loop...what about final block write?
      SYS_ERR(write(disk, &fb, sizeof(fb)), "write");
   }

   /* Every system call will exit out and return another number,
    * if we reach here, then everything should be successful
    */
   return 0;
}

int tfs_mount(char *filename) {

}

int tfs_unmount() {

}

fileDescriptor tfs_openFile(char *name) {
   //read superblock and find free space for the file
   //insert inode entry into superblock
   //make file pointer point to that part of the disk
   //put that into entry in table thang
   
   return 0;
}

//go through the file extent blocks till the last one and set the next block to 0
int tfs_closeFile(fileDescriptor FD) {
   return 0;
}

//make the indoe block by making header shit and memcpying at loc after shit
int tfs_writeFile(fileDescriptor FD, char *buffer, int size) {
   return 0;
}

int tfs_deleteFile(fileDescriptor FD) {
   return 0;
}

int tfs_readByte(fileDescriptor FD, char *buffer) {
   return 0;
}

int tfs_seek(fileDescriptor FD, int offset) {
   return 0;
}

