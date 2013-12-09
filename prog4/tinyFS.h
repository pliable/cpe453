#ifndef TINYFS_H
#define TINYFS_H
#define DEFAULT_DISK_NAME "tinyFSDisk"
#define DEFAULT_DISK_SIZE 10240
#define BLOCKSIZE 256
#define SYS_ERR(eno, sys_call) { if(eno < 0) { perror(sys_call); exit(EXIT_FAILURE); } }
#endif
