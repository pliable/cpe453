#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEFAULT_DISK_NAME "tinyFSDisk"
#define DEFAULT_DISK_SIZE 10240
#define BLOCKSIZE 256
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
   char fileName[8];
   uint32_t size;
} inodes;
#pragma pack(pop)

typedef struct resource_table resource_table;

struct resource_table {
   fileDescriptor fd;
   char buffer[BLOCKSIZE];
   resource_table *next;
};

fileDescriptor tfs_openFile(char *name);
int tfs_closeFile(fileDescriptor FD);
int tfs_writeFile(fileDescriptor FD, char *buffer, int size);
int tfs_deleteFile(fileDescriptor FD);
int tfs_readByte(fileDescriptor FD, char *buffer);
int tfs_seek(fileDescriptor FD, int offset);
