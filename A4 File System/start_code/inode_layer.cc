#include "inode_layer.h"
#include <unistd.h>


/*
 * Part IA: inode allocation
 */

/* Allocate an inode and return its inum (inode number). */
uint32_t
inode_layer::alloc_inode(uint32_t type)
{
    /* 
     * Your Part I code goes here.
     * hint1: read get_inode() and put_inode(), read INODE_NUM, IBLOCK, etc in inode_layer.h
     * hint2: you can use time(0) to get the timestamp for atime/ctime/mtime
     */

    struct inode *ino;
    uint32_t inum;
    for (uint32_t i = 0; i < INODE_NUM; i++) {
    	if (get_inode(i) == NULL) {
	    inum = i;
            ino = (struct inode*)malloc(sizeof(struct inode));
 	    ino->type = type;
    	    ino->size = 0;
   	    ino->atime = ino->ctime = ino->mtime = time(0);
    	    put_inode(inum, ino);
            return inum;
	}
    }
    /* no free inode left */
    return 0;

    /* Your Part I code ends here. */
}

/* Return an inode structure by inum, NULL otherwise.
 * Caller should release the memory. */
struct inode* 
inode_layer::get_inode(uint32_t inum)
{
    struct inode *ino, *ino_disk;
    char buf[BLOCK_SIZE];

    /* checking parameter */
    printf("\tim: get_inode %d\n", inum);
    if (inum < 0 || inum >= INODE_NUM) {
        printf("\tim: inum out of range\n");
        return NULL;
    }

    /* read the disk block containing the inode data structure */
    bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
    ino_disk = (struct inode*)buf + inum%IPB;

    /* try to read an inode that is not in use */
    if (ino_disk->type == 0) {
        printf("\tim: inode not exist\n");
        return NULL;
    }

    /* return the inode data structure */
    ino = (struct inode*)malloc(sizeof(struct inode));
    *ino = *ino_disk;
    return ino;
}

void
inode_layer::put_inode(uint32_t inum, struct inode *ino)
{
    char buf[BLOCK_SIZE];
    struct inode *ino_disk;

    /* checking parameter */
    printf("\tim: put_inode %d\n", inum);
    if (inum < 0 || inum >= INODE_NUM || ino == NULL) {
        printf("\tim: invalid parameter\n");
        return;
    }

    /* modify the inode data structure on disk */
    bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
    ino_disk = (struct inode*)buf + inum%IPB;
    *ino_disk = *ino;
    bm->write_block(IBLOCK(inum, bm->sb.nblocks), buf);
}


/*
 * Part IB: inode read/write
 */

blockid_t
block_layer::alloc_block()
{
    pthread_mutex_lock(&bitmap_mutex);
    /*
     * Your Part I code goes here.
     * hint1: go through the bitmap region, find a free data block, mark the corresponding bit in the bitmap to 1 and return the block number of the data block.
     * hint2: read free_block(); remember to call pthread_mutex_unlock before all the returns
     */

    uint32_t start = IBLOCK(INODE_NUM, sb.nblocks) + 1;
    char buf[BLOCK_SIZE];

    for (uint32_t id = start; id < sb.nblocks; id++) {
	d->read_block(BBLOCK(id), buf); 
	uint32_t bit_offset_in_block = id % BPB;
	uint32_t byte_offset_in_block = bit_offset_in_block / 8;
	uint32_t bit_offset_in_byte = bit_offset_in_block % 8;
	char* byte = &((char*)buf)[byte_offset_in_block];
	
	if ((*byte & ((char)1 << bit_offset_in_byte)) == 0) {
	    *byte |= (char)1 << bit_offset_in_byte;
	    d->write_block(BBLOCK(id), buf);
	    pthread_mutex_unlock(&bitmap_mutex);
	    return id;
	}
    }

    /* Your Part I code ends here. */
    pthread_mutex_unlock(&bitmap_mutex);
    /* no free data block left */
    return 0;
}

void
block_layer::free_block(uint32_t id)
{
    pthread_mutex_lock(&bitmap_mutex);
    char buf[BLOCK_SIZE];
    d->read_block(BBLOCK(id), buf);

    /* suppose id=5001, we need to modify the 5001 th bit */
    /* which is the 5001 % 4096 = 905 th bit in the block*/
    uint32_t bit_offset_in_block = id % BPB;
    /* which lives in the 905 / 8 = 113 th byte in the block */
    uint32_t byte_offset_in_block = bit_offset_in_block / 8;
    /* and is the 905 % 8 = 1 st bit in this byte */
    uint32_t bit_offset_in_byte = bit_offset_in_block % 8;

    /* You may need to learn the meaning of &= and << operators */
    char* byte = &((char*)buf)[byte_offset_in_block];
    /* (char)1 is (00000001)binary */
    /* (char)1 << 1 is (00000010)binary */
    /* ~((char)1 << 1) is (11111101)binary */
    /* &= makes the bit representing id to zero */
    *byte &= ~((char)1 << bit_offset_in_byte);

    d->write_block(BBLOCK(id), buf);
    pthread_mutex_unlock(&bitmap_mutex);
}


#define MIN(a,b) ((a)<(b) ? (a) : (b))


/* Get all the data of a file by inum. 
 * Return alloced data, should be freed by caller. */
void
inode_layer::read_file(uint32_t inum, char **buf_out, int *size)
{
    rwlocks[inum].reader_enter();
    
    /* check parameter */
    if(buf_out == NULL || size == NULL) {
        rwlocks[inum].reader_exit();
        return;
    }

    /* check existance of inode inum */
    struct inode* ino = get_inode(inum);
    if(ino == NULL) {
        rwlocks[inum].reader_exit();
        return;
    }

    /* modify the access time of inode inum */
    ino->atime = time(NULL);
    put_inode(inum, ino);

    /* prepare the return value */
    *size = ino->size;
    char* buf = (char*)malloc(*size);

    /*
     * Your Part I code goes here.
     * hint1: read all blocks of inode inum into buf_out, including direct and indirect blocks; you will need the memcpy function
     */

    uint32_t num_blocks = (*size % BLOCK_SIZE == 0) ? *size / BLOCK_SIZE : *size / BLOCK_SIZE + 1;
    char tmp[BLOCK_SIZE];
    uint32_t i;

    for (i = 0; i < MIN(num_blocks, NDIRECT); i++) {  // deal with direct blocks first
        bm->read_block(ino->blocks[i], tmp); 
	memcpy(buf + i * BLOCK_SIZE, tmp, MIN(BLOCK_SIZE, *size - i * BLOCK_SIZE));
    }

    if (num_blocks > NDIRECT) {  // still have indirect blocks
        blockid_t *indirect_blocks = (blockid_t *)malloc(sizeof(blockid_t) * NINDIRECT);
	bm->read_block(ino->blocks[NDIRECT], (char *)indirect_blocks);
 	
	while (i < num_blocks) {
	    bm->read_block(indirect_blocks[i-NDIRECT], tmp);
            memcpy(buf + i * BLOCK_SIZE, tmp, MIN(BLOCK_SIZE, *size - i * BLOCK_SIZE));
	    i++;
	}
    }

    /* Your Part I code ends here. */
    *buf_out = buf;
    free(ino);
    rwlocks[inum].reader_exit();
}

void
inode_layer::write_file(uint32_t inum, const char *buf, int size)
{
    rwlocks[inum].writer_enter();

    /* check parameter */
    if(size < 0 || (uint32_t)size > BLOCK_SIZE * MAXFILE){
        printf("\tim: error! size negative or too large.\n");
        rwlocks[inum].writer_exit();
        return;
    }

    struct inode* ino = get_inode(inum);
    if(ino==NULL){
        printf("\tim: error! inode not exist.\n");
        rwlocks[inum].writer_exit();
        return;
    }

    /*
     * Your Part I code goes here.
     * hint1: buf is a buffer containing size blocks, write buf to blocks of inode inum.
     * you need to consider the situation when size
     * is larger or smaller than the size of inode inum
     */

    uint32_t num_before = (ino->size % BLOCK_SIZE == 0) ? ino->size / BLOCK_SIZE : ino->size / BLOCK_SIZE + 1;
    uint32_t num_after = (size % BLOCK_SIZE == 0) ? size / BLOCK_SIZE : size / BLOCK_SIZE + 1;

    blockid_t *indirect = (blockid_t *)malloc(sizeof(blockid_t) * NINDIRECT);
    uint32_t ind = ino->blocks[NDIRECT];
    char tmp[BLOCK_SIZE];
    uint32_t i;

    if (num_before < num_after){ // need to alloc blocks
        if (num_after <= NDIRECT) {
	    for (i = num_before; i < num_after; i++)
                ino->blocks[i] = bm->alloc_block();
	}
        else if (num_before > NDIRECT) {
            bm->read_block(ind, (char *)indirect);
            for (i = num_before; i < num_after; i++)
                indirect[i-NDIRECT] = bm->alloc_block();
            bm->write_block(ind, (char *)indirect);
        }
        else { // (num_before < NDIRECT & num_after > NDIRECT)
            for (i = num_before; i < NDIRECT; i++)
                ino->blocks[i] = bm->alloc_block();
            ino->blocks[NDIRECT] = bm->alloc_block();
	    ind = ino->blocks[NDIRECT];
            for (i = 0; i < num_after - NDIRECT; i++)
                indirect[i] = bm->alloc_block();
            bm->write_block(ind, (char *)indirect);
        }
    }

    else {  // need to free blocks
        if (num_after <= NDIRECT) {
	    for (i = num_after; i < num_before; i++)
                bm->free_block(ino->blocks[i]);
	}
        else if (num_after > NDIRECT) {
	    bm->read_block(ind, (char *)indirect);
            for (i = num_after; i < num_before; i++)
                bm->free_block(indirect[i-NDIRECT]);
        }
        else { // (num_before > NDIRECT & num_after < NDIRECT)
 	    bm->read_block(ind, (char *)indirect);
            for (i = num_after; i < NDIRECT; i++)
                bm->free_block(ino->blocks[i]);
            for (i = 0; i < num_before - NDIRECT; i++)
                bm->free_block(indirect[i]);
        }
    }

    for (i = 0; i < MIN(num_after, NDIRECT); i++) {
        memcpy(tmp, buf + i * BLOCK_SIZE, BLOCK_SIZE);
        bm->write_block(ino->blocks[i], tmp);
    }

    if (num_after > NDIRECT) {
        while (i < num_after) {
            memcpy(tmp, buf + i * BLOCK_SIZE, BLOCK_SIZE);
            bm->write_block(indirect[i-NDIRECT], tmp);
            i++;
        }
    }

    ino->size = size;
    ino->mtime = time(0);
    ino->ctime = time(0);

    /* Your Part I code ends here. */
    put_inode(inum, ino);
    free(ino);
    rwlocks[inum].writer_exit();
}

/*
 * Part IC: inode free/remove
 */

void
inode_layer::free_inode(uint32_t inum)
{
    /* 
     * Your Part I code goes here.
     * hint1: simply mark inode inum as free
     */

    struct inode* ino = get_inode(inum);
    if (ino != NULL) {
	ino->type = 0;
	ino->size = 0;
	put_inode(inum, ino);
    }

    /* Your Part I code ends here. */
}



void
inode_layer::remove_file(uint32_t inum)
{
    struct inode* ino = get_inode(inum);
    if(ino == NULL)
        return;

    /*
     * Your Part I code goes here.
     * hint1: first, free all data blocks of inode inum (use bm->free_block function); second, free the inode
     */

    uint32_t block_num = (ino->size % BLOCK_SIZE == 0) ?  ino->size / BLOCK_SIZE : ino->size / BLOCK_SIZE + 1;
    for (uint32_t i = 0; i < MIN(block_num, NDIRECT); i++)
	bm->free_block(i);
    
    if (block_num > NDIRECT) {
	blockid_t *indirect_blocks = (blockid_t *)malloc(sizeof(blockid_t) * NINDIRECT);
	bm->read_block(ino->blocks[NDIRECT], (char *)indirect_blocks);
	for (uint32_t i = 0; i < block_num - MIN(block_num, NDIRECT); i++)
	    bm->free_block(indirect_blocks[i]);
	indirect_blocks = NULL;
    }

    free_inode(inum);

    /* Your Part I code ends here. */
    free(ino);
    return;
}



/*
 * Helper Functions
 */

/* inode layer ---------------------------------------- */

inode_layer::inode_layer()
{
    bm = new block_layer();
    uint32_t root_dir = alloc_inode(fs_protocol::T_DIR);
    if (root_dir != 0) {
        printf("\tim: error! alloc first inode %d, should be 0\n", root_dir);
        exit(0);
    }
}

void
inode_layer::getattr(uint32_t inum, fs_protocol::attr &a)
{
    struct inode* ino = get_inode(inum);
    if(ino == NULL)
        return;
    a.type = ino->type;
    a.size = ino->size;
    a.atime = ino->atime;
    a.mtime = ino->mtime;
    a.ctime = ino->ctime;
    free(ino);
}


/* block layer ---------------------------------------- */

// The layout of disk should be like this:
// |<-sb->|<-free block bitmap->|<-inode table->|<-data->|
block_layer::block_layer()
{
    d = new disk();

    // format the disk
    sb.size = BLOCK_SIZE * BLOCK_NUM;
    sb.nblocks = BLOCK_NUM;
    sb.ninodes = INODE_NUM;

}

void
block_layer::read_block(uint32_t id, char *buf)
{
    d->read_block(id, buf);
}

void
block_layer::write_block(uint32_t id, const char *buf)
{
    d->write_block(id, buf);
}


/* disk layer ---------------------------------------- */

disk::disk()
{
  bzero(blocks, sizeof(blocks));
}

void
disk::read_block(blockid_t id, char *buf)
{
    if(id < 0 || id > BLOCK_NUM || buf == NULL)
        return;
    memcpy(buf, blocks[id], BLOCK_SIZE);

    if (DISK_ACCESS_LATENCY)
        usleep(DISK_ACCESS_LATENCY);
}

void
disk::write_block(blockid_t id, const char *buf)
{
    if(id < 0 || id > BLOCK_NUM || buf == NULL)
        return;
    memcpy(blocks[id], buf, BLOCK_SIZE);

    if (DISK_ACCESS_LATENCY)
        usleep(DISK_ACCESS_LATENCY);
}
