  #include <stdio.h>
  #include <syscall-nr.h>
  #include "threads/interrupt.h"
  #include "threads/thread.h"
  #include "threads/malloc.h"
  #include "threads/vaddr.h"
  #include "threads/init.h"
  #include "threads/synch.h"
  #include "userprog/syscall.h"
  #include "userprog/process.h"
  #include "lib/string.h"
  #include "filesys/file.h"
  #include "filesys/filesys.h"
  #include <stdbool.h>
  #include "devices/disk.h"
  #include "userprog/pagedir.h"





  bool validate_user(const uint8_t *uaddr, size_t size);
  bool validate_kernel(const uint8_t *kaddr, size_t size);
  
  /* Reads a byte at user virtual address UADDR.
  UADDR must be below PHYS_BASE.
  Returns the byte value if successful, -1 if a segfault
  occurred. */
  static int
  get_user (const uint8_t *uaddr)
  {
    validate_user(uaddr, 1);
    // printf("getting from user address : %p\n", uaddr);
    int result;
    asm ("movl $1f, %0; movzbl %1, %0; 1:"
    : "=&a" (result) : "m" (*uaddr));
    return result;
  }

  bool validate_kernel(const uint8_t *kaddr, size_t size){
    // printf("address to be validated : %p\n", uaddr);
    if(!kaddr){
       // printf("\nvalidation failed 1\n");
       exit_on_error();
      return false;
    }
    void *ptr = kaddr;
    if(!is_kernel_vaddr(kaddr)){
      // printf("\nvalidation failed 2\n");
      exit_on_error();
      return false;
    }
    
    // printf("\nvalidation passed\n");
    return true;
  }

  bool validate_user(const uint8_t *uaddr, size_t size){
    // printf("address to be validated : %p\n", uaddr);
    bool valid = true;
    if(!uaddr){
       // printf("\nvalidation failed 1\n");
       //  thread_current()->exit_status=-1;
       // exit_on_error();
      return false;
    }
    void *ptr = uaddr;
    if(!is_user_vaddr(uaddr)){
      // printf("\nvalidation failed 2\n");
      // thread_current()->exit_status=-1;
      // exit_on_error();
      return false;
    }
    if(!is_user_vaddr(uaddr + size - 1)){
      // printf("\nvalidation failed 3\n");
      // thread_current()->exit_status=-1;
      // exit_on_error();
      return false;
    }
    if((uint8_t *)0x08048000 > uaddr  && 0x20 < uaddr){
      // printf("\nvalidation failed 4\n");
      // thread_current()->exit_status=-1;
      // exit_on_error();
      return false;
    }
    // printf("\nvalidation passed\n");
    return true;
  }


  void exit_on_error(void){
    printf ("%s: exit(%d)\n", thread_current()->name, -1);
    thread_exit();
  }

  
void * user_to_kernel_ptr(const void *vaddr){
  // TO DO: Need to check if all bytes within range are correct
  // for strings + buffers
  void * ptr=NULL;
  if(validate_user(vaddr,1))
    ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
  return ptr;
}


  static void syscall_handler (struct intr_frame *);

  void halt (struct intr_frame *f);
  void exit (struct intr_frame *f);
  void exec (struct intr_frame *f);
  void wait (struct intr_frame *f);
  void create (struct intr_frame *f);
  void remove (struct intr_frame *f);
  void open (struct intr_frame *f);
  void filesize (struct intr_frame *f);
  void read (struct intr_frame *f);
  void write (struct intr_frame *f);
  void seek (struct intr_frame *f);
  void tell (struct intr_frame *f);
  void close (struct intr_frame *f);

  syscall_init (void) 
  {
    intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  }


  // retrieve the system call number, then any system call arguments,
  // and carry out appropriate actions
  static void
  syscall_handler (struct intr_frame *f )// UNUSED 
  {
    // backing up registers

    // uint32_t edi_backup = f->edi ;               /* Saved EDI. */
    // uint32_t esi_backup = f->esi ;               /* Saved ESI. */
    // uint32_t ebp_backup = f->ebp ;               /* Saved EBP. */
    // uint32_t esp_dummy_backup = f->esp_dummy ;    /* Not used. */
    // uint32_t ebx_backup = f->ebx ;               /* Saved EBX. */
    // uint32_t edx_backup = f->edx ;               /* Saved EDX. */
    // uint32_t ecx_backup = f->ecx ;               /* Saved ECX. */
    // uint32_t eax_backup = f->eax ;               /* Saved EAX. */
    // uint16_t gs_backup = f->gs ;           /* Saved GS segment register. */
    // uint16_t fs_backup = f->fs ;           /* Saved FS segment register. */
    // uint16_t es_backup = f->es ;           /* Saved ES segment register. */
    // uint16_t ds_backup = f->ds ;           /* Saved DS segment register. */
    // uint16_t cs_backup = f->cs ;           /* Code segment for eip. */
    // uint16_t ss_backup = f->ss ;           /* Data segment for esp. */
    // uint32_t eflags_backup = f->eflags;            /* Saved CPU flags. */

    // print currently saved registers

    // printf ("system call!\n");
    // printf ("edi : %d\n", f->edi);
    // printf ("esi : %d\n", f->esi);
    // printf ("ebp : %d\n", f->ebp);
    // printf ("esp_dummy : %d\n", f->esp_dummy);
    // printf ("ebx : %d\n", f->ebx);
    // printf ("edx : %d\n", f->edx);
    // printf ("ecx : %d\n", f->ecx);
    // printf ("eax : %d\n", f->eax);
    // printf ("gs : %d\n", f->gs);
    // printf ("fs : %d\n", f->fs);
    // printf ("es : %d\n", f->es);
    // printf ("ds : %d\n", f->ds);
    // printf ("vec_no : %d\n", f->vec_no);
    // printf ("frame_pointer : %d\n", f->frame_pointer);
    // printf ("eip: %p\n", f->eip);
    // printf ("cs : %d\n", f->cs);
    // printf ("eflags : %d\n", f->eflags);
    // printf ("esp : %p\n", f->esp);
    // printf ("ss : %d\n", f->ss);

    // dump stack

    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");

    int * ptr = user_to_kernel_ptr(f->esp);
    // 
    if(!ptr){
      thread_current()->exit_status=-1;
      exit_on_error();    
    }

    
    // printf("ptr: %p\n", ptr);
    // printf("%d\n", *ptr);
    
    switch(*ptr){
      case SYS_HALT:
        halt(f);
        break;
      case SYS_EXIT:
        exit(f);
        break;
      case SYS_EXEC:
        exec(f);
        break;
      case SYS_WAIT:
        wait(f);
        break;
      case SYS_CREATE:
        create(f);
        break;
      case SYS_REMOVE:
        remove(f);
        break;
      case SYS_OPEN:
        open(f);
        break;
      case SYS_FILESIZE:
        filesize(f);
        break;
      case SYS_READ:
        read(f);
        break;
      case SYS_WRITE:
        write(f);
        break;
      case SYS_SEEK:
        seek(f);
        break;
      case SYS_TELL:
        tell(f);
        break;
      case SYS_CLOSE:
        close(f);
        break;
      default:
        // printf ("unknown system call!\n");
        exit_on_error();
        break;
    }

    
    // f->esp = f->eip;
    
    // thread_exit ();

    // restoring registers
    // f->edi = edi_backup ;               /* Saved EDI. */
    // f->esi = esi_backup ;               /* Saved ESI. */
    // f->ebp = ebp_backup ;               /* Saved EBP. */
    // f->esp_dummy = esp_dummy_backup ;   /* Not  used. */
    // f->ebx = ebx_backup ;               /* Saved EBX. */
    // f->edx = edx_backup ;               /* Saved EDX. */
    // f->ecx = ecx_backup ;               /* Saved ECX. */
    // f->eax = eax_backup ;               /* Saved EAX. */
    // f->gs = gs_backup ;           /* Saved GS segment register. */
    // f->fs = fs_backup ;           /* Saved FS segment register. */
    // f->es = es_backup ;           /* Saved ES segment register. */
    // f->ds = ds_backup ;
    // f->cs = cs_backup ;           /* Saved ES segment register. */
    // f->ss = ss_backup ;

    return;
  }

  



  /* 

    Invokes syscall NUMBER, passing no arguments, and returns the
     return value as an `int'. 
  #define syscall0(NUMBER)                                        \
          ({                                                      \
            int retval;                                           \
            asm volatile                                          \
              ("pushl %[number]; int $0x30; addl $4, %%esp"       \
                 : "=a" (retval)                                  \
                 : [number] "i" (NUMBER)                          \
                 : "memory");                                     \
            retval;                                               \
          })

   Invokes syscall NUMBER, passing argument ARG0, and returns the
     return value as an `int'. 
  #define syscall1(NUMBER, ARG0)                                           \
          ({                                                               \
            int retval;                                                    \
            asm volatile                                                   \
              ("pushl %[arg0]; pushl %[number]; int $0x30; addl $8, %%esp" \
                 : "=a" (retval)                                           \
                 : [number] "i" (NUMBER),                                  \
                   [arg0] "g" (ARG0)                                       \
                 : "memory");                                              \
            retval;                                                        \
          })

   Invokes syscall NUMBER, passing arguments ARG0 and ARG1, and
     returns the return value as an `int'. 
  #define syscall2(NUMBER, ARG0, ARG1)                            \
          ({                                                      \
            int retval;                                           \
            asm volatile                                          \
              ("pushl %[arg1]; pushl %[arg0]; "                   \
               "pushl %[number]; int $0x30; addl $12, %%esp"      \
                 : "=a" (retval)                                  \
                 : [number] "i" (NUMBER),                         \
                   [arg0] "g" (ARG0),                             \
                   [arg1] "g" (ARG1)                              \
                 : "memory");                                     \
            retval;                                               \
          })

   Invokes syscall NUMBER, passing arguments ARG0, ARG1, and
     ARG2, and returns the return value as an `int'. 
  #define syscall3(NUMBER, ARG0, ARG1, ARG2)                      \
          ({                                                      \
            int retval;                                           \
            asm volatile                                          \
              ("pushl %[arg2]; pushl %[arg1]; pushl %[arg0]; "    \
               "pushl %[number]; int $0x30; addl $16, %%esp"      \
                 : "=a" (retval)                                  \
                 : [number] "i" (NUMBER),                         \
                   [arg0] "g" (ARG0),                             \
                   [arg1] "g" (ARG1),                             \
                   [arg2] "g" (ARG2)                              \
                 : "memory");                                     \
            retval;                                               \
          })

  */

  /*

  void
  halt (void) 
  {
    syscall0 (SYS_HALT);
    NOT_REACHED ();
  }

  */
  void halt (struct intr_frame *f){
    // printf("%s\n", "halt syscall !");
    // hex_dump
    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");

    
    int *ptr = f->esp;
    ptr ++;


    power_off();

    return;
  }

  /*

  void
  exit (int status)
  {
    syscall1 (SYS_EXIT, status);
    NOT_REACHED ();
  }

  */
  void exit (struct intr_frame *f){
    // printf("%s\n", "exit syscall !");
    
    // hex_dump
    // printf("stack pointer : %p\n", f->esp);
    // printf("return pointer : %p\n", f->eip);
    
    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");
    // hex_dump(f->eip, f->eip, PHYS_BASE - (void *)f->eip, 1);

    
    int *ptr = f->esp;
    ptr ++;

    if(!validate_user(ptr, 1))
      exit_on_error();
    // retrieve status
    // printf("stack pointer : %p\n", ptr);
    int status = *ptr;
    ptr ++;

    // printf("exiting with status : %d\n", status);
    f->eax = status;
    // printf("yoohoo %d",status);
    // process termination message
    printf ("%s: exit(%d)\n", thread_current()->name, status);
    thread_current()->exit_status = status;
    
    thread_exit ();
    // printf("%s\n", "exit syscall finished!");
    return;
  }

  /*

  pid_t
  exec (const char *cmd_line)
  {
    return (pid_t) syscall1 (SYS_EXEC, cmd_line);
  }

  */

  void exec (struct intr_frame *f){
    // printf("%s\n", "exec syscall !");

    // printf("stack pointer : %p\n", f->esp);
    // printf("return pointer : %p\n", f->eip);


    // // hex_dump
    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");

    
    int *ptr = f->esp;
    ptr ++;

    // retrieve file
    if(!validate_user(ptr, 1))
      exit_on_error();
    const char *cmd_line = (char *)*ptr;
    ptr ++;


    cmd_line=user_to_kernel_ptr(cmd_line);
    if(!cmd_line){
      f->eax=-1;
      return;
    }
    int pid = process_execute(cmd_line);
    // thread_exit();
    // printf("sh %d\n",pid);
    // process_exit();
    f->eax = pid;
    // thread_listall();
    return;
  }

  /*

  int
  wait (pid_t pid)
  {
    return syscall1 (SYS_WAIT, pid);
  }

  */
  void wait (struct intr_frame *f){
    // printf("%s\n", "wait syscall !");
    // hex_dump
    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");

    
    int *ptr = f->esp;
    ptr ++;

    // retrieve pid
    if(!validate_user(ptr, 1))
      exit_on_error();
    int pid = *ptr;
    ptr ++;
    // printf("checking for thread alive\n %d",pid);
    // thread_listall();
    if(!thread_alive(pid)){
      // printf("thread not alive");
      f->eax= -1;
      return;
    }
    // printf("thread alive\n");
    int wait_status = process_wait(pid);
    // printf("wait status: %d\n", wait_status);
    f->eax = wait_status;
    // printf("done waiting\n");
    // thread_exit();
    return;
  }

  /*

  bool
  create (const char *file, unsigned initial_size)
  {
    return syscall2 (SYS_CREATE, file, initial_size);
  }

  */
  void create (struct intr_frame *f){
    // printf("%s\n", "create syscall !");
    // hex_dump
    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");

    
    int *ptr = f->esp;
    ptr ++;

    // retrieve file
    if(!validate_user(ptr, 1))
      exit_on_error();
    const char *file_name = (char *)*ptr;
    ptr ++;
    // printf("create 2\n");
    // retrieve initial_size 
    unsigned initial_size = *ptr;
    ptr ++;
    // printf("create name: %s\n", file_ptr);
    // printf("create size: %d\n", initial_size);
    // printf("file_ptr: %p \ndisksize: %p\n",file_ptr,(struct file *)(disk_size(filesys_disk)*DISK_SECTOR_SIZE));
    // printf("disk size sector : %d \n disk sector size : %d\n",disk_size(filesys_disk),DISK_SECTOR_SIZE);
    // if(file_ptr!=NULL){
    //   if(file_ptr <(struct file *) (disk_size(filesys_disk)*DISK_SECTOR_SIZE)){
    //     printf("pointer ok\n");
    //   }
    //   else{
    //     printf("file_ptr: %p \ndisksize: %p\n",file_ptr,(struct file *)(disk_size(filesys_disk)*DISK_SECTOR_SIZE));
    //     printf("disk overflow\n");
    //     f->eax = false;
    //     exit_on_error();
    //   }
    // }else{
    //   printf("null file pointer\n");
    //   f->eax = false;
    //   exit_on_error();
    // }
    file_name=user_to_kernel_ptr(file_name);
    if(!file_name){
        f->eax = false;
        exit_on_error();
    }

    
    // printf("file name: %s\n", file_name);

    lock_acquire(&filesys_lock);
    // printf("create 3\n");
    if(filesys_create (file_name,initial_size)){
      // printf("created\n");
       lock_release(&filesys_lock);
       f->eax=true;
    }else{ 
      // printf("could not create\n");
        lock_release(&filesys_lock);
        f->eax=false;
    }
    return;  
  }

  /*

  bool
  remove (const char *file)
  {
    return syscall1 (SYS_REMOVE, file);
  }

  */
  void remove (struct intr_frame *f){
    // hex_dump
    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");

    // printf("%s\n", "remove syscall !");
    int *ptr = f->esp;
    ptr ++;
    
    // retrieve file
    if(!validate_user(ptr, 1))
      exit_on_error();
    const char *file_name = (char *)*ptr;
    ptr += sizeof(char *);
    file_name=user_to_kernel_ptr(file_name);

    if(!file_name){
      f->eax=false;
      exit_on_error();
    }

    
    // printf("file name: %s\n", file_name);

    lock_acquire(&filesys_lock);
    if(filesys_remove (file_name)){
        lock_release(&filesys_lock);
        f->eax=true;
    }else{ 
        lock_release(&filesys_lock);
        f->eax=false;
    }
    return;
  }

  /*

  int
  open (const char *file)
  {
    return syscall1 (SYS_OPEN, file);
  }

  */
  void open (struct intr_frame *f){
    //hex_dump
    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");

    // printf("%s\n", "open syscall !");
    int *ptr = f->esp;
    ptr ++;
    
    // retrieve file
    if(!validate_user(ptr, 1))
      exit_on_error();
    const char *file_name = (char *)*ptr;
    ptr += sizeof(char *);

    file_name=user_to_kernel_ptr(file_name);
    if(!file_name){
      // printf("dfafdasf1");
      thread_current()->exit_status=-1;
      exit_on_error();
    }

    // printf("file_name ptr %p\n%s\n",file_name,file_name);
    // printf("file name: %s\n", file_name);

    lock_acquire(&filesys_lock);
    struct file * file_ptr=filesys_open(file_name);

    if(file_ptr!=NULL){
       lock_release(&filesys_lock);
       f->eax=process_add_file(file_ptr);
    }else{
       lock_release(&filesys_lock);
       f->eax=-1;
    } 
    return;
  }

  /*

  int
  filesize (int fd) 
  {
    return syscall1 (SYS_FILESIZE, fd);
  }

  */
  void filesize (struct intr_frame *f){
    // hex_dump
    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");

    // printf("%s\n", "filesize syscall !");

    int *ptr = f->esp;
    ptr ++;
    // retrieve fd
    if(!validate_user(ptr, 1))
      exit_on_error();
    int fd = *ptr;
    ptr ++;
    int file_size; 
    struct file *file_ptr;
    lock_acquire(&filesys_lock);
    file_ptr=process_get_file(fd);
    if(file_ptr==NULL){
       f->eax=-1;
    }else{
       f->eax=file_length(file_ptr);//form filesys/file.h     
    } 
    lock_release(&filesys_lock);
    return;
  }

  /*

  int
  read (int fd, void *buffer, unsigned size)
  {
    return syscall3 (SYS_READ, fd, buffer, size);
  }

  */
  void read (struct intr_frame *f){
    // hex_dump
    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");

    // printf("%s\n", "read syscall !");
    int *ptr = f->esp;
    ptr ++;

    // retrieve fd
    if(!validate_user(ptr, 1)){
      thread_current()->exit_status=-1;
      exit_on_error();
    }
    int fd = *ptr;
    ptr ++;

    // retrieve buffer
    if(!validate_user(ptr, 1)){
      thread_current()->exit_status=-1;
      exit_on_error();
    }
    char *buffer_ptr = (char *)*ptr;
    ptr ++;

    //retrieve size
    if(!validate_user(ptr, 1)){
      thread_current()->exit_status=-1;
      exit_on_error();
    }
    unsigned size = *ptr;
    ptr ++;
    ///////////////////////////////////////////////////////////////////////
    // validate user-provided buffer
    // buffer_ptr=user_to_kernel_ptr(buffer_ptr);



    if(!validate_user(buffer_ptr, size)){
      // printf("user validation failed\n");
      thread_current()->exit_status=-1;
      exit_on_error();
      return;
    }else{
      buffer_ptr=user_to_kernel_ptr(buffer_ptr);
      if(!buffer_ptr){
        thread_current()->exit_status=-1;
        exit_on_error();
      }
        
    }
    /////////////////////////////////////////////////////////////////////
    if(fd==STDIN_FILENO){
        int i;
        uint8_t * casted_buffer=(uint8_t *)buffer_ptr;
        for(i=0;i<size;i++){
            casted_buffer[i]=input_getc();
        }
        f->eax=size;
    }else if(fd==STDOUT_FILENO){
        f->eax=-1;//can't write to stdout
    }else{
        lock_acquire(&filesys_lock);
        struct file *file_ptr=process_get_file(fd);
        if(file_ptr==NULL){
            f->eax=-1;
        }else{
            int bytes=file_read(file_ptr,buffer_ptr,size);
            f->eax=bytes;
        }
        lock_release(&filesys_lock);
    }
    return;
  }

  /*

  int
  write (int fd, const void *buffer, unsigned size)
  {
    return syscall3 (SYS_WRITE, fd, buffer, size);
  }

  */
  void write (struct intr_frame *f){
    // hex_dump
    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");

    // printf("\n%s\n", "write syscall !");
    int *ptr = f->esp;
    ptr ++;
    // printf ("ptr : %p\n", ptr);
    // retrieve fd
    if(!validate_user(ptr, 1))
      exit_on_error();
    int fd = *ptr;
    ptr ++;
    // printf ("ptr : %p\n", ptr);

    // retrieve buffer
    if(!validate_user(ptr, 1))
      exit_on_error();
    char *buffer_ptr = (char *)*ptr;
    
    ptr ++;
    // printf ("ptr : %p\n", ptr);
    //retrieve size
    if(!validate_user(ptr, 1))
      exit_on_error();
    unsigned size = *ptr;
    ptr ++;

    // validate user-provided buffer
    if(!validate_user(buffer_ptr, size)){
      // printf("user validation failed\n");
      f->eax = 0;
      return;
    }else{
      buffer_ptr=user_to_kernel_ptr(buffer_ptr);
      if(!buffer_ptr)
        exit_on_error();
    }
    unsigned siz = size;
    if(siz){
      char *buffer = malloc(siz+1);
      memcpy(buffer, buffer_ptr, siz);
      
      // printf("size : %d\nfd : %d\n", size, fd);

      // write to console if fd==1
      if(fd == 1){
        // printf("writing to console\n");
        while(siz > 100){
          putbuf (buffer, 100);
          buffer += 100;
          siz -= 100;
        }
        if(siz)putbuf(buffer, siz);
        f->eax = size;
        return;
      }
      else if(fd == 0){
        //error - can't write to STDIN
        f->eax=-1;
      }else{
        lock_acquire(&filesys_lock);
        struct file *file_ptr=process_get_file(fd);
        if(file_ptr==NULL){
            f->eax=-1;
        }else{
            int bytes=file_write(file_ptr,buffer_ptr,size);
            f->eax=bytes;
        }
        lock_release(&filesys_lock);
        
      }
    }else{
      f->eax = 0;

    }


    // f->esp = ptr;
    // printf ("ptr : %p\n", ptr);
    // printf ("esp : %p\n", f->esp);
    
    // printf("%s\n", "write syscall finished!");
    // thread_exit();
    return;
  }

  /*

  void
  seek (int fd, unsigned position) 
  {
    syscall2 (SYS_SEEK, fd, position);
  }

  */
  void seek (struct intr_frame *f){
    // hex_dump
    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");

    // printf("%s\n", "seek syscall !");
    int *ptr = f->esp;
    ptr ++;

    // retrieve fd
    if(!validate_user(ptr, 1))
          exit_on_error();
    int fd = *ptr;
    ptr ++;

    // retrieve position 
    if(!validate_user(ptr, 1))
          exit_on_error();
    unsigned position = *ptr;
    ptr ++;
    
    lock_acquire(&filesys_lock);
    struct file *file_ptr=process_get_file(fd);
    if(file_ptr==NULL){
        f->eax=-1;
    }else{
        file_seek(file_ptr,position);
        f->eax=position;
    }
    lock_release(&filesys_lock);

    return;
  }

  /*

  unsigned
  tell (int fd) 
  {
    return syscall1 (SYS_TELL, fd);
  }

  */
  void tell (struct intr_frame *f){
    // hex_dump
    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");

    // printf("%s\n", "tell syscall !");
    int *ptr = f->esp;
    ptr ++;

    // retrieve fd
    if(!validate_user(ptr, 1))
      exit_on_error();
    int fd = *ptr;
    ptr ++;
    
    lock_acquire(&filesys_lock);
    struct file * file_ptr=process_get_file(fd);
    if(file_ptr==NULL){
        f->eax=-1;
    }else{
        off_t offset=file_tell(file_ptr);
        f->eax=offset;
    }
    lock_release(&filesys_lock);
    return;
  }

  /*

  void
  close (int fd)
  {
    syscall1 (SYS_CLOSE, fd);
  }

  */
  void close (struct intr_frame *f){
    // hex_dump
    // printf("\n-----------------------------------\n");
    // hex_dump(f->esp, f->esp, PHYS_BASE - f->esp, 1);
    // printf("\n-----------------------------------\n");

    // printf("%s\n", "close syscall !");
    int *ptr = f->esp;
    ptr ++;
    
    // retrieve fd
    if(!validate_user(ptr, 1))
      exit_on_error();
    int fd = *ptr;
    ptr ++;
    lock_acquire(&filesys_lock);
    f->eax=process_close_file(fd);
    lock_release(&filesys_lock);
    

    return;
  }
