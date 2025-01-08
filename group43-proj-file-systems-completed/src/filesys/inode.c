#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include <stdbool.h>

#define DIRECT_BLOCKS 12
#define INDIRECT_BLOCKS 128
#define CACHE_SIZE 64
struct cached_block {
  block_sector_t sector;
  int use;
  int dirty;
  int valid;
  struct lock lock; // only one thread can read / write to block at a time
  uint8_t* data;
};
struct buffer_cache {
  struct cached_block* cached_blocks[CACHE_SIZE];
  int clock_hand;
  struct lock lock; // only one thread can change the buffer cache's state
};
static struct buffer_cache* buffer_cache;

struct cached_block* clock_find_victim() {
  struct cached_block* cb;
  // don't need to run clock; already found empty space
  for (int i = 0; i < CACHE_SIZE; i++) {
    cb = buffer_cache->cached_blocks[i];
    if (cb->valid == 0) {
      return cb;
    }
  }
  while (true) {
    buffer_cache->clock_hand = (buffer_cache->clock_hand + 1) % CACHE_SIZE;
    cb = buffer_cache->cached_blocks[buffer_cache->clock_hand];
    if (cb->use == 0) {
      return cb;
    } else {
      lock_acquire(&cb->lock);
      cb->use = 0;
      lock_release(&cb->lock);
    }
  }
}

void evict_and_replace(struct cached_block* cb, block_sector_t old_sector, bool flush) {
  // flush old data to old sector; write in data from updated sector
  if (flush) {
    block_write(fs_device, old_sector, cb->data);
  }
  block_read(fs_device, cb->sector, cb->data);
}

void buffer_cache_init(void) {
  buffer_cache = malloc(sizeof(struct buffer_cache));
  for (int i = 0; i < CACHE_SIZE; i++) {
    struct cached_block* cb = malloc(sizeof(struct cached_block));
    cb->use = 0;
    cb->dirty = 0;
    cb->valid = 0;
    lock_init(&cb->lock);
    cb->data = malloc(BLOCK_SECTOR_SIZE);
    memset(cb->data, 0, BLOCK_SECTOR_SIZE);
    buffer_cache->cached_blocks[i] = cb;
  }
  buffer_cache->clock_hand = 0;
  lock_init(&buffer_cache->lock);
}

struct cached_block* cache_search(block_sector_t sector) {
  for (int i = 0; i < CACHE_SIZE; i++) {
    struct cached_block* cb = buffer_cache->cached_blocks[i];
    lock_acquire(&cb->lock); // DIANA ADDED THIS
    if (cb->sector == sector && cb->valid == 1) {
      lock_release(&cb->lock);
      return cb;
    }
    lock_release(&cb->lock);
  }
  return NULL;
}

struct cached_block* get_cached_block(block_sector_t sector) {
  lock_acquire(&buffer_cache->lock);
  struct cached_block* cb = cache_search(sector);
  if (cb == NULL) {
    cb = clock_find_victim();

    bool flush = false;
    block_sector_t old_sector = cb->sector;
    if (cb->dirty == 1 && cb->valid == 1) {
      flush = true;
    }

    lock_acquire(&cb->lock); // DIANA MOVED THIS FROM BELOW LINE 100
    cb->sector = sector;
    cb->valid = 1;
    cb->use = 1;
    cb->dirty = 0;
    lock_release(&buffer_cache->lock);

    evict_and_replace(cb, old_sector, flush);
    lock_release(&cb->lock);

  } else {
    lock_release(&buffer_cache->lock);
  }
  return cb;
}

void cache_read(block_sector_t sector, void* buffer, off_t offset, size_t chunk_size) {
  while (true) {
    struct cached_block* cb = get_cached_block(sector);
    lock_acquire(&cb->lock);
    if (cb->sector == sector) {
      memcpy(buffer, cb->data + offset, chunk_size);
      lock_release(&cb->lock);
      return;
    }
    lock_release(&cb->lock);
  }
}

void cache_write(block_sector_t sector, void* buffer, off_t offset, size_t chunk_size) {
  while (true) {
    struct cached_block* cb = get_cached_block(sector);
    lock_acquire(&cb->lock); // We already know get_cached_block deals with buffer_cache->lock
    if (cb->sector == sector) {
      memcpy(cb->data + offset, buffer, chunk_size);
      cb->dirty = 1; // Set dirty under cb->lock only
      lock_release(&cb->lock);
      return;
    }
    lock_release(&cb->lock);
  }
}

void flush_cache() {
  lock_acquire(&buffer_cache->lock);
  for (int i = 0; i < CACHE_SIZE; i++) {
    struct cached_block* cb = buffer_cache->cached_blocks[i];
    if (cb->dirty == 1 && cb->valid == 1) {
      block_write(fs_device, cb->sector, cb->data);
      // CACHE
      // cache_write(cb->sector, cb->data, 0, BLOCK_SECTOR_SIZE);

      cb->dirty = 0;
    }
  }
  lock_release(&buffer_cache->lock);
}

/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44

/* Returns the number of sectors to allocate for an inode SIZE
   bytes long. */
static inline size_t bytes_to_sectors(off_t size) { return DIV_ROUND_UP(size, BLOCK_SECTOR_SIZE); }

/* On-disk inode.
   Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk {
  block_sector_t direct[12];      // The pointers to data blocks. 48 bytes total
  block_sector_t indirect;        //Indirect block pointer. 4 bytes
  block_sector_t doubly_indirect; // Doubly indirect block pointer. 4 bytes
  off_t length;                   // File Size. 4 bytes
  unsigned magic;                 // Magic number. 4 bytes
  bool is_dir;                    //For #Subdirectories usage. 4 bytes
  uint32_t unused[111];           // Padding to make it 512 bytes. 444 bytes
};

struct inode {
  struct list_elem elem;  /* Element in inode list. */
  block_sector_t sector;  /* Sector number of disk location. */
  int open_cnt;           /* Number of openers. */
  bool removed;           /* True if deleted, false otherwise. */
  int deny_write_cnt;     /* 0: writes ok, >0: deny writes. */
  struct lock inode_lock; //Lock for syncing access within the inode itself
};

/* Returns the block device sector that contains byte offset POS
   within INODE.
   Returns -1 if INODE does not contain data for a byte at offset
   POS. */
static block_sector_t byte_to_sector(const struct inode* inode, off_t pos) {
  ASSERT(inode != NULL);
  if (pos < inode_length(inode)) {
    int sector_index = pos / BLOCK_SECTOR_SIZE;
    block_sector_t sector;
    block_sector_t block_buffer[BLOCK_SECTOR_SIZE / sizeof(block_sector_t)];

    //DIRECT
    if (sector_index < DIRECT_BLOCKS) {
      // block_read(fs_device, inode->sector, block_buffer);
      // CACHE
      cache_read(inode->sector, block_buffer, 0, BLOCK_SECTOR_SIZE);

      sector = block_buffer[sector_index];
    }

    //INDIRECT
    else if (sector_index < DIRECT_BLOCKS + INDIRECT_BLOCKS) {
      block_sector_t single_indirect;
      // block_read(fs_device, inode->sector, block_buffer);
      // CACHE
      cache_read(inode->sector, block_buffer, 0, BLOCK_SECTOR_SIZE);

      single_indirect = block_buffer[DIRECT_BLOCKS];
      // block_read(fs_device, single_indirect, block_buffer);
      // CACHE
      cache_read(single_indirect, block_buffer, 0, BLOCK_SECTOR_SIZE);

      sector = block_buffer[sector_index - DIRECT_BLOCKS];
    }

    //DOUBLY
    else {
      size_t double_index = sector_index - (DIRECT_BLOCKS + INDIRECT_BLOCKS);
      // block_read(fs_device, inode->sector, block_buffer);
      // CACHE
      cache_read(inode->sector, block_buffer, 0, BLOCK_SECTOR_SIZE);

      block_sector_t double_indirect = block_buffer[DIRECT_BLOCKS + 1];
      // block_read(fs_device, double_indirect, block_buffer);
      // CACHE
      cache_read(double_indirect, block_buffer, 0, BLOCK_SECTOR_SIZE);

      size_t single_indirect_index = double_index / INDIRECT_BLOCKS;
      size_t sector_in_single = double_index % INDIRECT_BLOCKS;
      block_sector_t single_indirect_block = block_buffer[single_indirect_index];
      // block_read(fs_device, single_indirect_block, block_buffer);
      // CACHE
      cache_read(single_indirect_block, block_buffer, 0, BLOCK_SECTOR_SIZE);

      sector = block_buffer[sector_in_single];
    }
    return sector;
  } else {
    return -1;
  }
}

/* List of open inodes, so that opening a single inode twice
   returns the same `struct inode'. */
static struct list open_inodes;

// FREE TOOL
void free_blocks(struct inode_disk* disk_inode, size_t blocks_to_free) {
  size_t freed = 0;

  // Free direct blocks
  for (int i = 0; i < 12 && freed < blocks_to_free; i++) {
    if (disk_inode->direct[i] != 0) {
      free_map_release(disk_inode->direct[i], 1);
      disk_inode->direct[i] = 0;
      freed++;
    }
  }

  // Free indirect blocks
  if (freed < blocks_to_free && disk_inode->indirect != 0) {
    block_sector_t indirect[BLOCK_SECTOR_SIZE / sizeof(block_sector_t)];
    // block_read(fs_device, disk_inode->indirect, indirect);
    // CACHE
    cache_read(disk_inode->indirect, indirect, 0, BLOCK_SECTOR_SIZE);

    for (int i = 0; i < BLOCK_SECTOR_SIZE / sizeof(block_sector_t) && freed < blocks_to_free; i++) {
      if (indirect[i] != 0) {
        free_map_release(indirect[i], 1);
        indirect[i] = 0;
        freed++;
      }
    }

    if (freed >= blocks_to_free) {
      // block_write(fs_device, disk_inode->indirect, indirect);
      // CACHE
      cache_write(disk_inode->indirect, indirect, 0, BLOCK_SECTOR_SIZE);
    } else {
      free_map_release(disk_inode->indirect, 1);
      disk_inode->indirect = 0;
    }
  }

  // Free doubly indirect blocks
  if (freed < blocks_to_free && disk_inode->doubly_indirect != 0) {
    block_sector_t doubly_indirect[BLOCK_SECTOR_SIZE / sizeof(block_sector_t)];
    // block_read(fs_device, disk_inode->doubly_indirect, doubly_indirect);
    // CACHE
    cache_read(disk_inode->doubly_indirect, doubly_indirect, 0, BLOCK_SECTOR_SIZE);

    for (int i = 0; i < BLOCK_SECTOR_SIZE / sizeof(block_sector_t) && freed < blocks_to_free; i++) {
      if (doubly_indirect[i] != 0) {
        block_sector_t indirect_block = doubly_indirect[i];
        block_sector_t indirect[BLOCK_SECTOR_SIZE / sizeof(block_sector_t)];
        // block_read(fs_device, indirect_block, indirect);
        // CACHE
        cache_read(indirect_block, indirect, 0, BLOCK_SECTOR_SIZE);

        for (int j = 0; j < BLOCK_SECTOR_SIZE / sizeof(block_sector_t) && freed < blocks_to_free;
             j++) {
          if (indirect[j] != 0) {
            free_map_release(indirect[j], 1);
            indirect[j] = 0;
            freed++;
          }
        }

        // block_write(fs_device, indirect_block, indirect);
        // CACHE
        cache_write(indirect_block, indirect, 0, BLOCK_SECTOR_SIZE);

        free_map_release(indirect_block, 1);
        doubly_indirect[i] = 0;
      }
    }

    if (freed >= blocks_to_free) {
      // block_write(fs_device, disk_inode->doubly_indirect, doubly_indirect);
      // CACHE
      cache_write(disk_inode->doubly_indirect, doubly_indirect, 0, BLOCK_SECTOR_SIZE);
    } else {
      free_map_release(disk_inode->doubly_indirect, 1);
      disk_inode->doubly_indirect = 0;
    }
  }
}

// ALLOCATE TOOL
bool allocate_blocks(struct inode_disk* disk_inode, size_t blocks_to_allocate) {
  size_t allocated = 0;

  // Allocate direct blocks
  for (int i = 0; i < 12 && allocated < blocks_to_allocate; i++) {
    if (disk_inode->direct[i] == 0) {
      block_sector_t new_block;
      if (!free_map_allocate(1, &new_block)) {
        return false;
      }
      disk_inode->direct[i] = new_block;
      allocated++;
    }
  }

  // Allocate indirect blocks
  if (allocated < blocks_to_allocate) {
    if (disk_inode->indirect == 0) {
      block_sector_t indirect_block;
      if (!free_map_allocate(1, &indirect_block)) {
        return false;
      }
      disk_inode->indirect = indirect_block;
      block_sector_t indirect[BLOCK_SECTOR_SIZE / sizeof(block_sector_t)];
      memset(indirect, 0, sizeof(indirect));
      // block_write(fs_device, indirect_block, indirect);
      // CACHE
      cache_write(indirect_block, indirect, 0, BLOCK_SECTOR_SIZE);
    }

    block_sector_t indirect[BLOCK_SECTOR_SIZE / sizeof(block_sector_t)];
    // block_read(fs_device, disk_inode->indirect, indirect);
    // CACHE
    cache_read(disk_inode->indirect, indirect, 0, BLOCK_SECTOR_SIZE);

    for (int i = 0;
         i < BLOCK_SECTOR_SIZE / sizeof(block_sector_t) && allocated < blocks_to_allocate; i++) {
      if (indirect[i] == 0) {
        block_sector_t new_block;
        if (!free_map_allocate(1, &new_block)) {
          // block_write(fs_device, disk_inode->indirect, indirect);
          // CACHE
          cache_write(disk_inode->indirect, indirect, 0, BLOCK_SECTOR_SIZE);
          return false;
        }
        indirect[i] = new_block;
        allocated++;
      }
    }
    // block_write(fs_device, disk_inode->indirect, indirect);
    // CACHE
    cache_write(disk_inode->indirect, indirect, 0, BLOCK_SECTOR_SIZE);
  }

  // Allocate doubly indirect blocks
  if (allocated < blocks_to_allocate) {
    if (disk_inode->doubly_indirect == 0) {
      block_sector_t new_indirect;
      if (!free_map_allocate(1, &new_indirect)) {
        return false;
      }
      disk_inode->doubly_indirect = new_indirect;

      block_sector_t doubly_indirect_init[BLOCK_SECTOR_SIZE / sizeof(block_sector_t)];
      memset(doubly_indirect_init, 0, sizeof(doubly_indirect_init));
      // block_write(fs_device, new_indirect, doubly_indirect_init);
      // CACHE
      cache_write(new_indirect, doubly_indirect_init, 0, BLOCK_SECTOR_SIZE);
    }

    block_sector_t doubly_indirect[BLOCK_SECTOR_SIZE / sizeof(block_sector_t)];
    // block_read(fs_device, disk_inode->doubly_indirect, doubly_indirect);
    // CACHE
    cache_read(disk_inode->doubly_indirect, doubly_indirect, 0, BLOCK_SECTOR_SIZE);

    for (int i = 0;
         i < BLOCK_SECTOR_SIZE / sizeof(block_sector_t) && allocated < blocks_to_allocate; i++) {
      if (doubly_indirect[i] == 0) {
        block_sector_t new_indirect_block;
        if (!free_map_allocate(1, &new_indirect_block)) {
          // block_write(fs_device, disk_inode->doubly_indirect, doubly_indirect);
          // CACHE
          cache_write(disk_inode->doubly_indirect, doubly_indirect, 0, BLOCK_SECTOR_SIZE);

          return false;
        }
        doubly_indirect[i] = new_indirect_block;
        block_sector_t indirect[BLOCK_SECTOR_SIZE / sizeof(block_sector_t)];
        memset(indirect, 0, sizeof(indirect));
        // block_write(fs_device, new_indirect_block, indirect);
        // CACHE
        cache_write(new_indirect_block, indirect, 0, BLOCK_SECTOR_SIZE);
      }

      block_sector_t indirect[BLOCK_SECTOR_SIZE / sizeof(block_sector_t)];
      // block_read(fs_device, doubly_indirect[i], indirect);
      // CACHE
      cache_read(doubly_indirect[i], indirect, 0, BLOCK_SECTOR_SIZE);

      for (int j = 0;
           j < BLOCK_SECTOR_SIZE / sizeof(block_sector_t) && allocated < blocks_to_allocate; j++) {
        if (indirect[j] == 0) {
          block_sector_t new_data_block;
          if (!free_map_allocate(1, &new_data_block)) {
            // block_write(fs_device, doubly_indirect, doubly_indirect);
            // CACHE
            cache_write(disk_inode->doubly_indirect, doubly_indirect, 0, BLOCK_SECTOR_SIZE);
            return false;
          }
          indirect[j] = new_data_block;
          allocated++;
        }
      }
      // block_write(fs_device, doubly_indirect[i], indirect);
      // CACHE
      cache_write(doubly_indirect[i], indirect, 0, BLOCK_SECTOR_SIZE);
    }
    // block_write(fs_device, disk_inode->doubly_indirect, doubly_indirect);
    // CACHE
    cache_write(disk_inode->doubly_indirect, doubly_indirect, 0, BLOCK_SECTOR_SIZE);
  }

  return true;
}

/* Initializes the inode module. */
void inode_init(void) { list_init(&open_inodes); }

/* Initializes an inode with LENGTH bytes of data and
   writes the new inode to sector SECTOR on the file system
   device.
   Returns true if successful.
   Returns false if memory or disk allocation fails. */
bool inode_create(block_sector_t sector, off_t length) {
  struct inode_disk* disk_inode = NULL;
  bool success = false;

  ASSERT(length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  ASSERT(sizeof *disk_inode == BLOCK_SECTOR_SIZE);

  disk_inode = calloc(1, sizeof *disk_inode);
  if (disk_inode != NULL) {
    size_t sectors = bytes_to_sectors(length);
    disk_inode->length = length;
    disk_inode->magic = INODE_MAGIC;
    disk_inode->is_dir = false;

    memset(disk_inode->direct, 0, sizeof(disk_inode->direct));
    disk_inode->indirect = 0;
    disk_inode->doubly_indirect = 0;
    if (!allocate_blocks(disk_inode, sectors)) {
      free(disk_inode);
      return false;
    }

    // block_write(fs_device, sector, disk_inode);
    // CACHE
    cache_write(sector, disk_inode, 0, BLOCK_SECTOR_SIZE);

    free(disk_inode);

    return true;
  }
  return success;
}

bool inode_create_dir(block_sector_t sector, off_t length) {
  struct inode_disk* disk_inode = NULL;
  bool success = false;

  ASSERT(length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  ASSERT(sizeof *disk_inode == BLOCK_SECTOR_SIZE);

  disk_inode = calloc(1, sizeof *disk_inode);
  if (disk_inode != NULL) {
    size_t sectors = bytes_to_sectors(length);
    disk_inode->length = length;
    disk_inode->magic = INODE_MAGIC;
    disk_inode->is_dir = true;

    memset(disk_inode->direct, 0, sizeof(disk_inode->direct));
    disk_inode->indirect = 0;
    disk_inode->doubly_indirect = 0;
    if (!allocate_blocks(disk_inode, sectors)) {
      free(disk_inode);
      return false;
    }

    // block_write(fs_device, sector, disk_inode);
    // CACHE
    cache_write(sector, disk_inode, 0, BLOCK_SECTOR_SIZE);

    free(disk_inode);

    return true;
  }
  return success;
}

/* Reads an inode from SECTOR
   and returns a `struct inode' that contains it.
   Returns a null pointer if memory allocation fails. */
struct inode* inode_open(block_sector_t sector) {
  struct list_elem* e;
  struct inode* inode;

  /* Check whether this inode is already open. */
  for (e = list_begin(&open_inodes); e != list_end(&open_inodes); e = list_next(e)) {
    inode = list_entry(e, struct inode, elem);
    if (inode->sector == sector) {
      inode_reopen(inode);
      return inode;
    }
  }

  /* Allocate memory. */
  inode = malloc(sizeof *inode);
  if (inode == NULL)
    return NULL;

  /* Initialize. */
  list_push_front(&open_inodes, &inode->elem);
  inode->sector = sector;
  inode->open_cnt = 1;
  inode->deny_write_cnt = 0;
  inode->removed = false;
  lock_init(&inode->inode_lock);
  struct inode_disk disk_inode;
  // block_read(fs_device, inode->sector, &disk_inode);
  // CACHE
  cache_read(inode->sector, &disk_inode, 0, BLOCK_SECTOR_SIZE);

  return inode;
}

/* Reopens and returns INODE. */
struct inode* inode_reopen(struct inode* inode) {
  if (inode != NULL)
    inode->open_cnt++;
  return inode;
}

/* Returns INODE's inode number. */
block_sector_t inode_get_inumber(const struct inode* inode) { return inode->sector; }

/* Closes INODE and writes it to disk.
   If this was the last reference to INODE, frees its memory.
   If INODE was also a removed inode, frees its blocks. */
void inode_close(struct inode* inode) {
  /* Ignore null pointer. */
  if (inode == NULL)
    return;

  /* Release resources if this was the last opener. */
  if (--inode->open_cnt == 0) {
    /* Remove from inode list and release lock. */
    list_remove(&inode->elem);

    /* Deallocate blocks if removed. */
    if (inode->removed) {
      struct inode_disk disk_inode;
      // block_read(fs_device, inode->sector, &disk_inode);
      // CACHE
      cache_read(inode->sector, &disk_inode, 0, BLOCK_SECTOR_SIZE);

      free_map_release(inode->sector, 1);
      //free_blocks(&disk_inode, bytes_to_sectors(disk_inode.length));
    }
    free(inode);
  }
}

/* Marks INODE to be deleted when it is closed by the last caller who
   has it open. */
void inode_remove(struct inode* inode) {
  ASSERT(inode != NULL);
  inode->removed = true;
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t inode_read_at(struct inode* inode, void* buffer_, off_t size, off_t offset) {
  uint8_t* buffer = buffer_;
  off_t bytes_read = 0;
  uint8_t* bounce = NULL;
  lock_acquire(&inode->inode_lock);

  while (size > 0) {
    /* Disk sector to read, starting byte offset within sector. */
    block_sector_t sector_idx = byte_to_sector(inode, offset);
    int sector_ofs = offset % BLOCK_SECTOR_SIZE;

    /* Bytes left in inode, bytes left in sector, lesser of the two. */
    off_t inode_left = inode_length(inode) - offset;
    int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
    int min_left = inode_left < sector_left ? inode_left : sector_left;

    /* Number of bytes to actually copy out of this sector. */
    int chunk_size = size < min_left ? size : min_left;
    if (chunk_size <= 0)
      break;

    cache_read(sector_idx, buffer + bytes_read, sector_ofs, chunk_size);
    // if (sector_ofs == 0 && chunk_size == BLOCK_SECTOR_SIZE) {
    //   /* Read full sector directly into caller's buffer. */
    //   block_read(fs_device, sector_idx, buffer + bytes_read);
    // } else {
    //   /* Read sector into bounce buffer, then partially copy
    //          into caller's buffer. */
    //   if (bounce == NULL) {
    //     bounce = malloc(BLOCK_SECTOR_SIZE);
    //     if (bounce == NULL)
    //       break;
    //   }
    //   block_read(fs_device, sector_idx, bounce);
    //   memcpy(buffer + bytes_read, bounce + sector_ofs, chunk_size);
    // }

    /* Advance. */
    size -= chunk_size;
    offset += chunk_size;
    bytes_read += chunk_size;
  }

  lock_release(&inode->inode_lock);

  return bytes_read;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
   Returns the number of bytes actually written, which may be
   less than SIZE if end of file is reached or an error occurs.
   (Normally a write at end of file would extend the inode, but
   growth is not yet implemented.) */
off_t inode_write_at(struct inode* inode, const void* buffer_, off_t size, off_t offset) {
  const uint8_t* buffer = buffer_;
  off_t bytes_written = 0;
  uint8_t* bounce = NULL;
  lock_acquire(&inode->inode_lock);

  if (inode->deny_write_cnt) {
    lock_release(&inode->inode_lock);
    return 0;
  }

  //CHECK

  if (offset + size >= inode_length(inode)) {
    if (!inode_resize(inode, offset + size)) {
      lock_release(&inode->inode_lock);
      return bytes_written;
    }
  }

  while (size > 0) {
    /* Sector to write, starting byte offset within sector. */
    block_sector_t sector_idx = byte_to_sector(inode, offset);
    int sector_ofs = offset % BLOCK_SECTOR_SIZE;

    /* Bytes left in inode, bytes left in sector, lesser of the two. */
    off_t inode_left = inode_length(inode) - offset;
    int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
    int min_left = inode_left < sector_left ? inode_left : sector_left;

    /* Number of bytes to actually write into this sector. */
    int chunk_size = size < min_left ? size : min_left;
    if (chunk_size <= 0)
      break;

    cache_write(sector_idx, buffer + bytes_written, sector_ofs, chunk_size);
    // if (sector_ofs == 0 && chunk_size == BLOCK_SECTOR_SIZE) {
    //   /* Write full sector directly to disk. */
    //   block_write(fs_device, sector_idx, buffer + bytes_written);
    // } else {
    //   /* We need a bounce buffer. */
    //   if (bounce == NULL) {
    //     bounce = malloc(BLOCK_SECTOR_SIZE);
    //     if (bounce == NULL)
    //       break;
    //   }

    //   /* If the sector contains data before or after the chunk
    //          we're writing, then we need to read in the sector
    //          first.  Otherwise we start with a sector of all zeros. */
    //   if (sector_ofs > 0 || chunk_size < sector_left)
    //     block_read(fs_device, sector_idx, bounce);
    //   else
    //     memset(bounce, 0, BLOCK_SECTOR_SIZE);
    //   memcpy(bounce + sector_ofs, buffer + bytes_written, chunk_size);
    //   block_write(fs_device, sector_idx, bounce);
    // }

    /* Advance. */
    size -= chunk_size;
    offset += chunk_size;
    bytes_written += chunk_size;
  }

  lock_release(&inode->inode_lock);
  return bytes_written;
}

/* Disables writes to INODE.
   May be called at most once per inode opener. */
void inode_deny_write(struct inode* inode) {
  inode->deny_write_cnt++;
  ASSERT(inode->deny_write_cnt <= inode->open_cnt);
}

/* Re-enables writes to INODE.
   Must be called once by each inode opener who has called
   inode_deny_write() on the inode, before closing the inode. */
void inode_allow_write(struct inode* inode) {
  ASSERT(inode->deny_write_cnt > 0);
  ASSERT(inode->deny_write_cnt <= inode->open_cnt);
  inode->deny_write_cnt--;
}

// Function implementations remain the same as you wrote before

bool inode_resize(struct inode* inode, off_t new_size) {
  ASSERT(inode != NULL);

  struct inode_disk* disk_inode = calloc(1, sizeof(struct inode_disk));
  if (!disk_inode) {
    return false;
  }
  // block_read(fs_device, inode->sector, disk_inode);
  // CACHE
  cache_read(inode->sector, disk_inode, 0, BLOCK_SECTOR_SIZE);

  off_t old_size = disk_inode->length;
  size_t old_sectors = bytes_to_sectors(old_size);
  size_t new_sectors = bytes_to_sectors(new_size);

  //shrinking
  if (new_sectors < old_sectors) {
    size_t blocks_to_free = old_sectors - new_sectors;
    free_blocks(disk_inode, blocks_to_free);
  }

  //expanding
  else if (new_sectors > old_sectors) {
    size_t blocks_to_allocate = new_sectors - old_sectors;
    if (!allocate_blocks(disk_inode, blocks_to_allocate)) {
      free_blocks(disk_inode, blocks_to_allocate);
      free(disk_inode);
      return false;
    }
  }

  disk_inode->length = new_size;

  // block_write(fs_device, inode->sector, disk_inode);
  cache_write(inode->sector, disk_inode, 0, BLOCK_SECTOR_SIZE);
  free(disk_inode);

  return true;
}

/* Returns the length, in bytes, of INODE's data. */
off_t inode_length(const struct inode* inode) {
  struct inode_disk disk_inode;
  // block_read(fs_device, inode->sector, &disk_inode);
  // CACHE
  cache_read(inode->sector, &disk_inode, 0, BLOCK_SECTOR_SIZE);
  return disk_inode.length;
}

bool is_dir(const struct inode* inode) {
  struct inode_disk disk_inode;
  // block_read(fs_device, inode->sector, &disk_inode);
  // CACHE
  cache_read(inode->sector, &disk_inode, 0, BLOCK_SECTOR_SIZE);
  return disk_inode.is_dir;
}

bool inode_is_removed(const struct inode* inode) { return inode->removed; }
