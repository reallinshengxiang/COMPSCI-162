#include "userprog/syscall.h"
#include <stdio.h>
#include <float.h>
#include <string.h>
#include <syscall-nr.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"

static void syscall_handler(struct intr_frame*);
static void copy_in(void*, const void*, size_t);

/* Serializes file system operations. */
static struct lock fs_lock;

void syscall_init(void) {
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&fs_lock);
}

/* System call handler. */
static void syscall_handler(struct intr_frame* f) {
  typedef int syscall_function(int, int, int);

  /* A system call. */
  struct syscall {
    size_t arg_cnt;         /* Number of arguments. */
    syscall_function* func; /* Implementation. */
  };

  /* Table of system calls. */
  static const struct syscall syscall_table[] = {
      {0, (syscall_function*)sys_halt},      {1, (syscall_function*)sys_exit},
      {1, (syscall_function*)sys_exec},      {1, (syscall_function*)sys_wait},
      {2, (syscall_function*)sys_create},    {1, (syscall_function*)sys_remove},
      {1, (syscall_function*)sys_open},      {1, (syscall_function*)sys_filesize},
      {3, (syscall_function*)sys_read},      {3, (syscall_function*)sys_write},
      {2, (syscall_function*)sys_seek},      {1, (syscall_function*)sys_tell},
      {1, (syscall_function*)sys_close},     {1, (syscall_function*)sys_practice},
      {1, (syscall_function*)sys_compute_e},
  };

  const struct syscall* sc;
  unsigned call_nr;
  int args[3];

  /* Get the system call. */
  copy_in(&call_nr, f->esp, sizeof call_nr);
  if (call_nr >= sizeof syscall_table / sizeof *syscall_table)
    process_exit();
  sc = syscall_table + call_nr;

  if (sc->func == NULL)
    process_exit();

  /* Get the system call arguments. */
  ASSERT(sc->arg_cnt <= sizeof args / sizeof *args);
  memset(args, 0, sizeof args);
  copy_in(args, (uint32_t*)f->esp + 1, sizeof *args * sc->arg_cnt);

  /* Execute the system call,
     and set the return value. */
  f->eax = sc->func(args[0], args[1], args[2]);
}

/* Closes a file safely */
void safe_file_close(struct file* file) {
  lock_acquire(&fs_lock);
  file_close(file);
  lock_release(&fs_lock);
}

/* Returns true if UADDR is a valid, mapped user address,
   false otherwise. */
static bool verify_user(const void* uaddr) {
  return (uaddr < PHYS_BASE && pagedir_get_page(thread_current()->pcb->pagedir, uaddr) != NULL);
}

/* Copies a byte from user address USRC to kernel address DST.
   USRC must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static inline bool get_user(uint8_t* dst, const uint8_t* usrc) {
  int eax;
  asm("movl $1f, %%eax; movb %2, %%al; movb %%al, %0; 1:" : "=m"(*dst), "=&a"(eax) : "m"(*usrc));
  return eax != 0;
}

/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static inline bool put_user(uint8_t* udst, uint8_t byte) {
  int eax;
  asm("movl $1f, %%eax; movb %b2, %0; 1:" : "=m"(*udst), "=&a"(eax) : "q"(byte));
  return eax != 0;
}

/* Copies SIZE bytes from user address USRC to kernel address
   DST.
   Call process_exit() if any of the user accesses are invalid. */
static void copy_in(void* dst_, const void* usrc_, size_t size) {
  uint8_t* dst = dst_;
  const uint8_t* usrc = usrc_;

  for (; size > 0; size--, dst++, usrc++)
    if (usrc >= (uint8_t*)PHYS_BASE || !get_user(dst, usrc))
      process_exit();
}

/* Creates a copy of user string US in kernel memory
   and returns it as a page that must be freed with
   palloc_free_page().
   Truncates the string at PGSIZE bytes in size.
   Call process_exit() if any of the user accesses are invalid. */
static char* copy_in_string(const char* us) {
  char* ks;
  size_t length;

  ks = palloc_get_page(0);
  if (ks == NULL)
    process_exit();

  for (length = 0; length < PGSIZE; length++) {
    if (us >= (char*)PHYS_BASE || !get_user(ks + length, us++)) {
      palloc_free_page(ks);
      process_exit();
    }

    if (ks[length] == '\0')
      return ks;
  }
  ks[PGSIZE - 1] = '\0';
  return ks;
}

/* Halt system call. */
int sys_halt(void) { shutdown_power_off(); }

/* Exit system call. */
int sys_exit(int exit_code) {
  thread_current()->pcb->wait_status->exit_code = exit_code;
  process_exit();
  NOT_REACHED();
}

/* Exec system call. */
int sys_exec(const char* ufile) {
  pid_t tid;
  char* kfile = copy_in_string(ufile);

  lock_acquire(&fs_lock);
  tid = process_execute(kfile);
  lock_release(&fs_lock);

  palloc_free_page(kfile);

  return tid;
}

/* Wait system call. */
int sys_wait(pid_t child) { return process_wait(child); }

/* Create system call. */
int sys_create(const char* ufile, unsigned initial_size) {
  char* kfile = copy_in_string(ufile);
  bool ok;

  lock_acquire(&fs_lock);
  ok = filesys_create(kfile, initial_size);
  lock_release(&fs_lock);

  palloc_free_page(kfile);

  return ok;
}

/* Remove system call. */
int sys_remove(const char* ufile) {
  char* kfile = copy_in_string(ufile);
  bool ok;

  lock_acquire(&fs_lock);
  ok = filesys_remove(kfile);
  lock_release(&fs_lock);

  palloc_free_page(kfile);

  return ok;
}

/* Open system call. */
int sys_open(const char* ufile) {
  char* kfile = copy_in_string(ufile);
  struct file_descriptor* fd;
  int handle = -1;

  fd = malloc(sizeof *fd);
  if (fd != NULL) {
    lock_acquire(&fs_lock);
    fd->file = filesys_open(kfile);
    if (fd->file != NULL) {
      struct thread* cur = thread_current();
      handle = fd->handle = cur->pcb->next_handle++;
      list_push_front(&cur->pcb->fds, &fd->elem);
    } else
      free(fd);
    lock_release(&fs_lock);
  }

  palloc_free_page(kfile);
  return handle;
}

/* Returns the file descriptor associated with the given handle.
   Terminates the process if HANDLE is not associated with an
   open file. */
static struct file_descriptor* lookup_fd(int handle) {
  struct thread* cur = thread_current();
  struct list_elem* e;

  for (e = list_begin(&cur->pcb->fds); e != list_end(&cur->pcb->fds); e = list_next(e)) {
    struct file_descriptor* fd;
    fd = list_entry(e, struct file_descriptor, elem);
    if (fd->handle == handle)
      return fd;
  }

  process_exit();
  NOT_REACHED();
}

/* Filesize system call. */
int sys_filesize(int handle) {
  struct file_descriptor* fd = lookup_fd(handle);
  int size;

  lock_acquire(&fs_lock);
  size = file_length(fd->file);
  lock_release(&fs_lock);

  return size;
}

/* Read system call. */
int sys_read(int handle, void* udst_, unsigned size) {
  uint8_t* udst = udst_;
  struct file_descriptor* fd;
  int bytes_read = 0;

  /* Handle keyboard reads. */
  if (handle == STDIN_FILENO) {
    for (bytes_read = 0; (size_t)bytes_read < size; bytes_read++)
      if (udst >= (uint8_t*)PHYS_BASE || !put_user(udst++, input_getc()))
        process_exit();
    return bytes_read;
  }

  /* Handle all other reads. */
  fd = lookup_fd(handle);
  lock_acquire(&fs_lock);
  while (size > 0) {
    /* How much to read into this page? */
    size_t page_left = PGSIZE - pg_ofs(udst);
    size_t read_amt = size < page_left ? size : page_left;
    off_t retval;

    /* Check that touching this page is okay. */
    if (!verify_user(udst)) {
      lock_release(&fs_lock);
      process_exit();
    }

    /* Read from file into page. */
    retval = file_read(fd->file, udst, read_amt);
    if (retval < 0) {
      if (bytes_read == 0)
        bytes_read = -1;
      break;
    }
    bytes_read += retval;

    /* If it was a short read we're done. */
    if (retval != (off_t)read_amt)
      break;

    /* Advance. */
    udst += retval;
    size -= retval;
  }
  lock_release(&fs_lock);

  return bytes_read;
}

/* Write system call. */
int sys_write(int handle, void* usrc_, unsigned size) {
  uint8_t* usrc = usrc_;
  struct file_descriptor* fd = NULL;
  int bytes_written = 0;

  /* Lookup up file descriptor. */
  if (handle != STDOUT_FILENO)
    fd = lookup_fd(handle);

  lock_acquire(&fs_lock);
  while (size > 0) {
    /* How much bytes to write to this page? */
    size_t page_left = PGSIZE - pg_ofs(usrc);
    size_t write_amt = size < page_left ? size : page_left;
    off_t retval;

    /* Check that we can touch this user page. */
    if (!verify_user(usrc)) {
      lock_release(&fs_lock);
      process_exit();
    }

    /* Do the write. */
    if (handle == STDOUT_FILENO) {
      putbuf(usrc, write_amt);
      retval = write_amt;
    } else
      retval = file_write(fd->file, usrc, write_amt);
    if (retval < 0) {
      if (bytes_written == 0)
        bytes_written = -1;
      break;
    }
    bytes_written += retval;

    /* If it was a short write we're done. */
    if (retval != (off_t)write_amt)
      break;

    /* Advance. */
    usrc += retval;
    size -= retval;
  }
  lock_release(&fs_lock);

  return bytes_written;
}

/* Seek system call. */
int sys_seek(int handle, unsigned position) {
  struct file_descriptor* fd = lookup_fd(handle);

  lock_acquire(&fs_lock);
  if ((off_t)position >= 0)
    file_seek(fd->file, position);
  lock_release(&fs_lock);

  return 0;
}

/* Tell system call. */
int sys_tell(int handle) {
  struct file_descriptor* fd = lookup_fd(handle);
  unsigned position;

  lock_acquire(&fs_lock);
  position = file_tell(fd->file);
  lock_release(&fs_lock);

  return position;
}

/* Close system call. */
int sys_close(int handle) {
  struct file_descriptor* fd = lookup_fd(handle);
  safe_file_close(fd->file);
  list_remove(&fd->elem);
  free(fd);
  return 0;
}

/* Practice system call. */
int sys_practice(int input) { return input + 1; }

/* Compute e and return a float cast to an int */
int sys_compute_e(int n) { return sys_sum_to_e(n); }
