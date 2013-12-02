#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BLOCKSIZE 256
#define DISKNAME "TinyFSDisk"
