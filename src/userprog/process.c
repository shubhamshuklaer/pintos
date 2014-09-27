  #include "userprog/process.h"
  #include <debug.h>
  #include <inttypes.h>
  #include <round.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <stdarg.h>
  #include <list.h>
  #include "userprog/gdt.h"
  #include "userprog/pagedir.h"
  #include "userprog/tss.h"
  #include "filesys/directory.h"
  #include "filesys/file.h"
  #include "filesys/filesys.h"
  #include "threads/flags.h"
  #include "threads/init.h"
  #include "threads/interrupt.h"
  #include "threads/palloc.h"
  #include "threads/malloc.h"
  #include "threads/thread.h"
  #include "threads/vaddr.h"
  #include "lib/string.h"

  static thread_func start_process NO_RETURN;
  static bool load (const char *cmdline, void (**eip) (void), void **esp);

  /* Starts a new thread running a user program loaded from
     FILENAME.  The new thread may be scheduled (and may even exit)
     before process_execute() returns.  Returns the new process's
     thread id, or TID_ERROR if the thread cannot be created. */
  tid_t
  process_execute (const char *cmdline) 
  {
    // printf("%s\n", "execute process");
    char *fn_copy;
    tid_t tid;
    
    /* Make a copy of FILE_NAME.
       Otherwise there's a race between the caller and load(). */
    fn_copy = palloc_get_page (0);
    if (fn_copy == NULL)
      return TID_ERROR;
    strlcpy (fn_copy, cmdline, PGSIZE);

    // printf("%s\n", "execute thread"); 
    /* Create a new thread to execute FILE_NAME. */
    tid = thread_create (cmdline, PRI_DEFAULT+1, start_process, fn_copy);
    
    // printf("%s\n", "created thread"); 
    if (tid == TID_ERROR){
      palloc_free_page (fn_copy);  
    }else{
      
    }
    // start_process(fn_copy);
    // printf("%s\n", "exiting process");
    return tid;
  }

  /* A thread function that loads a user process and starts it
     running. */
  static void
  start_process (void *cmdline_)
  {
    // ASSERT(1==0); 
    char *cmdline = cmdline_;
    printf("\nnew process : '%s'\n", (char *)cmdline);
    struct intr_frame if_;
    bool success;

    /* Initialize interrupt frame and load executable. */
    memset (&if_, 0, sizeof if_);
    if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
    if_.cs = SEL_UCSEG;
    if_.eflags = FLAG_IF | FLAG_MBS;
    success = load (cmdline, &if_.eip, &if_.esp);
    // printf("eip : %p, esp: %p\n", &if_.eip, &if_.esp);

    

    /* If load failed, quit. */
    
    if (!success){
      // printf("%s\n", "not successful");
      thread_exit ();
      // printf("%s\n", "not at all successful");
    }else{
      printf("\nnew process : '%s'\n", (char *)cmdline);
      // printf("%s\n", "successful");
    }

    palloc_free_page (cmdline);
    /* Start the user process by simulating a return from an
       interrupt, implemented by intr_exit (in
       threads/intr-stubs.S).  Because intr_exit takes all of its
       arguments on the stack in the form of a `struct intr_frame',
       we just point the stack pointer (%esp) to our stack frame
       and jump to it. */
    asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
    // printf("\nreached\n");
    NOT_REACHED ();
  }

  /* Waits for thread TID to die and returns its exit status.  If
     it was terminated by the kernel (i.e. killed due to an
     exception), returns -1.  If TID is invalid or if it was not a
     child of the calling process, or if process_wait() has already
     been successfully called for the given TID, returns -1
     immediately, without waiting.

     This function will be implemented in problem 2-2.  For now, it
     does nothing. 

    Process executed as : process_wait (process_execute (task));

     */
  int
  process_wait (tid_t child_tid )// UNUSED 
  {
    printf("running thread name : %s\t; and id : %d\n", thread_name(), thread_tid());
    printf("child thread id : %d\n", child_tid);
    
    struct thread *current_thread = thread_current();
    printf("process wait 1\n");
    if(list_empty(&current_thread->child_procs))return -1;
    printf("process wait 2\n");
    struct list_elem *e;

    for (e = list_begin (&current_thread->child_procs); e != list_end (&current_thread->child_procs);e = list_next (e)){
      struct thread *ptr = list_entry (e, struct thread, child_proc);
      // ...do something with f...
      if(ptr->tid == child_tid){
        process_wait(child_tid);
        break;
      }
    }

    
    return -1;
  }

  /* Free the current process's resources. */
  void
  process_exit (void)
  {
    printf("exiting process\n");
    struct thread *cur = thread_current ();
    printf("exiting process with name : '%s'\tid: %d\n", thread_current ()->name, thread_current ()->tid);

    int child_tid = thread_tid();
    
    struct thread *child_thread = thread_current();
    printf("child thread to be exited: '%s'\n", child_thread->name);
    struct thread *parent_thread = child_thread->parent;
    if(parent_thread){
      printf("parent thread: '%s'\n", parent_thread->name);
      int i;
      for (i = 0; i < parent_thread->num_child_procs; ++i)
      {
        printf("child no. : %d\n", i);
        if(list_front(&parent_thread->child_procs) == &child_thread->child_proc){
          printf("removing process with tid: %d\t, from child_list of tid: %d\n", child_thread->tid, parent_thread->tid);
          list_pop_front(&parent_thread->child_procs);
          parent_thread->num_child_procs--;
          printf("process removed from '%s'\n", parent_thread->name);
          break;
        }
        else{
          list_push_back(&parent_thread->child_procs, list_pop_front(&parent_thread->child_procs));
        }
      }
    }
    
    

    uint32_t *pd;

    /* Destroy the current process's page directory and switch back
       to the kernel-only page directory. */
    pd = cur->pagedir;
    if (pd != NULL) 
      {
        /* Correct ordering here is crucial.  We must set
           cur->pagedir to NULL before switching page directories,
           so that a timer interrupt can't switch back to the
           process page directory.  We must activate the base page
           directory before destroying the process's page
           directory, or our active page directory will be one
           that's been freed (and cleared). */
        cur->pagedir = NULL;
        pagedir_activate (NULL);
        pagedir_destroy (pd);
      }
      printf("process exited\n");
  }

  /* Sets up the CPU for running user code in the current
     thread.
     This function is called on every context switch. */
  void
  process_activate (void)
  {
    struct thread *t = thread_current ();

    /* Activate thread's page tables. */
    pagedir_activate (t->pagedir);

    /* Set thread's kernel stack for use in processing
       interrupts. */
    tss_update ();
  }
  
  /* We load ELF binaries.  The following definitions are taken
     from the ELF specification, [ELF1], more-or-less verbatim.  */

  /* ELF types.  See [ELF1] 1-2. */
  typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
  typedef uint16_t Elf32_Half;

  /* For use with ELF types in printf(). */
  #define PE32Wx PRIx32   /* Print Elf32_Word in hexadecimal. */
  #define PE32Ax PRIx32   /* Print Elf32_Addr in hexadecimal. */
  #define PE32Ox PRIx32   /* Print Elf32_Off in hexadecimal. */
  #define PE32Hx PRIx16   /* Print Elf32_Half in hexadecimal. */

  /* Executable header.  See [ELF1] 1-4 to 1-8.
     This appears at the very beginning of an ELF binary. */
  struct Elf32_Ehdr
    {
      unsigned char e_ident[16];
      Elf32_Half    e_type;
      Elf32_Half    e_machine;
      Elf32_Word    e_version;
      Elf32_Addr    e_entry;
      Elf32_Off     e_phoff;
      Elf32_Off     e_shoff;
      Elf32_Word    e_flags;
      Elf32_Half    e_ehsize;
      Elf32_Half    e_phentsize;
      Elf32_Half    e_phnum;
      Elf32_Half    e_shentsize;
      Elf32_Half    e_shnum;
      Elf32_Half    e_shstrndx;
    };

  /* Program header.  See [ELF1] 2-2 to 2-4.
     There are e_phnum of these, starting at file offset e_phoff
     (see [ELF1] 1-6). */
  struct Elf32_Phdr
    {
      Elf32_Word p_type;
      Elf32_Off  p_offset;
      Elf32_Addr p_vaddr;
      Elf32_Addr p_paddr;
      Elf32_Word p_filesz;
      Elf32_Word p_memsz;
      Elf32_Word p_flags;
      Elf32_Word p_align;
    };

  /* Values for p_type.  See [ELF1] 2-3. */
  #define PT_NULL    0            /* Ignore. */
  #define PT_LOAD    1            /* Loadable segment. */
  #define PT_DYNAMIC 2            /* Dynamic linking info. */
  #define PT_INTERP  3            /* Name of dynamic loader. */
  #define PT_NOTE    4            /* Auxiliary info. */
  #define PT_SHLIB   5            /* Reserved. */
  #define PT_PHDR    6            /* Program header table. */
  #define PT_STACK   0x6474e551   /* Stack segment. */

  /* Flags for p_flags.  See [ELF3] 2-3 and 2-4. */
  #define PF_X 1          /* Executable. */
  #define PF_W 2          /* Writable. */
  #define PF_R 4          /* Readable. */

  static bool setup_stack (void **esp, const char *cmdline);
  static bool validate_segment (const struct Elf32_Phdr *, struct file *);
  static bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
                            uint32_t read_bytes, uint32_t zero_bytes,
                            bool writable);

  /* Loads an ELF executable from FILE_NAME into the current thread.
     Stores the executable's entry point into *EIP
     and its initial stack pointer into *ESP.
     Returns true if successful, false otherwise. */
  bool
  load (const char *cmdline, void (**eip) (void), void **esp) 
  {
    // printf("\nload entered : %s\n", cmdline);
    struct thread *t = thread_current ();
    struct Elf32_Ehdr ehdr;
    struct file *file = NULL;
    off_t file_ofs;
    bool success = false;
    int i;

    /* Allocate and activate page directory. */
    t->pagedir = pagedir_create ();
    if (t->pagedir == NULL) 
      goto done;
    process_activate ();

    /* Open executable file. */
    char *file_name, *save_ptr;
    char *cmdline_copy = malloc(strlen(cmdline)+1);
    strlcpy(cmdline_copy, cmdline, strlen(cmdline)+1);
    file_name = strtok_r ((char *)cmdline, " ,;", &save_ptr);
    // printf("%s\nname of file : \n%s\n", file_name, file_name);
    file = filesys_open (file_name);
    if (file == NULL) 
      {
        printf ("load: %s: open failed\n", file_name);
        goto done; 
      }

    /* Read and verify executable header. */
    if (file_read (file, &ehdr, sizeof ehdr) != sizeof ehdr
        || memcmp (ehdr.e_ident, "\177ELF\1\1\1", 7)
        || ehdr.e_type != 2
        || ehdr.e_machine != 3
        || ehdr.e_version != 1
        || ehdr.e_phentsize != sizeof (struct Elf32_Phdr)
        || ehdr.e_phnum > 1024) 
      {
        printf ("load: %s: error loading executable\n", file_name);
        goto done; 
      }

    // printf("%s\n", "reading program headers");
    /* Read program headers. */
    file_ofs = ehdr.e_phoff;
    for (i = 0; i < ehdr.e_phnum; i++) 
    {
      // printf("%d\n", i);
      struct Elf32_Phdr phdr;

      if (file_ofs < 0 || file_ofs > file_length (file))
        goto done;
      // printf("%s\n", "seeking file");
      file_seek (file, file_ofs);

      // printf("%s\n", "reading file");
      if (file_read (file, &phdr, sizeof phdr) != sizeof phdr)
        goto done;
      file_ofs += sizeof phdr;
      // printf("%s\n", "reading type");
      switch (phdr.p_type) 
        {
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
          if (validate_segment (&phdr, file)) 
          {
            // printf("%s\n", "segment validated");
            bool writable = (phdr.p_flags & PF_W) != 0;
            uint32_t file_page = phdr.p_offset & ~PGMASK;
            uint32_t mem_page = phdr.p_vaddr & ~PGMASK;
            uint32_t page_offset = phdr.p_vaddr & PGMASK;
            uint32_t read_bytes, zero_bytes;
            if (phdr.p_filesz > 0)
              {
                /* Normal segment.
                   Read initial part from disk and zero the rest. */
                read_bytes = page_offset + phdr.p_filesz;
                zero_bytes = (ROUND_UP (page_offset + phdr.p_memsz, PGSIZE)
                              - read_bytes);
              }
            else 
              {
                // printf("%s\n", "Entirely zero");
                /* Entirely zero.
                   Don't read anything from disk. */
                read_bytes = 0;
                zero_bytes = ROUND_UP (page_offset + phdr.p_memsz, PGSIZE);
              }
            if (!load_segment (file, file_page, (void *) mem_page,
                               read_bytes, zero_bytes, writable)){
              // printf("%s\n", "failed loading segment");
              goto done;
            }else{
              // printf("%s\n", "passed loading segment");
            }
          }
          else{
            // printf("%s\n", "segment not validated");
            goto done;
          }
          break;
        
        }
    }

    // printf("\nsetting up stack : %s\n", cmdline_copy);
    /* Set up stack. */
    if (!setup_stack (esp, cmdline_copy))
      goto done;

    /* Start address. */
    *eip = (void (*) (void)) ehdr.e_entry;

    success = true;

   done:
    // printf("%s\n", "done");
    /* We arrive here whether the load is successful or not. */
    file_close (file);
    // printf("%s\n", "return success");
    return success;
  }
  
  /* load() helpers. */

  static bool install_page (void *upage, void *kpage, bool writable);

  /* Checks whether PHDR describes a valid, loadable segment in
     FILE and returns true if so, false otherwise. */
  static bool
  validate_segment (const struct Elf32_Phdr *phdr, struct file *file) 
  {
    // printf("%s\n", "1");
    /* p_offset and p_vaddr must have the same page offset. */
    if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK)) 
      return false; 

    // printf("%s\n", "2");
    /* p_offset must point within FILE. */
    if (phdr->p_offset > (Elf32_Off) file_length (file)) 
      return false;

    // printf("%s\n", "3");
    /* p_memsz must be at least as big as p_filesz. */
    if (phdr->p_memsz < phdr->p_filesz) 
      return false; 

    // printf("%s\n", "4");
    /* The segment must not be empty. */
    if (phdr->p_memsz == 0)
      return false;
    
    // printf("%s\n", "5");
    /* The virtual memory region must both start and end within the
       user address space range. */
    if (!is_user_vaddr ((void *) phdr->p_vaddr))
      return false;

    // printf("%s\n", "6");
    if (!is_user_vaddr ((void *) (phdr->p_vaddr + phdr->p_memsz)))
      return false;

    // printf("%s\n", "7");
    /* The region cannot "wrap around" across the kernel virtual
       address space. */
    if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
      return false;

    // printf("\n%d : %d\n", phdr->p_vaddr, PGSIZE);
    /* Disallow mapping page 0.
       Not only is it a bad idea to map page 0, but if we allowed
       it then user code that passed a null pointer to system calls
       could quite likely panic the kernel by way of null pointer
       assertions in memcpy(), etc. */
    if (phdr->p_vaddr < PGSIZE);
      // return false;

    // printf("%s\n", "9");
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
  static bool
  load_segment (struct file *file, off_t ofs, uint8_t *upage,
                uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
  {
    ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
    ASSERT (pg_ofs (upage) == 0);
    ASSERT (ofs % PGSIZE == 0);

    // printf("%s\n", "seeking file");
    file_seek (file, ofs);
    while (read_bytes > 0 || zero_bytes > 0) 
      {
        // printf("%s\n", "infi");
        /* Calculate how to fill this page.
           We will read PAGE_READ_BYTES bytes from FILE
           and zero the final PAGE_ZERO_BYTES bytes. */
        size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
        size_t page_zero_bytes = PGSIZE - page_read_bytes;

        /* Get a page of memory. */
        uint8_t *kpage = palloc_get_page (PAL_USER);
        if (kpage == NULL)
          return false;

        /* Load this page. */
        if (file_read (file, kpage, page_read_bytes) != (int) page_read_bytes)
          {
            palloc_free_page (kpage);
            return false; 
          }
        memset (kpage + page_read_bytes, 0, page_zero_bytes);

        /* Add the page to the process's address space. */
        if (!install_page (upage, kpage, writable)) 
          {
            palloc_free_page (kpage);
            return false; 
          }

        /* Advance. */
        read_bytes -= page_read_bytes;
        zero_bytes -= page_zero_bytes;
        upage += PGSIZE;
      }
    return true;
  }

  /* Create a minimal stack by mapping a zeroed page at the top of
     user virtual memory. */
  // memcpy (void *dst_, const void *src_, size_t size) ;
  static bool
  setup_stack (void **esp, const char *cmdline) 
  {
    // printf("\nsetup stack with cmdline : %s\n", cmdline);
    
    uint8_t *kpage;
    bool success = false;

    kpage = palloc_get_page (PAL_USER | PAL_ZERO);
    // printf("%s\n", "got page");
    if (kpage != NULL) {
      success = install_page (((uint8_t *) PHYS_BASE) - PGSIZE, kpage, true);
      if (success){
        // printf("%s\n", "building stack");
        *esp = PHYS_BASE;
        char *arg, *save_ptr;
        int argc = 0, def_argc = 2, cmdline_len = strlen(cmdline), cmdline_ptr;
        char **argv = malloc( sizeof(char *)*def_argc*2);
        char *cmdline_copy = malloc(strlen(cmdline)+1);
        strlcpy(cmdline_copy, cmdline, strlen(cmdline)+1);

        //  ignore trailing spaces
        cmdline_ptr = cmdline_len-1;
        while(cmdline_copy[cmdline_ptr--]==' ')cmdline_len--;
        // ignore multiple white-spaces
        while(cmdline_ptr){
          if(cmdline_copy[cmdline_ptr]==' '){
            if(cmdline_copy[cmdline_ptr-1]==' ')cmdline_len--;
          }
          cmdline_ptr--;
        }

        // printf("%s\n", "building stack 2");
        *esp = PHYS_BASE - cmdline_len - 1;
        for (arg = strtok_r ((char *)cmdline, " ", &save_ptr); arg != NULL;arg = strtok_r (NULL, " ", &save_ptr)){
          
          // store pointers to arguments
          argv[argc++] = *esp;
          
          // printf("%d argument %s stack pointer %d\n", argc-1, arg,PHYS_BASE - *esp);
          // push arguments to the stack
          memcpy (*esp, arg, strlen(arg)+1);
          *esp += strlen(arg)+1;
          // resize argv if required
          if(argc > def_argc){
            def_argc*=2;
            argv = (char **)realloc(argv, sizeof(char *)*def_argc*2);
          }
        }
        // printf("\nbuilding stack 3 : %s\n", cmdline_copy);
        argv[argc] = 0;
        // printf("%s\n", "building stack 3.2");
        *esp = PHYS_BASE - cmdline_len - 1;
        // hex_dump(*esp, *esp, PHYS_BASE - *esp, 1); 
        // word-align
        int i = ((size_t)*esp) % 4;
        // printf("%d\n",i);
        // printf("%s\n", "building stack 3.5");
        if(i){
          *esp -= i;  
          memcpy(*esp, &argv[argc], i);
        }
        // printf("%s\n", "building stack 3.8");
        // push address of arguments
        int num_args = argc;
        while(num_args > 0){
          *esp -= sizeof(char *);
          memcpy (*esp, &argv[num_args], sizeof(char *));
          num_args--;
        }
        // printf("%s\n", "building stack 4");

        //push argv[0]
        *esp -= sizeof(char *);
        memcpy (*esp, &argv[0], sizeof(char *));

        arg = *esp;
        // push argv
        *esp -= sizeof(char **);
        memcpy (*esp, &arg, sizeof(char **));
        
        // printf("\narc: %d\n", argc);
        // push argc
        *esp -= sizeof(int);
        memcpy (*esp, &argc, sizeof(int));
        // printf("%s\n", "building stack 5");
        // push return address
        *esp -= sizeof(void *);
        memcpy (*esp, &argv[argc], sizeof(void *));
        // printf("%s\n", "building stack 6");
        num_args = argc;
        // printf("%s\n", "freeing argv");
       
        free(argv);
      
      // printf("%s\n", "building stack 8");
      }else{
        palloc_free_page (kpage);
        // printf("%s\n", "building stack 9");
      }
    }
    // printf("\n\n%s\n\n", "dumping");
    // hex_dump(*esp, *esp, PHYS_BASE - *esp, 1);

    // printf("total stack size : %d", PHYS_BASE - *esp);
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
  static bool
  install_page (void *upage, void *kpage, bool writable)
  {
    struct thread *t = thread_current ();

    /* Verify that there's not already a page at that virtual
       address, then map our page there. */
    return (pagedir_get_page (t->pagedir, upage) == NULL
            && pagedir_set_page (t->pagedir, upage, kpage, writable));
  }


  int process_add_file(struct file *file_ptr){
    struct process_file *pf=(struct process_file *)malloc(sizeof(struct process_file));
    struct thread *t=thread_current();
    if(pf==NULL)
     return -1;
    pf->file=file_ptr;
    pf->fd=t->fd;
    t->fd++;
    list_push_back(&(t->file_list),pf->elem);
    return pf->fd;

  }

struct file * process_get_file(int fd){
    struct file * file_ptr;
    struct list_elem e;
    struct thread *t=thread_current();
    struct process_file * pf;
    for(e=list_begin(&t->file_list),e!=list_end(&t->file_list),e=list_next(e)){
        pf=list_entry(e,struct process_file,elem);
        if(pf->fd==fd)
            return pf->file;   
    }
    return NULL;
}

int process_close_file(int fd){
    struct file * file_ptr;
    struct list_elem e;
    struct thread *t=thread_current();
    struct process_file * pf;
    for(e=list_begin(&t->file_list),e!=list_end(&t->file_list),e=list_next(e)){
        pf=list_entry(e,struct process_file,elem);
        if(pf->fd==fd){
            file_close(pf->file);
            list_remove(&(pf->elem));
            return fd;
        }   
    }
    return -1;
}
