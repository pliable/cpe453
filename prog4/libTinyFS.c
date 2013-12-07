#include "libTinyFS.h"
#include "libDisk.c"

resource_table *fileTable;

fileDescriptor globalFP = 0;
int fsIsMounted = 0;

int tfs_mkfs(char *filename, int nBytes) {
   int disk, c, offset, numShifts = 0;;
   int numBlocks = nBytes/BLOCKSIZE, numBits = numBlocks - 1;
   /* lengthOfBitVector is number of bytes we need to represent our vector*/
   int lengthOfBitVector = numBlocks / 8;
   superblock sb;
   formatted_block fb;
   char freeBlocksVector = 0;
   char mask = 1;

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
   /* Bit vector solution for free blocks LITTLE ENDIAN SOLUTION */
   while(numBits > 0) {
      /* Shift and OR with 0x00000001 */
      freeBlocksVector = freeBlocksVector << 1;
      freeBlocksVector = freeBlocksVector | mask;
      numShifts++;
      if(numShifts == 8) {
         SYS_ERR(write(disk, &freeBlocksVector, 1), "write");
         /* Zero out the Vector */
         freeBlocksVector = freeBlocksVector & 0;
         numShifts = 0;
      }
      numBits--;
   }
   /* Shift and write the leftovers */
   if(numShifts) {
      freeBlocksVector = freeBlocksVector << (8 - numShifts);
      SYS_ERR(write(disk, &freeBlocksVector, 1), "write");
   }
   
   //format the blocks on the disk

   fb.type = 4;
   fb.magic = MAGIC;
   fb.finalByte = 0;

   for(c = 1; c < numBlocks; c++) {//start at block 1 cos block 0 is the super
      SYS_ERR(lseek(disk, BLOCKSIZE*c, SEEK_SET), "lseek");

      if(c + 1 >= numBlocks) {//we are on the final block, so there is no next
         fb.blockAddress = '\0';
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
   int disk, blockNo = 0;
   char block[256];
   if(fsIsMounted) {
      return -1; /*a file system is already mounted */
   }
   disk = openDisk(filename, 0);
   while(1) {
      readBlock(disk, blockNo, block);
      if(block[1] != MAGIC) { /*magic number check */
         return -1; /*malformed fs */
      } else if(block[2] == '\0') { /* final block checked in the file system */
         break;
      }
      blockNo++;
   }
   fsIsMounted = 1;
   return 0;
}

int tfs_unmount() {



   fsIsMounted = 0;
   return 0;
}

//read superblock and find free space for the file
//insert inode entry into superblock
//make file pointer point to that part of the disk
//put that into entry in table thang
fileDescriptor tfs_openFile(char *name) {
   time_t t;
   inode i;
   int disk;
   uint8_t super[BLOCKSIZE];
   uint8_t addr, bitVectorByte, grandsonOfNasty = 0/* To get the real address of the block */;
   globalFP++;
   char finder = 128, addressPlacer;

   /* open disk */
   disk = openDisk(name, 0);

   /* grab superblock */
   readBlock(disk, 0, &super);

   int superIndex = 4;/* Beginning of the free block list vector index */

   bitVectorByte = super[superIndex];//first byte of the free block list bit vector

   /* grab byte offset, index into super array */
   addr = super[2];

   /* fill out inodes */
   i.type = 2;
   i.magic = MAGIC;
   /* Find a free block in the bit vector */
   while(1) {
      addressPlacer = bitVectorByte & finder;
      grandsonOfNasty++;
      /* Found a free block */
      if(addressPlacer == 128) {
         i.blockAddress = grandsonOfNasty;
         finder = 127; /* 01111111 */
         super[superIndex] = super[superIndex] & finder;
         break;
      }
      bitVectorByte = bitVectorByte << 1;
      if(bitVectorByte == 0) {
         /* Need to go to the next byte to find a block */
         bitVectorByte++;
         superIndex++;
         /* No more blocks free */
         if(bitVectorByte == 0) {
            return -1; /* Disk full */
         }
      }
   }
   i.finalByte = 0;
   strncpy(i.fileName, name, 8);
   i.size = 0;
   time(&t);
   i.createTime = t;
   i.accessTime = t;
   i.modifyTime = t;
   i.readWrite = 1;
   //write inode block to it's proper place
   //make inode block address entry in super block

   //writeblock for the updated super block 

   return globalFP;
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

