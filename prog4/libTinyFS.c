#include "libTinyFS.h"
#include "libDisk.c"

resource_table *fileTable;
fileinfo *files;

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
   uint8_t inodeReader[BLOCKSIZE];
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

   /* Go through file table shit to create file entry or see if entry exists */
   fileinfo *f = files;
   if(files == NULL) {
      f = (fileinfo *)malloc(sizeof(fileinfo));
      f->fd = globalFP;
      fp = 0; /* Beginning of the file */
      strcpy(f->filename, i.fileName)
   }
   else {
      if(strcmp(f->filename, name)) {//just checks the head noce
         return -1;//file already exists error
      }
      while(f=f->next) {//to goto the end of the list
         //check to see if this file entry exists already
         if(strcmp(f->filename, name)) {//just checks the head noce
            return -1;//file already exists error
         }
      }
      f->next = (fileinfo *)malloc(sizeof(fileinfo));
      f->fd = globalFP;
      fp = 0; /* Beginning of the file */
      strcpy(f->filename, i.fileName)
   }
   resource_table *t = filetable;
   if(t == NULL) {/* First entry into the table */
      t = (resource_table *)malloc(sizeof(resource_table));
      t->fd = globalFP;
      t->buffer = (char *)malloc(BLOCKSIZE - sizeof(inode));
   }
   else {
      while(t=t->next);
      t->next = (resource_table *)malloc(sizeof(resource_table));
      t->fd = globalFP;
      t->buffer = (char *)malloc(BLOCKSIZE - sizeof(inode));
   }


   /* fill out inodes */
   time(&t);
   i.createTime = t;
   i.accessTime = t;
   i.modifyTime = t;
   i.type = 2;
   i.magic = MAGIC;

   /* Need to check through all the inodes FIRST to see if file exists
    * THEN we can look for a free block to stick this file */
   while(1) {
      addressPlacer = bitVectorByte & finder;
      grandsonOfNasty++;
      /* Found a used block */
      if(addressPlacer == 0) { 
         readBlock(disk, grandsonOfNasty, &inodeReader);
         if(inodeReader[2] != 2) { /* check to see if it is a valid inode block */
            continue;
         }
         char fileNameChecker[8];
         strncpy(fileNameChecker, inodeReader[sizeof(formatted_block)], 8); /* Copy the filename so we can check it */
         if(strcmp(fineNameChecker, name)) { /* name matches */
            memcpy(inodeReader[21], t, 8); /* Update access time if filenames match */
            writeblock(disk, grandsonOfNasty, &inodeReader);
            return globalFP;
         }
      }
      bitVectorByte = bitVectorByte << 1;
      if(bitVectorByte == 0) {
         /* Need to go to the next byte to find a block */
         bitVectorByte++;
         superIndex++;
         /* No more blocks free */
         if(bitVectorByte == 0) {
            break; /* went through all the bits */
         }
      }
   }  

   /* Reset all values and rerun */
   grandsonOfNasty = 0;
   superIndex = 4;
   bitVectorByte = super[superIndex];//first byte of the free block list bit vector

   /* File was not found on the disk, so find a free block in the bit vector */
   while(1) {
      addressPlacer = bitVectorByte & finder;
      grandsonOfNasty++;
      /* Found a free block */
      if(addressPlacer == 128) { /* will break if bit vector is not in order (ex: 1001...) */
         i.blockAddress = grandsonOfNasty;
         /* Zero out the bit that is sued for that address */
         int gson = 0;
         finder = 128; /* 10000000 */
         while(gson < (8-(grandsonOfNasty%8))) {
            finder = finder >> 1;
            finder = finder | 128;
            gson++;
         }
         finder = finder >> 1;/* Place the zero in */
         gson++
         while(gson < 8) {
            finder = finder >> 1;
            finder = finder | 128;
            gson++;
         }
         /* End zeroing that specific bit out */
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
   i.readWrite = 1;
   //write inode block to it's proper place
   writeBlock(disk, i.blockAddress, &i)
   //make inode block address entry in super block
   uint8_t find = super[2];
   find + i.blockAddress < 252 ? super[find + i.blockAddress] = i.blockAddress : return -1; //disk is full for -1 case
   /*while(1) {
      if(super[find] == '\0') {
         super[find] = i.blockAddress;
         break;
      }
      find++;
      //case for if we go into another block for inode entries
      if(find >= 252) {
         return -1; //disk is full b/c no more inode entries can be put in
      }
   }*/

   //writeblock for the updated super block 
   writeBlock(disk, super, 0);


   //filetable entry

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

