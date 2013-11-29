#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEFAULT_DISK_NAME "tinyFSDisk"
#define DEFAULT_DISK_SIZE 10240
#define BLOCKSIZE 256
#typedef int fileDescriptor;

int tfs_mkfs(char *filename, int nBytes);
int tfs_mount(char *filname);
int tfs_unmount(void);
fileDescriptor tfs_openFile(char *name);
int tfs_closeFile(fileDescriptor FD);
int tfs_writeFile(fileDescriptor FD, char *buffer, int size);
int tfs_deleteFile(fileDescriptor FD);
int tfs_readByte(fileDescriptor FD, char *buffer);
int tfs_seek(fileDescriptor FD, int offset);
