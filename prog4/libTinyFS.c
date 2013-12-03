#include "libTinyFS.h"

fileDescriptor tfs_openFile(char *name) {
   //read superblock and find free space for the file
   //make file pointer point to that part of the disk
   //put that into entry in table thang
   return 0;
}

int tfs_closeFile(fileDescriptor FD) {
   return 0;
}

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

