#include "libTinyFS.h"
#include "libDisk.c"
#include "TinyFS_errno.h"

fileinfo *resourceTable = NULL;

fileDescriptor globalFD = 0;
int fsIsMounted = 0;
char *currentFSMounted;

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
      return ALREADYMOUNTEDERR; /*a file system is already mounted */
   }
   disk = openDisk(filename, 0);
   while(1) {
      readBlock(disk, blockNo, block);
      if(block[1] != MAGIC) { /*magic number check */
         return MALFORMEDFS; /*malformed fs */
      } else if(block[2] == '\0') { /* final block checked in the file system */
         break;
      }
      blockNo++;
   }
   fsIsMounted = 1;
   currentFSMounted = (char *)malloc(strlen(filename));
   strcpy(currentFSMounted, filename);
   return 0;
}

int tfs_unmount() {
   fileDescriptor toClose;

   if(!fsIsMounted) {
      return NOFSMOUNTED; //no fs mounted error
   }

   /* closing all files */
   while(resourceTable != NULL) {
      toClose = resourceTable->fd;
      tfs_closeFile(toClose);
   }

   free(currentFSMounted);
   fsIsMounted = 0;
   return 0;
}

int shiftShit(uint8_t *bitVectorByte, int *superIndex) {
   *bitVectorByte = *bitVectorByte << 1;
   if(*bitVectorByte == 0) {
      /* Need to go to the next byte to find a block */
      *bitVectorByte++;
      *superIndex++;
      /* No more blocks free */
      if(*bitVectorByte == 0) {
         return NOMOREBITS; /* went through all the bits */
      }
   }

   return 1;
}

//read superblock and find free space for the file
//insert inode entry into superblock
//make file pointer point to that part of the disk
//put that into entry in table thang
fileDescriptor tfs_openFile(char *name) {
   time_t t;
   int h;
   inode i;
   int disk;
   uint8_t find;
   uint8_t super[BLOCKSIZE];
   uint8_t inodeReader[BLOCKSIZE];
   uint8_t addr, bitVectorByte, grandsonOfNasty = 0/* To get the real address of the block */;
   unsigned char finder = 128, addressPlacer;
   if(!fsIsMounted) {
      return NOFSMOUNTED; //no fs mounted
   }
   globalFD++;

   /* open disk */
   disk = openDisk(currentFSMounted, 0);

   /* grab superblock */
   readBlock(disk, 0, &super);

   int superIndex = 4, a;/* Beginning of the free block list vector index */

   bitVectorByte = super[superIndex];//first byte of the free block list bit vector

   /* grab byte offset, index into super array */
   addr = super[2];

   /* pointer to current node in resource table */
   fileinfo *currFileInfo;

   /* Go through file table shit to create file entry or see if entry exists */
   if(resourceTable == NULL) { //List is empty
      resourceTable = malloc(sizeof(fileinfo));
      resourceTable->fd = globalFD;
      resourceTable->fp = 0;
      strcpy(resourceTable->filename, name);
      resourceTable->next = 0;
      currFileInfo = resourceTable;
   } else {
      currFileInfo = resourceTable;
      while(currFileInfo->next != 0) {
         if(!strcmp(currFileInfo->filename, name)) {
            return FILEALREADYOPEN; //file already open error
         }
         /* iterating through list */
         currFileInfo = currFileInfo->next;
      }

      currFileInfo->next = malloc(sizeof(fileinfo));
      currFileInfo = currFileInfo->next;

      if(currFileInfo == 0) {
         return OUTOFMEMORY; //out of memory error
      }

      currFileInfo->fd = globalFD;
      currFileInfo->fp = 0;
      strcpy(currFileInfo->filename, name);
      currFileInfo->next = 0;
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
         if(inodeReader[0] != 2) { /* check to see if it is a valid inode block */
            if((a = shiftShit(&bitVectorByte, &superIndex)) < 0) {
               break;  
             }
            else {
               continue;
            }
         }
         char fileNameChecker[8];
         memcpy(&fileNameChecker, &inodeReader[sizeof(formatted_block)], 8); /* Copy the filename so we can check it */
         if(!strcmp(fileNameChecker, name)) {
            memcpy(&inodeReader[22], &t, 8); /* Update access time if filenames match */
            writeBlock(disk, grandsonOfNasty, &inodeReader);
            return globalFD;
         }
      }
      if((a = shiftShit(&bitVectorByte, &superIndex)) < 0) {
         break;
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
         /* Zero out the bit that is used for that address */
         int gson = 0;
         finder = 128; /* 10000000 */
         while(gson < (8-(grandsonOfNasty%8))) {
            finder = finder >> 1;
            finder = finder | 128;
            gson++;
         }
         finder = finder >> 1;/* Place the zero in */
         gson++;

         while(gson < 8) {
            finder = finder >> 1;
            finder = finder | 128;
            gson++;
         }
         /* End zeroing that specific bit out */
         super[superIndex] = super[superIndex] & finder;
         currFileInfo->startBlock = grandsonOfNasty;
         break;
      }
      if((a = shiftShit(&bitVectorByte, &superIndex)) < 0) {
         return DISKFULL;//disk full
      }
   }
   i.finalByte = 0;
   strncpy(i.fileName, name, 8);
   i.size = 0;
   i.readWrite = 1;
   for(h = 0; h < BLOCKSIZE; h++) {
      inodeReader[h] = 0;
   }
   memcpy(inodeReader, &i, sizeof(inode));
   //write inode block to it's proper place
   writeBlock(disk, i.blockAddress, &inodeReader);
   //make inode block address entry in super block
   find = super[2];
   if(find + i.blockAddress < 252) {
      super[find + i.blockAddress] = i.blockAddress;
   } else {
      //disk is full for -1 case
      return DISKFULL;
   }

   //writeblock for the updated super block 
   writeBlock(disk, 0, &super);


   //filetable entry

   return globalFD;
}

//go through the file extent blocks till the last one and set the next block to 0
int tfs_closeFile(fileDescriptor FD) {
   fileinfo *currFileInfo;
   fileinfo *prevFileInfo;

   if(!fsIsMounted) {
      return NOFSMOUNTED;//no fs mounted error
   }

   currFileInfo = resourceTable;

   if(resourceTable->fd == FD) {
      resourceTable = resourceTable->next;
      return 0;
   }
   
   prevFileInfo = currFileInfo;
   currFileInfo = currFileInfo->next;

   while(currFileInfo != NULL) {
      if(currFileInfo->fd == FD) {
         prevFileInfo->next = currFileInfo->next;
         return 0;
      }
      prevFileInfo = currFileInfo;
      currFileInfo = currFileInfo->next;
   }

   /* return error here since FD wasn't found */
   return FILENOTOPEN;
}

//make the indoe block by making header shit and memcpying at loc after shit
int tfs_writeFile(fileDescriptor FD, char *buffer, int size) {
   int currDisk, offset = sizeof(inode), superIndex = 4, a;
   uint8_t blockBuff[BLOCKSIZE], currBlockToRead, super[BLOCKSIZE],
            grandsonOfNasty = 0, bitVectorByte;
   uint16_t sizeConvert = size;
   fileinfo *currFileInfo;
   currFileInfo = resourceTable;
   unsigned char addressPlacer, finder = 128;
   time_t fileFucker;

   if(!fsIsMounted) {
      return NOFSMOUNTED; //no fs mounted
   }

   currDisk = openDisk(currentFSMounted, 0);
   readBlock(currDisk, 0, &super);
   bitVectorByte = super[superIndex];


   time(&fileFucker);
   while(currFileInfo != NULL) {
      if(currFileInfo->fd == FD) {
         currBlockToRead = currFileInfo->startBlock;
         readBlock(currDisk, currBlockToRead, &blockBuff);
         /* Check for readonly */
         if(blockBuff[RWBYTE] == 0) {
            return READONLYERR;//file is read only error
         }
         blockBuff[22] = fileFucker;
         blockBuff[30] = fileFucker;
         writeBlock(currDisk, currBlockToRead, &blockBuff);
         currFileInfo->fp = 0;
         if(size > blockBuff[12]) {//if the size passed in is larger than the old size of the file
            blockBuff[12] = sizeConvert;/* Update the size of the file */
         }
         while(1) {
            readBlock(currDisk, currBlockToRead, blockBuff);
            if(blockBuff[0] == 4) {
               blockBuff[0] = 3;
            }
            /* Finish buffer case */
            if(size == (BLOCKSIZE - offset)) {
               memcpy(&blockBuff[offset], buffer, BLOCKSIZE-offset);
               blockBuff[2] = 0;
               writeBlock(currDisk, currBlockToRead, blockBuff);
               break;
            }
            else if(size < (BLOCKSIZE - offset)) {
               memcpy(&blockBuff[offset], buffer, size);
               blockBuff[2] = 0;
               writeBlock(currDisk, currBlockToRead, blockBuff);
               break;
            }
            /* Finish block case */
            else if(size > (BLOCKSIZE - offset)) {
               /* Write out inode block */
               memcpy(&blockBuff[offset], buffer, BLOCKSIZE-offset);
               size = size - (BLOCKSIZE - offset); /* Decrement size by the amount we wrote */
               buffer = buffer + (BLOCKSIZE - offset);//move the buffer over after writing
               offset = sizeof(formatted_block);
               /* Find free block in the bit vector */
               while(1) {
                  addressPlacer = bitVectorByte & finder;
                  grandsonOfNasty++;
                  /* Found a free block */
                  if(addressPlacer == 128) { /* will break if bit vector is not in order (ex: 1001...) */
                     /* Zero out the bit that is used for that address */
                     int gson = 0;
                     unsigned char finder = 128; /* 10000000 */
                     while(gson < (8-(grandsonOfNasty%8))) {
                        finder = finder >> 1;
                        finder = finder | 128;
                        gson++;
                     }
                     finder = finder >> 1;/* Place the zero in */
                     gson++;
                     while(gson < 8) {
                        finder = finder >> 1;
                        finder = finder | 128;
                        gson++;
                     }
                     /* End zeroing that specific bit out */
                     super[superIndex] = super[superIndex] & finder;
                     break;
                  }
                  if((a = shiftShit(&bitVectorByte, &superIndex)) < 0) {
                     return DISKFULL; //disk full error
                  }
               }
               /* Goto bit vector and find a free block
                * put free block# into current block read (red) from disk
                * modify header to be a file extent 
                */
               blockBuff[2] = grandsonOfNasty;/* The next free block we can write to */
               writeBlock(currDisk, currBlockToRead, blockBuff);
               currBlockToRead = blockBuff[2];/* next block in the file */
            }
         }
         break;
      }
      currFileInfo = currFileInfo->next;
      if(currFileInfo == NULL) {
         return FILENOTOPEN; //file not found error
      }
   }
   writeBlock(currDisk, 0, &super);

   return 0;
}

int tfs_deleteFile(fileDescriptor FD) {
   int y, disk, grandHoOfNasty = 0;
   formatted_block f;
   fileinfo *currFileInfo;
   uint8_t blockDestroyer[BLOCKSIZE], boofer[BLOCKSIZE], super[BLOCKSIZE], currBlock;
   uint8_t whichByte, whichBit, byteSelector;

   if(!fsIsMounted) {
      return NOFSMOUNTED;//no fs mounted
   }

   for(y = 0 ; y < BLOCKSIZE ; y++) {
      blockDestroyer[y] = '\0';
   }
   disk = openDisk(currentFSMounted, 0);

   readBlock(disk, 0, &super);
   currFileInfo = resourceTable;
   f.type = 4;
   f.magic = MAGIC;

   while(currFileInfo != NULL) {
      if(currFileInfo->fd == FD) {
         //we found our file!
         currBlock = currFileInfo->startBlock;
         while(1) {
            readBlock(disk, currBlock, &boofer);
            f.blockAddress = boofer[2];
            memcpy(&boofer, &blockDestroyer, BLOCKSIZE);
            memcpy(&boofer, &f, sizeof(formatted_block));
            writeBlock(disk, currBlock, &boofer);

            /* update superblock */
            whichByte = currBlock / 8;
            whichBit = currBlock % 8;
            printf("%u  %u\n", whichByte, whichBit);
            byteSelector = super[sizeof(formatted_block) + whichByte];
            unsigned char finder = 128;
            for(y = 1; y < whichBit; y++) {
               finder = finder >> 1;
            }
            byteSelector = byteSelector | finder;
            super[sizeof(formatted_block) + whichByte] = byteSelector;
            /* End update */
            currBlock = f.blockAddress;
            if(currBlock == 0) {
               grandHoOfNasty = 1;
               break;
            }
         }
      }
      currFileInfo = currFileInfo->next;
   }

   if(currFileInfo == NULL && !grandHoOfNasty) {
      //file not found
      return FILENOTOPEN;
   }
   writeBlock(disk, 0, &super);//commit super block changes to disk

   tfs_closeFile(FD);
   return 0;
}

int tfs_readByte(fileDescriptor FD, char *extBuffer) {
   fileinfo *currFileInfo;
   int fileptr, disk;
   unsigned int offset = 0;
   uint8_t blockToRead;
   char buffer[BLOCKSIZE];
   time_t axecess;

   if(!fsIsMounted) {
      return NOFSMOUNTED;//no fs mounted
   }

   currFileInfo = resourceTable;

   while(currFileInfo != NULL) {
      if(currFileInfo->fd == FD) {
         //we found our file!
         fileptr = currFileInfo->fp;
         break;
      }
      currFileInfo = currFileInfo->next;
   }

   if(currFileInfo == NULL) {
      //file not found
      return FILENOTOPEN;
   }

   disk = openDisk(currentFSMounted, 0);

   blockToRead = currFileInfo->startBlock;
   time(&axecess);
   readBlock(disk, blockToRead, &buffer);
   buffer[22] = axecess;
   writeBlock(disk, blockToRead, &buffer);
   

   offset = currFileInfo->fp;

   if(offset == 0) {
      readBlock(disk, blockToRead, buffer);
      *extBuffer = buffer[sizeof(inode)];
      currFileInfo->fp++;
      return 0;
   }
   while(offset > 0) {
      readBlock(disk, blockToRead, buffer);
      if(buffer[0] == 2) {/* inode block */
         if(buffer[2] == 0) { /* last block */
            if(offset < (BLOCKSIZE - sizeof(inode))){/* no overflow */
               *extBuffer = buffer[offset+sizeof(inode)];
               break;
            }
            else {
               return OVERFLOW; //read too far error
            }
         }
         else {/* Not the last block */
            if(offset < (BLOCKSIZE - sizeof(inode))){/* no overflow */
               *extBuffer = buffer[offset+sizeof(inode)];
               break;
            }
            else {
               offset = offset - (BLOCKSIZE - sizeof(inode));
            }
         }
      }
      else if(buffer[0] == 3) {/* extent block */
         if(buffer[2] == 0) { /* last block */
            if(offset < (BLOCKSIZE - sizeof(formatted_block))){/* no overflow */
               *extBuffer = buffer[offset+sizeof(sizeof(formatted_block))];
               break;
            }
            else {
               return OVERFLOW; //read too far error
            }
            //final block
         }
         else {/* Not the last block */
            if(offset < (BLOCKSIZE - sizeof(formatted_block))){/* no overflow */
               *extBuffer = buffer[offset+sizeof(formatted_block)];
               break;
            }
            else {
               offset = offset - (BLOCKSIZE - sizeof(formatted_block));
            }
         }
      }
      blockToRead = buffer[2];//grab the next block
   }

   currFileInfo->fp++;
   return 0;
}

int tfs_seek(fileDescriptor FD, int offset) {
   fileinfo *currFileInfo;
   uint8_t doofer[BLOCKSIZE];
   int disk;
   time_t axess;

   if(!fsIsMounted) {
      return NOFSMOUNTED;//no fs mounted
   }

   currFileInfo = resourceTable;
   disk = openDisk(currentFSMounted, 0);
   time(&axess);

   while(currFileInfo != NULL) {
      if(currFileInfo->fd == FD) {
         currFileInfo->fp = offset;
         readBlock(disk, currFileInfo->startBlock, &doofer);
         doofer[22] = axess;
         writeBlock(disk, currFileInfo->startBlock, &doofer);
         return 0;
      }
      currFileInfo = currFileInfo->next;
   }

   return FILENOTOPEN;//no FD was found
}

time_t tfs_readFileInfo(fileDescriptor FD) {
   fileinfo *currFileInfo;
   int disk;
   uint8_t buffer[BLOCKSIZE];
   time_t theTime;

   currFileInfo = resourceTable;
   disk = openDisk(currentFSMounted, 0);

   while(currFileInfo != NULL) {
      if(currFileInfo->fd == FD) {
         readBlock(disk, currFileInfo->startBlock, &buffer);
         memcpy(&theTime, &buffer[14], 8);
         return theTime;
      }
      currFileInfo = currFileInfo->next;
   }

   //no file found error
   return FILENOTOPEN;
}

int tfs_makeRO(char *name) {
   fileinfo *currFileInfo;
   uint8_t doofer[BLOCKSIZE];
   int disk;
   time_t axess;

   currFileInfo = resourceTable;
   disk = openDisk(currentFSMounted, 0);
   time(&axess);

   while(currFileInfo != NULL) {
      if(!strcmp(currFileInfo->filename, name)) {
         //fuond the file
         readBlock(disk, currFileInfo->startBlock, &doofer);
         doofer[RWBYTE] = 0;//set file to readOnly
         doofer[22] = axess;//mod acccess time
         doofer[30] = axess;//mod modified tame
         writeBlock(disk, currFileInfo->startBlock, &doofer);

         return 0;
      }
      currFileInfo = currFileInfo->next;
   }

   return 0;
}

int tfs_makeRW(char *name) {
   fileinfo *currFileInfo;
   uint8_t doofer[BLOCKSIZE];
   int disk;
   time_t axess;

   currFileInfo = resourceTable;
   disk = openDisk(currentFSMounted, 0);
   time(&axess);

   while(currFileInfo != NULL) {
      if(!strcmp(currFileInfo->filename, name)) {
         //fuond the file
         readBlock(disk, currFileInfo->startBlock, &doofer);
         doofer[RWBYTE] = 1;//set file to readOnly
         doofer[22] = axess;//mod acccess time
         doofer[30] = axess;//mod modified tame
         writeBlock(disk, currFileInfo->startBlock, &doofer);

         return 0;
      }
      currFileInfo = currFileInfo->next;
   }

   return 0;
}

int tfs_writeByte(fileDescriptor FD, uint8_t data) {
   fileinfo *currFileInfo;
   int fileptr, disk;
   unsigned int offset = 0;
   uint8_t blockToRead;
   char buffer[BLOCKSIZE];
   time_t axecess;

   if(!fsIsMounted) {
      return NOFSMOUNTED;//no fs mounted
   }

   currFileInfo = resourceTable;

   while(currFileInfo != NULL) {
      if(currFileInfo->fd == FD) {
         //we found our file!
         fileptr = currFileInfo->fp;
         break;
      }
      currFileInfo = currFileInfo->next;
   }

   if(currFileInfo == NULL) {
      //file not found
      return FILENOTOPEN;
   }

   disk = openDisk(currentFSMounted, 0);

   blockToRead = currFileInfo->startBlock;
   time(&axecess);
   readBlock(disk, blockToRead, &buffer);
   buffer[22] = axecess;//change access tiem
   buffer[30] = axecess;//change modded time
   writeBlock(disk, blockToRead, &buffer);
   

   offset = currFileInfo->fp;

   if(offset == 0) {
      readBlock(disk, blockToRead, buffer);
      buffer[sizeof(inode)] = data;
      currFileInfo->fp++;
      writeBlock(disk, blockToRead, &buffer);
      return 0;
   }
   while(offset > 0) {
      readBlock(disk, blockToRead, buffer);
      if(buffer[0] == 2) {/* inode block */
         if(buffer[2] == 0) { /* last block */
            if(offset < (BLOCKSIZE - sizeof(inode))){/* no overflow */
               buffer[offset+sizeof(inode)] = data;
               break;
            }
            else {
               return OVERFLOW; //read too far error
            }
         }
         else {/* Not the last block */
            if(offset < (BLOCKSIZE - sizeof(inode))){/* no overflow */
               buffer[offset+sizeof(inode)] = data;
               break;
            }
            else {
               offset = offset - (BLOCKSIZE - sizeof(inode));
            }
         }
      }
      else if(buffer[0] == 3) {/* extent block */
         if(buffer[2] == 0) { /* last block */
            if(offset < (BLOCKSIZE - sizeof(formatted_block))){/* no overflow */
               buffer[offset+sizeof(formatted_block)] = data;
               break;
            }
            else {
               return OVERFLOW; //read too far error
            }
            //final block
         }
         else {/* Not the last block */
            if(offset < (BLOCKSIZE - sizeof(formatted_block))){/* no overflow */
               buffer[offset+sizeof(formatted_block)] = data;
               break;
            }
            else {
               offset = offset - (BLOCKSIZE - sizeof(formatted_block));
            }
         }
      }
      blockToRead = buffer[2];//grab the next block
   }
   writeBlock(disk, blockToRead, &buffer);

   currFileInfo->fp++;
   return 0;
}
