#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAGIC 0x45
#define DEFAULT_DISK_NAME "tinyFSDisk"
#define DEFAULT_DISK_SIZE 10240
#define BLOCKSIZE 256
/* for SYS_ERR, first parameter is system call, second is string name of system call */
#define SYS_ERR(eno, sys_call) { if(eno < 0) { perror(sys_call); exit(EXIT_FAILURE); } }
typedef int fileDescriptor;
typedef struct f{
   fileDescriptor fp;
   char *filename;
   struct f *next;
} fileinfo;

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

typedef struct resource_table resource_table;

struct resource_table {
   fileDescriptor fd;
   char buffer[BLOCKSIZE];
   resource_table *next;
};

fileDescriptor tfs_openFile(char *name);
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
