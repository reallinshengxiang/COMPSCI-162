#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "lib/float.h"

static void syscall_handler(struct intr_frame*);

struct lock filesys_lock;

void syscall_init(void) {
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&filesys_lock);
}

static void syscall_handler(struct intr_frame* f UNUSED) {
  uint32_t* args = ((uint32_t*)f->esp);

  /*
   * The following print statement, if uncommented, will print out the syscall
   * number whenever a process enters a system call. You might find it useful
   * when debugging. It will cause tests to fail, however, so you should not
   * include it in your final submission.
   */

  //printf("System call number: %d\n", args[0]);

  void validate_ptr(const void* ptr) {
    if (!is_user_vaddr(ptr)) {
      f->eax = -1;
      printf("%s: exit(%d)\n", thread_current()->pcb->process_name, -1);
      process_exit(-1);
    }
    if (!pagedir_get_page(thread_current()->pcb->pagedir, ptr)) {
      f->eax = -1;
      printf("%s: exit(%d)\n", thread_current()->pcb->process_name, -1);
      process_exit(-1);
    }
  }

  void validate_string(const void* ptr) {
    char* str = (char*)ptr;
    while (true) {
      if (!is_user_vaddr(str)) {
        f->eax = -1;
        printf("%s: exit(%d)\n", thread_current()->pcb->process_name, -1);
        process_exit(-1);
      }
      if (!pagedir_get_page(thread_current()->pcb->pagedir, str)) {
        f->eax = -1;
        printf("%s: exit(%d)\n", thread_current()->pcb->process_name, -1);
        process_exit(-1);
      }
      if (*str == '\0') {
        return;
      }
      str++;
    }
  }

  for (size_t i = 0; i < sizeof(uint32_t) * 4; i++) {
    validate_ptr(f->esp + i);
  }

  //EXIT IF TOO BIG
  if (f->esp >= PHYS_BASE - sizeof(uint32_t)) {
    args[0] = SYS_EXIT;
    args[1] = -1;
  }

  if (args[0] == SYS_CREATE) {

    validate_ptr(args[1]);

    char* file_name = (char*)args[1];
    int init_size = (int)args[2];
    if (file_name == NULL) {
      f->eax = 0;
      return;
    } else if (init_size < 0) {
      f->eax = 0;
      return;
    } else {
      lock_acquire(&filesys_lock);
      bool status = filesys_create(file_name, init_size);
      lock_release(&filesys_lock);
      if (status) {
        f->eax = 1;
      } else {
        f->eax = 0;
      }
    }

    return;
  }

  if (args[0] == SYS_OPEN) {
    validate_ptr(args[1]);
    struct thread* t = thread_current();
    char* file_name = (char*)args[1];
    lock_acquire(&filesys_lock);
    struct file* of = filesys_open(file_name);

    if (of == NULL) {
      f->eax = -1;
      return;
    }

    lock_release(&filesys_lock);

    int fd_num = -1;
    struct list_elem* iter;
    for (iter = list_begin(&(t->pcb->fd_list.lst)); iter != list_end(&(t->pcb->fd_list.lst));
         iter = list_next(iter)) {
      struct file_descriptor* fd = list_entry(iter, struct file_descriptor, elem);
      if (fd->file == NULL) {
        fd->file = of;
      }
    }

    int next_fd = t->pcb->fd_list.next_fd;
    struct file_descriptor* fd = malloc(sizeof(struct file_descriptor));
    if (fd == NULL) {
      f->eax = -1;
      return;
    }
    fd->file = of;
    t->pcb->fd_list.next_fd = next_fd;
    list_push_back(&t->pcb->fd_list.lst, &fd->elem);

    t->pcb->fd_list.next_fd++;
    f->eax = next_fd;

    return;
  }

  if (args[0] == SYS_REMOVE) {

    validate_ptr(args[1]);

    struct thread* t = thread_current();
    char* file_name = (char*)args[1];

    lock_acquire(&filesys_lock);
    bool success = filesys_remove(file_name);
    lock_release(&filesys_lock);

    if (success) {
      f->eax = 1;
    } else {
      f->eax = 0;
    }

    return;
  }
  if (args[0] == SYS_FILESIZE) {
    struct thread* t = thread_current();
    int fd = args[1];
    int index = 3;

    struct list_elem* iter;
    for (iter = list_begin(&(t->pcb->fd_list.lst)); iter != list_end(&(t->pcb->fd_list.lst));
         iter = list_next(iter)) {

      struct file_descriptor* file_desc = list_entry(iter, struct file_descriptor, elem);

      if (index == fd && file_desc->file != NULL) {
        f->eax = file_length(file_desc->file);
        return;
      }

      index++;
    }

    f->eax = -1;
    return;
  }

  if (args[0] == SYS_READ) {
    validate_ptr(args[2]);
    struct thread* t = thread_current();
    int fd = args[1];
    void* buf = args[2];
    int32_t size = (int32_t)args[3];

    if (fd == 0) {
      char* std_buf = (char*)buf;
      int key;
      int i = 0;

      while (i < size && (key = input_getc()) != 0) {
        std_buf[i] = (char)key;
        i++;
      }

      f->eax = i;
      return;
    }

    if (fd == 1 || fd == 2 || fd < 0 || fd > MAX_FILES) {
      f->eax = -1;
      return;
    }

    if (fd > 2) {
      lock_acquire(&filesys_lock);
      struct list_elem* iter = list_begin(&(t->pcb->fd_list.lst));
      struct file_descriptor* curr_file = NULL;

      for (int i = 3; i <= fd && iter != list_end(&(t->pcb->fd_list.lst)); i++) {
        if (i == fd) {
          curr_file = list_entry(iter, struct file_descriptor, elem);
          break;
        }
        iter = list_next(iter);
      }

      if (curr_file != NULL && curr_file->file != NULL) {
        f->eax = file_read(curr_file->file, buf, size);
      } else {
        f->eax = -1;
      }
      lock_release(&filesys_lock);
      return;
    }

    f->eax = -1;
    return;
  }

  if (args[0] == SYS_WRITE) {
    //dont test arg1 for some reason..!! - Sarvagya
    validate_ptr(args[2]);

    struct thread* t = thread_current();
    int fd = args[1];
    char* buff = (char*)args[2];
    size_t sizeW = args[3];

    if (fd == 1) {
      putbuf(buff, sizeW);
      f->eax = sizeW;
      return;
    }

    lock_acquire(&filesys_lock);
    int index = 3;
    struct list_elem* iter;
    for (iter = list_begin(&(t->pcb->fd_list.lst)); iter != list_end(&(t->pcb->fd_list.lst));
         iter = list_next(iter)) {
      if (index == fd) {
        struct file_descriptor* file_desc = list_entry(iter, struct file_descriptor, elem);

        if (file_desc->file != NULL) {

          int bytes_written = file_write(file_desc->file, buff, sizeW);
          lock_release(&filesys_lock);
          f->eax = bytes_written;

          return;
        }
      }
      index++;
    }

    f->eax = -1;
    lock_release(&filesys_lock);
    return;
  }

  if (args[0] == SYS_SEEK) {
    bool success = false;
    int i = 3;
    int fd = args[1];
    int position = (int)args[2];
    struct thread* t = thread_current();
    struct list_elem* iter = list_begin(&(t->pcb->fd_list.lst));
    struct file_descriptor* file_desc;

    for (iter = list_begin(&(t->pcb->fd_list.lst)); iter != list_end(&(t->pcb->fd_list.lst));
         iter = list_next(iter)) {
      if (i == fd) {
        file_desc = list_entry(iter, struct file_descriptor, elem);
        success = true;
        break;
      }
      i++;
    }
    if (success) {
      struct file* file = file_desc->file;
      file_seek(file, position);
      f->eax = 0;
      return;
    }
    f->eax = -1;
    return;
  }

  if (args[0] == SYS_TELL) {
    int i = 3;
    int fd = args[1];
    bool success = false;
    struct thread* t = thread_current();
    struct list_elem* iter = list_begin(&(t->pcb->fd_list.lst));
    struct file_descriptor* file_desc;

    for (iter = list_begin(&(t->pcb->fd_list.lst)); iter != list_end(&(t->pcb->fd_list.lst));
         iter = list_next(iter)) {
      if (i == fd) {
        file_desc = list_entry(iter, struct file_descriptor, elem);
        success = true;
        break;
      }
      i++;
    }

    if (success) {
      struct file* file = file_desc->file;
      int pos = file_tell(file);
      f->eax = pos;
      return;
    }
    f->eax = -1;
    return;
  }

  if (args[0] == SYS_CLOSE) {
    lock_acquire(&filesys_lock);
    int i = 3;
    int fd = args[1];
    bool success = false;
    struct thread* t = thread_current();
    struct list_elem* iter = list_begin(&(t->pcb->fd_list.lst));
    struct file_descriptor* file_desc;

    for (iter = list_begin(&(t->pcb->fd_list.lst)); iter != list_end(&(t->pcb->fd_list.lst));
         iter = list_next(iter)) {
      if (i == fd) {
        file_desc = list_entry(iter, struct file_descriptor, elem);
        if (file_desc->file != NULL) {
          success = true;
        }
        break;
      }
      i++;
    }

    if (success) {
      struct file* file = file_desc->file;
      file_allow_write(file);
      file_close(file);
      file_desc->file = NULL;
      f->eax = 0;
      lock_release(&filesys_lock);
      return;
    }
    f->eax = -1;
    lock_release(&filesys_lock);
    return;
  }

  if (args[0] == SYS_PRACTICE) {
    f->eax = args[1] + 1;
    return;
  }

  if (args[0] == SYS_HALT) {
    shutdown_power_off();
  }

  if (args[0] == SYS_EXIT) {
    f->eax = args[1];
    printf("%s: exit(%d)\n", thread_current()->pcb->process_name, args[1]);
    process_exit(args[1]);
  }

  if (args[0] == SYS_WAIT) {
    struct thread* parent_thread = thread_current();
    struct process* parent = parent_thread->pcb;
    pid_t child_pid = args[1];
    f->eax = process_wait(child_pid);
    return;
  }

  if (args[0] == SYS_EXEC) {
    validate_ptr(args[1]);
    validate_string(args[1]);

    char* command = palloc_get_page(0);
    strlcpy(command, args[1], PGSIZE);

    char* save_ptr;
    char* filename = strtok_r(command, " ", &save_ptr);

    if (filename == NULL) {
      f->eax = -1;
      palloc_free_page(command);
      return;
    }

    lock_acquire(&filesys_lock);

    struct file* of = filesys_open(filename);
    if (of == NULL) {
      f->eax = -1;
      lock_release(&filesys_lock);
      palloc_free_page(command);
      return;
    }

    file_deny_write(of);

    struct thread* parent_thread = thread_current();
    pid_t child_pid = process_execute((char*)args[1]);
    if (child_pid == TID_ERROR) {
      f->eax = -1;
    } else {
      f->eax = child_pid;
    }
    file_allow_write(of);
    file_close(of);
    lock_release(&filesys_lock);

    return;
  }

  if (args[0] == SYS_COMPUTE_E) {
    // validate_ptr(args[1]);
    f->eax = sys_sum_to_e(args[1]);
    return;
  }
}
