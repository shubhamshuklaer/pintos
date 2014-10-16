#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "threads/synch.h"

struct process_file {
      struct file *file;
      int fd;
      struct list_elem elem;
};

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);
int process_add_file(struct file * file_ptr);
struct file * process_get_file(int fd);
int process_close_file(int fd);
bool install_page (void *upage, void *kpage, bool writable);

#endif /* userprog/process.h */
