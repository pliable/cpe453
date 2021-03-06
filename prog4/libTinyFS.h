#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "libDisk.h"
#include "tinyFS.h"

#define MAGIC 0x45
#define RWBYTE (sizeof(inode) - 1)
/* for SYS_ERR, first parameter is system call, second is string name of system call */
typedef int fileDescriptor;

typedef struct fileinfo fileinfo;

struct fileinfo {
   fileDescriptor fd;
   int fp;/* File pointer */
   char filename[8];
   uint8_t startBlock;
   fileinfo *next;
};

#pragma pack(push)
#pragma pack(1)
/* struct to represent superblock, modify accordingly for TinyFS */
typedef struct {
   uint8_t type;
   uint8_t magic;
   uint8_t byteOffset;
   uint8_t finalByte;
} superblock;

/* struct to represent formatted block */
typedef struct {
   uint8_t type;
   uint8_t magic;
   uint8_t blockAddress;
   uint8_t finalByte;
} formatted_block;

/* struct to represent an inode, modify accordingly for TinyFS */
typedef struct {
   /* header stuff */
   uint8_t type;
   uint8_t magic;
   uint8_t blockAddress;
   uint8_t finalByte;
   /* rest of data */
   char fileName[8];
   uint16_t size;
   /* time_t is 8 bytes */
   time_t createTime;
   time_t accessTime;
   time_t modifyTime;
   uint8_t readWrite;
} inode;
#pragma pack(pop)


fileDescriptor tfs_openFile(char *name);
int shiftShit(uint8_t *bitVectorByte, int *superIndex);
int tfs_mkfs(char *filename, int nBytes);
int tfs_mount(char *filename);
int tfs_unmount(void);
int tfs_closeFile(fileDescriptor FD);
int tfs_writeFile(fileDescriptor FD, char *buffer, int size);
int tfs_deleteFile(fileDescriptor FD);
int tfs_readByte(fileDescriptor FD, char *buffer);
int tfs_seek(fileDescriptor FD, int offset);
int openDisk(char *filename, int nBytes);
int readBlock(int disk, int bNum, void *block);
int writeBlock(int disk, int bNum, void *block);
int tfs_makeRO(char *name);
int tfs_makeRW(char *name);
int tfs_writeByte(fileDescriptor FD, uint8_t data);
time_t tfs_readFileInfo(fileDescriptor FD);
