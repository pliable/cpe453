#include <stdint.h>

/* struct to represent superblock, modify accordingly for TinyFS */
typedef struct {
   uint32_t ninodes;
   uint16_t pad1;
   int16_t i_blocks;
   int16_t z_blocks;
   uint16_t firstdata;
   int16_t log_zone_size;
   int16_t pad2;
   uint32_t max_file;
   uint32_t zones;
   int16_t magic;
   int16_t pad3;
   uint16_t blocksize;
   uint8_t subversion;
} superblock;

/* struct to represent an inode, modify accordingly for TinyFS */
typedef struct {
   uint16_t mode;
   uint16_t links;
   uint16_t uid;
   uint16_t gid;
   uint32_t size;
   int32_t atime;
   int32_t mtime;
   int32_t ctime;
   /* will we be using indirect zones for data? or another method... */
   uint32_t zone[DIRECT_ZONES];
   uint32_t indirect;
   uint32_t two_indirect;
   uint32_t unused;
} inodes;

/* directory entry */
typedef struct {
   uint32_t inode;
   unsigned char name[60];
} dentry;

/* permissions */
typedef struct {
   int mask;
   char indicator;
} perms;
