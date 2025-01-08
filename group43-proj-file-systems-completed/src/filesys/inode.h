#ifndef FILESYS_INODE_H
#define FILESYS_INODE_H

#include <stdbool.h>
#include "filesys/off_t.h"
#include "devices/block.h"

struct inode_disk;
struct bitmap;

void inode_init(void);
bool inode_create(block_sector_t, off_t);
struct inode* inode_open(block_sector_t);
struct inode* inode_reopen(struct inode*);
block_sector_t inode_get_inumber(const struct inode*);
void inode_close(struct inode*);
void inode_remove(struct inode*);
off_t inode_read_at(struct inode*, void*, off_t size, off_t offset);
off_t inode_write_at(struct inode*, const void*, off_t size, off_t offset);
void inode_deny_write(struct inode*);
void inode_allow_write(struct inode*);
off_t inode_length(const struct inode*);
bool inode_resize(struct inode* inode, off_t new_size);
void free_blocks(struct inode_disk* disk_inode, size_t blocks_to_free);
bool allocate_blocks(struct inode_disk* disk_inode, size_t blocks_to_allocate);
bool inode_create_dir(block_sector_t sector, off_t length);
bool is_dir(const struct inode* inode);
bool inode_is_removed(const struct inode* inode);

#endif /* filesys/inode.h */
