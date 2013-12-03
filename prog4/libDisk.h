#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BLOCKSIZE 256
#define MAGIC 0x45
#define FIRSTBLOCKOFFSET 0x256
#define DISKNAME "TinyFSDisk"

int openDisk(char *filename, int nBytes);
int readBlock(int disk, int bNum, void *block);
int writeBlock(int disk, int bNum, void *block);
