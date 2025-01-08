#include "filesys/directory.h"
#include <stdio.h>
#include <string.h>
#include <list.h>
#include "filesys/filesys.h"
#include "filesys/inode.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "userprog/process.h"

/* Creates a directory with space for ENTRY_CNT entries in the
   given SECTOR.  Returns true if successful, false on failure. */
bool dir_create(block_sector_t sector, size_t entry_cnt) {
  return inode_create_dir(sector, (entry_cnt + 2) * sizeof(struct dir_entry));
}

/* Opens and returns the directory for the given INODE, of which
   it takes ownership.  Returns a null pointer on failure. */
struct dir* dir_open(struct inode* inode) {
  struct dir* dir = calloc(1, sizeof *dir);
  if (inode != NULL && dir != NULL) {
    dir->inode = inode;
    dir->pos = 0;
    return dir;
  } else {
    inode_close(inode);
    free(dir);
    return NULL;
  }
}

/* Opens the root directory and returns a directory for it.
   Return true if successful, false on failure. */
struct dir* dir_open_root(void) {
  return dir_open(inode_open(ROOT_DIR_SECTOR));
}

/* Opens and returns a new directory for the same inode as DIR.
   Returns a null pointer on failure. */
struct dir* dir_reopen(struct dir* dir) {
  return dir_open(inode_reopen(dir->inode));
}

/* Destroys DIR and frees associated resources. */
void dir_close(struct dir* dir) {
  if (dir != NULL) {
    inode_close(dir->inode);
    free(dir);
  }
}

/* Returns the inode encapsulated by DIR. */
struct inode* dir_get_inode(struct dir* dir) {
  return dir->inode;
}

/* Searches DIR for a file with the given NAME.
   If successful, returns true, sets *EP to the directory entry
   if EP is non-null, and sets *OFSP to the byte offset of the
   directory entry if OFSP is non-null.
   otherwise, returns false and ignores EP and OFSP. */
static bool lookup(const struct dir* dir, const char* name, struct dir_entry* ep, off_t* ofsp) {
  struct dir_entry e;
  size_t ofs;

  ASSERT(dir != NULL);
  ASSERT(name != NULL);
  ASSERT(dir->inode != NULL);

  for (ofs = 0; inode_read_at(dir->inode, &e, sizeof e, ofs) == sizeof e; ofs += sizeof e)
    if (e.in_use && !strcmp(name, e.name)) {
      if (ep != NULL)
        *ep = e;
      if (ofsp != NULL)
        *ofsp = ofs;
      return true;
    }
  return false;
}

/* Searches DIR for a file with the given NAME
   and returns true if one exists, false otherwise.
   On success, sets *INODE to an inode for the file, otherwise to
   a null pointer.  The caller must close *INODE. */
bool dir_lookup(const struct dir* dir, const char* name, struct inode** inode) {
  struct dir_entry e;

  ASSERT(dir != NULL);
  ASSERT(name != NULL);

  if (lookup(dir, name, &e, NULL))
    *inode = inode_open(e.inode_sector);
  else
    *inode = NULL;

  return *inode != NULL;
}

/* Adds a file named NAME to DIR, which must not already contain a
   file by that name.  The file's inode is in sector
   INODE_SECTOR.
   Returns true if successful, false on failure.
   Fails if NAME is invalid (i.e. too long) or a disk or memory
   error occurs. */
bool dir_add(struct dir* dir, const char* name, block_sector_t inode_sector) {
  struct dir_entry e;
  off_t ofs;
  bool success = false;

  ASSERT(dir != NULL);
  ASSERT(name != NULL);

  /* Check NAME for validity. */
  if (*name == '\0' || strlen(name) > NAME_MAX)
    return false;

  /* Check that NAME is not in use. */
  if (lookup(dir, name, NULL, NULL))
    goto done;

  /* Set OFS to offset of free slot.
     If there are no free slots, then it will be set to the
     current end-of-file.
     inode_read_at() will only return a short read at end of file.
     Otherwise, we'd need to verify that we didn't get a short
     read due to something intermittent such as low memory. */
  for (ofs = 0; inode_read_at(dir->inode, &e, sizeof e, ofs) == sizeof e; ofs += sizeof e)
    if (!e.in_use)
      break;

  /* Write slot. */
  e.in_use = true;
  strlcpy(e.name, name, sizeof e.name);
  e.inode_sector = inode_sector;
  success = inode_write_at(dir->inode, &e, sizeof e, ofs) == sizeof e;

  if (success)
    dir->num_entries++;

done:
  return success;
}

/* Removes any entry for NAME in DIR.
   Returns true if successful, false on failure,
   which occurs only if there is no file with the given NAME. */
bool dir_remove(struct dir* dir, const char* name) {
  struct dir_entry e;
  struct inode* inode = NULL;
  bool success = false;
  off_t ofs;

  ASSERT(dir != NULL);
  ASSERT(name != NULL);

  /* Find directory entry. */
  if (!lookup(dir, name, &e, &ofs))
    goto done;

  /* Open inode. */
  inode = inode_open(e.inode_sector);
  if (inode == NULL)
    goto done;

  /* Erase directory entry. */
  e.in_use = false;
  if (inode_write_at(dir->inode, &e, sizeof e, ofs) != sizeof e)
    goto done;

  /* Remove inode. */
  inode_remove(inode);
  success = true;

  if (success)
    dir->num_entries--;

done:
  inode_close(inode);
  return success;
}

/* Reads the next directory entry in DIR and stores the name in
   NAME.  Returns true if successful, false if the directory
   contains no more entries. */
bool dir_readdir(struct dir* dir, char name[NAME_MAX + 1]) {
  struct dir_entry e;

  while (inode_read_at(dir->inode, &e, sizeof e, dir->pos) == sizeof e) {
    dir->pos += sizeof e;
    if (e.in_use) {
      if (strcmp(e.name, ".") != 0 && strcmp(e.name, "..") != 0) {
        strlcpy(name, e.name, NAME_MAX + 1);
        return true;
      }
    }
  }
  return false;
}

bool system_chdir(const char* dir) {
  struct dir* start_dir = dir_reopen(thread_current()->pcb->work_dir);
  char next_name[NAME_MAX + 1];
  struct inode* inode = NULL;
  bool lookup_val;

  if (dir[0] == '/') {
    dir_close(start_dir);
    start_dir = dir_open_root();
  }

  int path_part = get_next_part(next_name, &dir);
  while (path_part == 1) {
    if (!dir_lookup(start_dir, next_name, &inode)) {
      dir_close(start_dir);
      return false;
    }

    struct dir* next_dir = dir_open(inode);
    if (!next_dir) {
      dir_close(start_dir);
      return false;
    }

    dir_close(start_dir);
    start_dir = next_dir;

    path_part = get_next_part(next_name, &dir);
  }

  if (path_part == -1) {
    dir_close(start_dir);
    return false;
  }

  // Update the current working directory
  dir_close(thread_current()->pcb->work_dir);
  thread_current()->pcb->work_dir = start_dir;
  return true;
}

bool system_mkdir(const char* dir) {
  if (*dir == '\0') {
    return false;
  }

  struct dir* start_dir = dir_reopen(thread_current()->pcb->work_dir);
  char next_name[NAME_MAX + 1];
  block_sector_t num = 0;
  bool lookup_val;
  bool success;

  int is_absolute = (dir[0] == '/');
  if (is_absolute) {
    start_dir = dir_open_root();
  }

  struct inode* curr_file;
  int path_part = get_next_part(next_name, &dir);
  while (path_part == 1) {
    lookup_val = dir_lookup(start_dir, next_name, &curr_file);

    if (*dir == '\0') {
      if (lookup_val) {
        return false;
      }
      break;
    }

    if (path_part == -1)
      return false;

    if (lookup_val) {
      if (!is_dir(curr_file)) {
        return false;
      } else {
        dir_close(start_dir);
        start_dir = dir_open(curr_file);
      }
    } else {
      return false;
    }
    path_part = get_next_part(next_name, &dir);
  }

  success = (start_dir != NULL && free_map_allocate(1, &num) && dir_create(num, 1) &&
             dir_add(start_dir, &next_name, num));
  if (!success && num != 0)
    free_map_release(num, 1);

  struct inode* curr_node;
  const char* dot = ".";
  const char* dotdot = "..";
  dir_lookup(start_dir, next_name, &curr_node);
  struct dir* curr_dir = dir_open(curr_node);

  success = (free_map_allocate(1, &num) && curr_node != NULL &&
             dir_add(curr_dir, dot, inode_get_inumber(curr_node)) &&
             dir_add(curr_dir, dotdot, inode_get_inumber(start_dir->inode)));

  dir_close(start_dir);
  dir_close(curr_dir);

  return success;
}