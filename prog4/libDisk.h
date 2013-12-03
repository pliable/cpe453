#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "libTinyFS.h"

#define MAGIC 0x45
#define FIRSTBLOCKOFFSET 0x256
