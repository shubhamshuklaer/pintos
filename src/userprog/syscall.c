#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


// retrieve the system call number, then any system call arguments,
// and carry out appropriate actions
static void
syscall_handler (struct intr_frame *f )// UNUSED 
{
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
  // printf ("*eip : %d\n", f->eip);
  // printf ("cs : %d\n", f->cs);
  // printf ("eflags : %d\n", f->eflags);
  // printf ("*esp : %d\n", f->esp);
  // printf ("ss : %d\n", f->ss);

	int *ptr = f->esp;
  f->esp -= sizeof(int);
  printf("%d\n", *ptr);
  
  switch(ptr){
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
      printf ("unknown system call!\n");
      break;
  }

  

  // thread_exit ();
}

/* Invokes syscall NUMBER, passing no arguments, and returns the
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
  int *ptr = f->esp;
  

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
  int *ptr = f->esp;
  

}

/*

pid_t
exec (const char *file)
{
  return (pid_t) syscall1 (SYS_EXEC, file);
}

*/

void exec (struct intr_frame *f){
  int *ptr = f->esp;
  

}

/*

int
wait (pid_t pid)
{
  return syscall1 (SYS_WAIT, pid);
}

*/

void wait (struct intr_frame *f){
  int *ptr = f->esp;
  

}

/*

bool
create (const char *file, unsigned initial_size)
{
  return syscall2 (SYS_CREATE, file, initial_size);
}

*/

void create (struct intr_frame *f){
  int *ptr = f->esp;
  

}

/*

bool
remove (const char *file)
{
  return syscall1 (SYS_REMOVE, file);
}

*/

void remove (struct intr_frame *f){
  int *ptr = f->esp;
  

}

/*

int
open (const char *file)
{
  return syscall1 (SYS_OPEN, file);
}

*/

void open (struct intr_frame *f){
  int *ptr = f->esp;
  

}

/*

int
filesize (int fd) 
{
  return syscall1 (SYS_FILESIZE, fd);
}

*/

void filesize (struct intr_frame *f){
  int *ptr = f->esp;
  

}

/*

int
read (int fd, void *buffer, unsigned size)
{
  return syscall3 (SYS_READ, fd, buffer, size);
}

*/



void read (struct intr_frame *f){
  int *ptr = f->esp;
  int fd, const void *buffer, unsigned size;

}
/*

int
write (int fd, const void *buffer, unsigned size)
{
  return syscall3 (SYS_WRITE, fd, buffer, size);
}

*/

void write (struct intr_frame *f){
  int *ptr = f->esp;
  int fd, const void *buffer, unsigned size;


}
/*

void
seek (int fd, unsigned position) 
{
  syscall2 (SYS_SEEK, fd, position);
}

*/
void seek (struct intr_frame *f){
  int *ptr = f->esp;
  

}
/*

unsigned
tell (int fd) 
{
  return syscall1 (SYS_TELL, fd);
}

*/void tell (struct intr_frame *f){
  int *ptr = f->esp;
  

}
/*

oid
close (int fd)
{
  syscall1 (SYS_CLOSE, fd);
}

*/void close (struct intr_frame *f){
  int *ptr = f->esp;
  

}