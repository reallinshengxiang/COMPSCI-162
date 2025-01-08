#include "userprog/process.h"
#include <debug.h>
#include <float.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "userprog/syscall.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static thread_func start_process NO_RETURN;
static thread_func start_pthread NO_RETURN;
static bool load(const char* cmd_line, void (**eip)(void), void** esp);
bool setup_thread(void (**eip)(void), void** esp);

/* Data structure shared between process_execute() in the
   invoking thread and start_process() in the newly invoked
   thread. */
struct exec_info {
  const char* file_name;           /* Program to load. */
  struct semaphore load_done;      /* "Up"ed when loading complete. */
  struct wait_status* wait_status; /* Child process. */
  bool success;                    /* Program successfully loaded? */
};

/* Initializes user programs in the system by ensuring the main
   thread has a minimal PCB so that it can execute and wait for
   the first user process. Any additions to the PCB should be also
   initialized here if main needs those members */
void userprog_init(void) {
  struct thread* t = thread_current();
  bool success;

  /* Allocate process control block
     It is imoprtant that this is a call to calloc and not malloc,
     so that t->pcb->pagedir is guaranteed to be NULL (the kernel's
     page directory) when t->pcb is assigned, because a timer interrupt
     can come at any time and activate our pagedir */
  t->pcb = calloc(sizeof(struct process), 1);
  success = t->pcb != NULL;

  /* Main only needs a list of children */
  if (success)
    list_init(&t->pcb->children);

  /* Kill the kernel if we did not succeed */
  ASSERT(success);
}

/* Starts a new thread running a user program loaded from
   FILENAME.  The new thread may be scheduled (and may even exit)
   before process_execute() returns.  Returns the new process's
   process id, or TID_ERROR if the thread cannot be created. */
pid_t process_execute(const char* file_name) {
  struct exec_info exec;
  char thread_name[16];
  char* save_ptr;
  tid_t tid;

  /* Initialize exec_info. */
  exec.file_name = file_name;
  sema_init(&exec.load_done, 0);

  /* Create a new thread to execute FILE_NAME. */
  strlcpy(thread_name, file_name, sizeof thread_name);
  strtok_r(thread_name, " ", &save_ptr);
  tid = thread_create(thread_name, PRI_DEFAULT, start_process, &exec);
  if (tid != TID_ERROR) {
    sema_down(&exec.load_done);
    if (exec.success)
      list_push_back(&thread_current()->pcb->children, &exec.wait_status->elem);
    else
      tid = TID_ERROR;
  }

  return tid;
}

/* A thread function that loads a user process and starts it
   running. */
static void start_process(void* exec_) {
  struct thread* t = thread_current();

  struct exec_info* exec = exec_;
  struct intr_frame if_;
  uint32_t fpu_curr[27];
  bool success, pcb_success, ws_success;

  /* Allocate process control block */
  struct process* new_pcb = malloc(sizeof(struct process));
  success = pcb_success = new_pcb != NULL;

  /* Initialize process control block */
  if (success) {
    // Ensure that timer_interrupt() -> schedule() -> process_activate()
    // does not try to activate our uninitialized pagedir
    new_pcb->pagedir = NULL;
    t->pcb = new_pcb;

    // Continue initializing the PCB as normal
    list_init(&t->pcb->children);
    list_init(&t->pcb->fds);
    t->pcb->next_handle = 2;
    t->pcb->main_thread = t;
    strlcpy(t->pcb->process_name, t->name, sizeof t->name);
  }

  /* Allocate wait_status. */
  if (success) {
    exec->wait_status = t->pcb->wait_status = malloc(sizeof *exec->wait_status);
    success = ws_success = exec->wait_status != NULL;
  }

  /* Initialize wait_status. */
  if (success) {
    lock_init(&exec->wait_status->lock);
    exec->wait_status->ref_cnt = 2;
    exec->wait_status->pid = t->tid;
    exec->wait_status->exit_code = -1;
    sema_init(&exec->wait_status->dead, 0);
  }

  /* Initialize interrupt frame and load executable. */
  if (success) {
    memset(&if_, 0, sizeof if_);
    fpu_save_init(&if_.fpu, &fpu_curr);
    if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
    if_.cs = SEL_UCSEG;
    if_.eflags = FLAG_IF | FLAG_MBS;
    success = load(exec->file_name, &if_.eip, &if_.esp);
  }

  /* Handle failure with succesful PCB malloc. Must free the PCB */
  if (!success && pcb_success) {
    // Avoid race where PCB is freed before t->pcb is set to NULL
    // If this happens, then an unfortuantely timed timer interrupt
    // can try to activate the pagedir, but it is now freed memory
    struct process* pcb_to_free = t->pcb;
    t->pcb = NULL;
    free(pcb_to_free);
  }

  /* Handle failure with successful wait_status malloc */
  if (!success && ws_success)
    free(exec->wait_status);

  /* Notify parent thread and clean up. */
  exec->success = success;
  sema_up(&exec->load_done);
  if (!success)
    thread_exit();

  /* Start the user process by simulating a return from an
     interrupt, implemented by intr_exit (in
     threads/intr-stubs.S).  Because intr_exit takes all of its
     arguments on the stack in the form of a `struct intr_frame',
     we just point the stack pointer (%esp) to our stack frame
     and jump to it. */
  asm volatile("movl %0, %%esp; jmp intr_exit" : : "g"(&if_) : "memory");
  NOT_REACHED();
}

/* Releases one reference to CS and, if it is now unreferenced,
   frees it. */
static void release_child(struct wait_status* cs) {
  int new_ref_cnt;

  lock_acquire(&cs->lock);
  new_ref_cnt = --cs->ref_cnt;
  lock_release(&cs->lock);

  if (new_ref_cnt == 0)
    free(cs);
}

/* Waits for process with PID child_pid to die and returns its exit status.
   If it was terminated by the kernel (i.e. killed due to an
   exception), returns -1.  If child_pid is invalid or if it was not a
   child of the calling process, or if process_wait() has already
   been successfully called for the given PID, returns -1
   immediately, without waiting. */
int process_wait(pid_t child_pid) {
  struct thread* cur = thread_current();
  struct list_elem* e;

  for (e = list_begin(&cur->pcb->children); e != list_end(&cur->pcb->children); e = list_next(e)) {
    struct wait_status* cs = list_entry(e, struct wait_status, elem);
    if (cs->pid == child_pid) {
      int exit_code;
      list_remove(e);
      sema_down(&cs->dead);
      exit_code = cs->exit_code;
      release_child(cs);
      return exit_code;
    }
  }
  return -1;
}

/* Free the current process's resources. */
void process_exit(void) {
  struct thread* cur = thread_current();
  struct list_elem *e, *next;
  uint32_t* pd;

  /* If this thread does not have a PCB, don't worry */
  if (cur->pcb == NULL) {
    thread_exit();
    NOT_REACHED();
  }

  /* Close executable (and allow writes). */
  safe_file_close(cur->pcb->bin_file);

  /* Free entries of children list. */
  for (e = list_begin(&cur->pcb->children); e != list_end(&cur->pcb->children); e = next) {
    struct wait_status* cs = list_entry(e, struct wait_status, elem);
    next = list_remove(e);
    release_child(cs);
  }

  /* Close all currently open file descriptors */
  while (!list_empty(&cur->pcb->fds)) {
    e = list_begin(&cur->pcb->fds);
    struct file_descriptor* fd = list_entry(e, struct file_descriptor, elem);
    sys_close(fd->handle);
  }

  /* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  pd = cur->pcb->pagedir;
  if (pd != NULL) {
    /* Correct ordering here is crucial.  We must set
         cur->pcb->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
    cur->pcb->pagedir = NULL;
    pagedir_activate(NULL);
    pagedir_destroy(pd);
  }

  /* Notify parent that we're dead, as the last thing we do. */
  if (cur->pcb->wait_status != NULL) {
    struct wait_status* cs = cur->pcb->wait_status;
    printf("%s: exit(%d)\n", cur->pcb->process_name, cs->exit_code);
    sema_up(&cs->dead);
    release_child(cs);
  }

  /* Free the PCB of this process and kill this thread
     Avoid race where PCB is freed before t->pcb is set to NULL
     If this happens, then an unfortuantely timed timer interrupt
     can try to activate the pagedir, but it is now freed memory */
  struct process* pcb_to_free = cur->pcb;
  cur->pcb = NULL;
  free(pcb_to_free);
  thread_exit();
}

/* Sets up the CPU for running user code in the current
   thread. This function is called on every context switch. */
void process_activate(void) {
  struct thread* t = thread_current();

  /* Activate thread's page tables. */
  if (t->pcb != NULL && t->pcb->pagedir != NULL)
    pagedir_activate(t->pcb->pagedir);
  else
    pagedir_activate(NULL);

  /* Set thread's kernel stack for use in processing interrupts.
     This does nothing if this is not a user process. */
  tss_update();
}

/* We load ELF binaries.  The following definitions are taken
   from the ELF specification, [ELF1], more-or-less verbatim.  */

/* ELF types.  See [ELF1] 1-2. */
typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

/* For use with ELF types in printf(). */
#define PE32Wx PRIx32 /* Print Elf32_Word in hexadecimal. */
#define PE32Ax PRIx32 /* Print Elf32_Addr in hexadecimal. */
#define PE32Ox PRIx32 /* Print Elf32_Off in hexadecimal. */
#define PE32Hx PRIx16 /* Print Elf32_Half in hexadecimal. */

/* Executable header.  See [ELF1] 1-4 to 1-8.
   This appears at the very beginning of an ELF binary. */
struct Elf32_Ehdr {
  unsigned char e_ident[16];
  Elf32_Half e_type;
  Elf32_Half e_machine;
  Elf32_Word e_version;
  Elf32_Addr e_entry;
  Elf32_Off e_phoff;
  Elf32_Off e_shoff;
  Elf32_Word e_flags;
  Elf32_Half e_ehsize;
  Elf32_Half e_phentsize;
  Elf32_Half e_phnum;
  Elf32_Half e_shentsize;
  Elf32_Half e_shnum;
  Elf32_Half e_shstrndx;
};

/* Program header.  See [ELF1] 2-2 to 2-4.
   There are e_phnum of these, starting at file offset e_phoff
   (see [ELF1] 1-6). */
struct Elf32_Phdr {
  Elf32_Word p_type;
  Elf32_Off p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_Word p_filesz;
  Elf32_Word p_memsz;
  Elf32_Word p_flags;
  Elf32_Word p_align;
};

/* Values for p_type.  See [ELF1] 2-3. */
#define PT_NULL 0           /* Ignore. */
#define PT_LOAD 1           /* Loadable segment. */
#define PT_DYNAMIC 2        /* Dynamic linking info. */
#define PT_INTERP 3         /* Name of dynamic loader. */
#define PT_NOTE 4           /* Auxiliary info. */
#define PT_SHLIB 5          /* Reserved. */
#define PT_PHDR 6           /* Program header table. */
#define PT_STACK 0x6474e551 /* Stack segment. */

/* Flags for p_flags.  See [ELF3] 2-3 and 2-4. */
#define PF_X 1 /* Executable. */
#define PF_W 2 /* Writable. */
#define PF_R 4 /* Readable. */

static bool setup_stack(const char* cmd_line, void** esp);
static bool validate_segment(const struct Elf32_Phdr*, struct file*);
static bool load_segment(struct file* file, off_t ofs, uint8_t* upage, uint32_t read_bytes,
                         uint32_t zero_bytes, bool writable);

/* Loads an ELF executable from FILE_NAME into the current thread.
   Stores the executable's entry point into *EIP
   and its initial stack pointer into *ESP.
   Returns true if successful, false otherwise. */
bool load(const char* cmd_line, void (**eip)(void), void** esp) {
  struct thread* t = thread_current();
  char file_name[NAME_MAX + 2];
  struct Elf32_Ehdr ehdr;
  struct file* file = NULL;
  off_t file_ofs;
  bool success = false;
  char* cp;
  int i;

  /* Allocate and activate page directory. */
  t->pcb->pagedir = pagedir_create();
  if (t->pcb->pagedir == NULL)
    goto done;
  process_activate();

  /* Extract file_name from command line. */
  while (*cmd_line == ' ')
    cmd_line++;
  strlcpy(file_name, cmd_line, sizeof file_name);
  cp = strchr(file_name, ' ');
  if (cp != NULL)
    *cp = '\0';

  /* Open executable file. */
  t->pcb->bin_file = file = filesys_open(file_name);
  if (file == NULL) {
    printf("load: %s: open failed\n", file_name);
    goto done;
  }
  file_deny_write(file);

  /* Read and verify executable header. */
  if (file_read(file, &ehdr, sizeof ehdr) != sizeof ehdr ||
      memcmp(ehdr.e_ident, "\177ELF\1\1\1", 7) || ehdr.e_type != 2 || ehdr.e_machine != 3 ||
      ehdr.e_version != 1 || ehdr.e_phentsize != sizeof(struct Elf32_Phdr) || ehdr.e_phnum > 1024) {
    printf("load: %s: error loading executable\n", file_name);
    goto done;
  }

  /* Read program headers. */
  file_ofs = ehdr.e_phoff;
  for (i = 0; i < ehdr.e_phnum; i++) {
    struct Elf32_Phdr phdr;

    if (file_ofs < 0 || file_ofs > file_length(file))
      goto done;
    file_seek(file, file_ofs);

    if (file_read(file, &phdr, sizeof phdr) != sizeof phdr)
      goto done;
    file_ofs += sizeof phdr;
    switch (phdr.p_type) {
      case PT_NULL:
      case PT_NOTE:
      case PT_PHDR:
      case PT_STACK:
      default:
        /* Ignore this segment. */
        break;
      case PT_DYNAMIC:
      case PT_INTERP:
      case PT_SHLIB:
        goto done;
      case PT_LOAD:
        if (validate_segment(&phdr, file)) {
          bool writable = (phdr.p_flags & PF_W) != 0;
          uint32_t file_page = phdr.p_offset & ~PGMASK;
          uint32_t mem_page = phdr.p_vaddr & ~PGMASK;
          uint32_t page_offset = phdr.p_vaddr & PGMASK;
          uint32_t read_bytes, zero_bytes;
          if (phdr.p_filesz > 0) {
            /* Normal segment.
                     Read initial part from disk and zero the rest. */
            read_bytes = page_offset + phdr.p_filesz;
            zero_bytes = (ROUND_UP(page_offset + phdr.p_memsz, PGSIZE) - read_bytes);
          } else {
            /* Entirely zero.
                     Don't read anything from disk. */
            read_bytes = 0;
            zero_bytes = ROUND_UP(page_offset + phdr.p_memsz, PGSIZE);
          }
          if (!load_segment(file, file_page, (void*)mem_page, read_bytes, zero_bytes, writable))
            goto done;
        } else
          goto done;
        break;
    }
  }

  /* Set up stack. */
  if (!setup_stack(cmd_line, esp))
    goto done;

  /* Start address. */
  *eip = (void (*)(void))ehdr.e_entry;

  success = true;

done:
  /* We arrive here whether the load is successful or not. */
  return success;
}

/* load() helpers. */

static bool install_page(void* upage, void* kpage, bool writable);

/* Checks whether PHDR describes a valid, loadable segment in
   FILE and returns true if so, false otherwise. */
static bool validate_segment(const struct Elf32_Phdr* phdr, struct file* file) {
  /* p_offset and p_vaddr must have the same page offset. */
  if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK))
    return false;

  /* p_offset must point within FILE. */
  if (phdr->p_offset > (Elf32_Off)file_length(file))
    return false;

  /* p_memsz must be at least as big as p_filesz. */
  if (phdr->p_memsz < phdr->p_filesz)
    return false;

  /* The segment must not be empty. */
  if (phdr->p_memsz == 0)
    return false;

  /* The virtual memory region must both start and end within the
     user address space range. */
  if (!is_user_vaddr((void*)phdr->p_vaddr))
    return false;
  if (!is_user_vaddr((void*)(phdr->p_vaddr + phdr->p_memsz)))
    return false;

  /* The region cannot "wrap around" across the kernel virtual
     address space. */
  if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
    return false;

  /* Disallow mapping page 0.
     Not only is it a bad idea to map page 0, but if we allowed
     it then user code that passed a null pointer to system calls
     could quite likely panic the kernel by way of null pointer
     assertions in memcpy(), etc. */
  if (phdr->p_vaddr < PGSIZE)
    return false;

  /* It's okay. */
  return true;
}

/* Loads a segment starting at offset OFS in FILE at address
   UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
   memory are initialized, as follows:

        - READ_BYTES bytes at UPAGE must be read from FILE
          starting at offset OFS.

        - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.

   The pages initialized by this function must be writable by the
   user process if WRITABLE is true, read-only otherwise.

   Return true if successful, false if a memory allocation error
   or disk read error occurs. */
static bool load_segment(struct file* file, off_t ofs, uint8_t* upage, uint32_t read_bytes,
                         uint32_t zero_bytes, bool writable) {
  ASSERT((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT(pg_ofs(upage) == 0);
  ASSERT(ofs % PGSIZE == 0);

  file_seek(file, ofs);
  while (read_bytes > 0 || zero_bytes > 0) {
    /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
    size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
    size_t page_zero_bytes = PGSIZE - page_read_bytes;

    /* Get a page of memory. */
    uint8_t* kpage = palloc_get_page(PAL_USER);
    if (kpage == NULL)
      return false;

    /* Load this page. */
    if (file_read(file, kpage, page_read_bytes) != (int)page_read_bytes) {
      palloc_free_page(kpage);
      return false;
    }
    memset(kpage + page_read_bytes, 0, page_zero_bytes);

    /* Add the page to the process's address space. */
    if (!install_page(upage, kpage, writable)) {
      palloc_free_page(kpage);
      return false;
    }

    /* Advance. */
    read_bytes -= page_read_bytes;
    zero_bytes -= page_zero_bytes;
    upage += PGSIZE;
  }
  return true;
}

/* Reverse the order of the ARGC pointers to char in ARGV. */
static void reverse(int argc, char** argv) {
  for (; argc > 1; argc -= 2, argv++) {
    char* tmp = argv[0];
    argv[0] = argv[argc - 1];
    argv[argc - 1] = tmp;
  }
}

/* Pushes the SIZE bytes in BUF onto the stack in KPAGE, whose
   page-relative stack pointer is *OFS, and then adjusts *OFS
   appropriately.  The bytes pushed are rounded to a 32-bit
   boundary.

   If successful, returns a pointer to the newly pushed object.
   On failure, returns a null pointer. */
static void* push(uint8_t* kpage, size_t* ofs, const void* buf, size_t size) {
  size_t padsize = ROUND_UP(size, sizeof(uint32_t));
  if (*ofs < padsize)
    return NULL;

  *ofs -= padsize;
  memcpy(kpage + *ofs + (padsize - size), buf, size);
  return kpage + *ofs + (padsize - size);
}

/* Sets up command line arguments in KPAGE, which will be mapped
   to UPAGE in user space.  The command line arguments are taken
   from CMD_LINE, separated by spaces.  Sets *ESP to the initial
   stack pointer for the process. */
static bool init_cmd_line(uint8_t* kpage, uint8_t* upage, const char* cmd_line, void** esp) {
  size_t ofs = PGSIZE;
  char* const null = NULL;
  char* cmd_line_copy;
  char *karg, *saveptr;
  int argc;
  char** argv;
  static void* arguments[MAX_ARGS];

  /* Push command line string. */
  cmd_line_copy = push(kpage, &ofs, cmd_line, strlen(cmd_line) + 1);
  if (cmd_line_copy == NULL)
    return false;

  /* Parse command line into arguments */
  argc = 0;
  for (karg = strtok_r(cmd_line_copy, " ", &saveptr); karg != NULL;
       karg = strtok_r(NULL, " ", &saveptr)) {
    arguments[argc++] = upage + (karg - (char*)kpage);
  }

  // Insert padding to ensure the stack pointer will ultimately be 16-byte-aligned
  size_t alignment_adjustment =
      ((PGSIZE - ofs) + (argc + 1) * sizeof(char*) + sizeof(char**) + sizeof(int)) % 16;
  ofs -= 16 - alignment_adjustment;

  // Push sentinel null for argv[argc]
  if (push(kpage, &ofs, &null, sizeof null) == NULL)
    return false;

  // Push command line arguments
  for (int i = 0; i < argc; i++) {
    if (push(kpage, &ofs, arguments + i, sizeof(void**)) == NULL)
      return false;
  }

  /* Reverse the order of the command line arguments. */
  argv = (char**)(upage + ofs);
  reverse(argc, (char**)(kpage + ofs));

  /* Push argv, argc, "return address". */
  if (push(kpage, &ofs, &argv, sizeof argv) == NULL ||
      push(kpage, &ofs, &argc, sizeof argc) == NULL ||
      push(kpage, &ofs, &null, sizeof null) == NULL)
    return false;

  /* Set initial stack pointer. */
  *esp = upage + ofs;
  return true;
}

/* Create a minimal stack for T by mapping a page at the
   top of user virtual memory.  Fills in the page using CMD_LINE
   and sets *ESP to the stack pointer. */
static bool setup_stack(const char* cmd_line, void** esp) {
  uint8_t* kpage;
  bool success = false;

  kpage = palloc_get_page(PAL_USER | PAL_ZERO);
  if (kpage != NULL) {
    uint8_t* upage = ((uint8_t*)PHYS_BASE) - PGSIZE;
    if (install_page(upage, kpage, true))
      success = init_cmd_line(kpage, upage, cmd_line, esp);
    else
      palloc_free_page(kpage);
  }
  return success;
}

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails. */
static bool install_page(void* upage, void* kpage, bool writable) {
  struct thread* t = thread_current();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page(t->pcb->pagedir, upage) == NULL &&
          pagedir_set_page(t->pcb->pagedir, upage, kpage, writable));
}

/* Returns true if t is the main thread of the process p */
bool is_main_thread(struct thread* t, struct process* p) { return p->main_thread == t; }

/* Gets the PID of a process */
pid_t get_pid(struct process* p) { return (pid_t)p->main_thread->tid; }

/* Creates a new stack for the thread and sets up its arguments.
   Stores the thread's entry point into *EIP and its initial stack
   pointer into *ESP. Handles all cleanup if unsuccessful. Returns
   true if successful, false otherwise.

   This function will be implemented in Project 2: Multithreading. For
   now, it does nothing. You may find it necessary to change the
   function signature. */
bool setup_thread(void (**eip)(void) UNUSED, void** esp UNUSED) { return false; }

/* Starts a new thread with a new user stack running SF, which takes
   TF and ARG as arguments on its user stack. This new thread may be
   scheduled (and may even exit) before pthread_execute () returns.
   Returns the new thread's TID or TID_ERROR if the thread cannot
   be created properly.

   This function will be implemented in Project 2: Multithreading and
   should be similar to process_execute (). For now, it does nothing.
   */
tid_t pthread_execute(stub_fun sf UNUSED, pthread_fun tf UNUSED, void* arg UNUSED) { return -1; }

/* A thread function that creates a new user thread and starts it
   running. Responsible for adding itself to the list of threads in
   the PCB.

   This function will be implemented in Project 2: Multithreading and
   should be similar to start_process (). For now, it does nothing. */
static void start_pthread(void* exec_ UNUSED) {}

/* Waits for thread with TID to die, if that thread was spawned
   in the same process and has not been waited on yet. Returns TID on
   success and returns TID_ERROR on failure immediately, without
   waiting.

   This function will be implemented in Project 2: Multithreading. For
   now, it does nothing. */
tid_t pthread_join(tid_t tid UNUSED) { return -1; }

/* Free the current thread's resources. Most resources will
   be freed on thread_exit(), so all we have to do is deallocate the
   thread's userspace stack. Wake any waiters on this thread.

   The main thread should not use this function. See
   pthread_exit_main() below.

   This function will be implemented in Project 2: Multithreading. For
   now, it does nothing. */
void pthread_exit(void) {}

/* Only to be used when the main thread explicitly calls pthread_exit.
   The main thread should wait on all threads in the process to
   terminate properly, before exiting itself. When it exits itself, it
   must terminate the process in addition to all necessary duties in
   pthread_exit.

   This function will be implemented in Project 2: Multithreading. For
   now, it does nothing. */
void pthread_exit_main(void) {}
