#ifndef inode_h
#define inode_h

#include <stdint.h>
#include "fs_protocol.h" 
#include "locks.h"

#define DISK_SIZE  1024*1024*16
#define BLOCK_SIZE 512
#define BLOCK_NUM  (DISK_SIZE/BLOCK_SIZE)

typedef uint32_t blockid_t;
extern int DISK_ACCESS_LATENCY;


// disk layer -----------------------------------------

class disk {
 private:
    unsigned char blocks[BLOCK_NUM][BLOCK_SIZE];

 public:
    disk();
    void read_block(uint32_t id, char *buf);
    void write_block(uint32_t id, const char *buf);
};

// block layer -----------------------------------------

typedef struct superblock {
    uint32_t size;
    uint32_t nblocks;
    uint32_t ninodes;
} superblock_t;


class block_layer {
 private:
    disk *d;
    pthread_mutex_t bitmap_mutex;
 public:
    block_layer();
    struct superblock sb;

    uint32_t alloc_block();
    void free_block(uint32_t id);
    void read_block(uint32_t id, char *buf);
    void write_block(uint32_t id, const char *buf);
};

// inode layer -----------------------------------------

// Block containing inode i
#define IBLOCK(i, nblocks)     ((nblocks)/BPB + (i)/IPB + 1)

// Bitmap bits per block
#define BPB           (BLOCK_SIZE*8)

// Block containing bit for block b
#define BBLOCK(b) ((b)/BPB + 1)

// Total number of inodes
#define INODE_NUM  1024

// Inodes per block.
#define IPB           (BLOCK_SIZE / sizeof(struct inode))

// Direct blocks per inode
#define NDIRECT 8

// Indirect blocks per inode
#define NINDIRECT (BLOCK_SIZE / sizeof(uint))

// Maximum blocks per inode
#define MAXFILE (NDIRECT + NINDIRECT)

typedef struct inode {           
    short type;                  // allocated/free, file/directory, etc.
    unsigned int size;           // size of inode
    unsigned int atime;          // access time
    unsigned int mtime;          // modification time
    unsigned int ctime;          // create time
    blockid_t blocks[NDIRECT+1]; // data block addresses
} inode_t;


class inode_layer {
 private:
    block_layer *bm;
    struct inode* get_inode(uint32_t inum);
    void put_inode(uint32_t inum, struct inode *ino);
    a4_rwlock rwlocks[INODE_NUM];

 public:
    inode_layer();
    uint32_t alloc_inode(uint32_t type);
    void free_inode(uint32_t inum);
    void read_file(uint32_t inum, char **buf, int *size);
    void write_file(uint32_t inum, const char *buf, int size);
    void remove_file(uint32_t inum);
    void getattr(uint32_t inum, fs_protocol::attr &a);
};

#endif
