#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "threads/thread.h"
#include "userprog/process.h"

/* Partition that contains the file system. */
struct block* fs_device;

static void do_format(void);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void filesys_init(bool format) {
  fs_device = block_get_role(BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC("No file system device found, can't initialize file system.");

  inode_init();
  free_map_init();
  buffer_cache_init();

  if (format)
    do_format();

  free_map_open();
  thread_current()->pcb->work_dir = dir_open_root();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void filesys_done(void) {
  free_map_close();
  flush_cache();
}

bool traverse_path(const char* path, struct dir** dir, struct inode** curr_inode,
                   char name[NAME_MAX + 1]) {

  if (*path == '\0') {
    return false;
  }

  struct dir* cur_dir =
      (*path == '/') ? dir_open_root() : dir_reopen(thread_current()->pcb->work_dir);

  if (inode_is_removed(cur_dir->inode)) {
    dir_close(cur_dir);
    return false;
  }

  struct inode* inode = inode_reopen(cur_dir->inode);

  while (get_next_part(name, &path)) {
    if (strcmp(name, ".") == 0) {
      continue;
    }
    if (inode == NULL) {
      dir_close(cur_dir);
      return false;
    }
    if (!is_dir(inode)) {
      inode_close(inode);
      dir_close(cur_dir);
      return false;
    } else {

      struct dir* next_dir = dir_open(inode);
      dir_close(cur_dir);
      cur_dir = next_dir;
      dir_lookup(cur_dir, name, &inode);
    }
  }

  *dir = cur_dir;
  *curr_inode = inode;
  return true;
}

/* Creates a file named NAME with the given INITIAL_SIbZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool filesys_create(const char* name, off_t initial_size) {
  /*
  if (thread_current()->pcb->work_dir == NULL) {
    thread_current()->pcb->work_dir = dir_open_root();
  }
  */
  struct dir* start_dir = dir_reopen(thread_current()->pcb->work_dir);
  struct inode* curr_inode = NULL;
  char next_name[NAME_MAX + 1];

  if (!traverse_path(name, &start_dir, &curr_inode, next_name)) {
    dir_close(start_dir);
    return false;
  }
  if (curr_inode != NULL) {
    inode_close(curr_inode);
    dir_close(start_dir);
    return false;
  }

  block_sector_t inode_sector = 0;
  bool success = free_map_allocate(1, &inode_sector) && inode_create(inode_sector, initial_size) &&
                 dir_add(start_dir, next_name, inode_sector);

  if (!success && inode_sector != 0) {
    free_map_release(inode_sector, 1);
  }

  dir_close(start_dir);
  return success;
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file* filesys_open(const char* name) {
  /*
  if (thread_current()->pcb->work_dir == NULL) {
    thread_current()->pcb->work_dir = dir_open_root();
  }*/

  struct dir* start_dir = dir_reopen(thread_current()->pcb->work_dir);
  struct inode* curr_inode = NULL;
  char next_name[NAME_MAX + 1];

  if (!traverse_path(name, &start_dir, &curr_inode, next_name)) {
    dir_close(start_dir);
    return NULL;
  }

  dir_lookup(start_dir, next_name, &curr_inode);

  return file_open(curr_inode);
}

bool is_empty(struct inode* inode) {
  struct dir_entry e;
  for (size_t ofs = 0; inode_read_at(inode, &e, sizeof e, ofs) == sizeof e; ofs += sizeof e) {
    if (e.in_use) {
      // Use strcmp to check the names of directory entries
      if (strcmp(e.name, ".") != 0 && strcmp(e.name, "..") != 0) {
        return false; // Found an entry that is neither "." nor ".."
      }
    }
  }
  return true; // No entries found other than "." and ".."
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool filesys_remove(const char* name) {
  struct dir* start_dir;
  char next_name[NAME_MAX + 1];
  struct inode* curr_file = NULL;
  bool lookup_val;

  if (!traverse_path(name, &start_dir, &curr_file, next_name)) {
    dir_close(start_dir);
    return false;
  }

  bool success = true;

  //need to check if directory is empty
  struct inode* inode;
  if (!dir_lookup(start_dir, next_name, &inode)) {
    dir_close(start_dir);
    return false;
  }
  if (is_dir(inode) && !is_empty(inode)) {
    dir_close(start_dir);
    return false;
  }
  struct thread* t = thread_current();
  if (inode_get_inumber(inode) == inode_get_inumber(t->pcb->work_dir->inode)) {
    return false;
  }
  struct list_elem* e;
  struct list* list = &t->pcb->fds;
  for (e = list_begin(list); e != list_end(list); e = list_next(e)) {
    struct file_descriptor* fd;
    fd = list_entry(e, struct file_descriptor, elem);
    if (fd->is_dir && inode_get_inumber(inode) == inode_get_inumber(fd->directory->inode)) {
      return false;
    }
  }
  success = dir_remove(start_dir, next_name);

  inode_close(curr_file);
  dir_close(start_dir);

  return success;
}

/* Formats the file system. */
static void do_format(void) {
  printf("Formatting file system...");
  free_map_create();
  if (!dir_create(ROOT_DIR_SECTOR, 16))
    PANIC("root directory creation failed");
  free_map_close();
  printf("done.\n");
}

int get_next_part(char part[NAME_MAX + 1], const char** srcp) {
  const char* src = *srcp;
  char* dst = part;

  /* Skip leading slashes.  If it's all slashes, we're done. */
  while (*src == '/')
    src++;
  if (*src == '\0')
    return 0;

  /* Copy up to NAME_MAX character from SRC to DST.  Add null terminator. */
  while (*src != '/' && *src != '\0') {
    if (dst < part + NAME_MAX)
      *dst++ = *src;
    else
      return -1;
    src++;
  }
  *dst = '\0';

  /* Advance source pointer. */
  *srcp = src;
  return 1;
}