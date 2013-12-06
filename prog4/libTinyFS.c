#include "libTinyFS.h"

fileDescriptor globalFP = 0;
struct node head;

int tfs_mkfs(char *filename, int nBytes) {

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

