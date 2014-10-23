#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "threads/synch.h"

struct process_file {
      struct file *file;
      int fd;
      struct list_elem elem;
};

struct mapping_file {
	struct file *file;
	unsigned int s_uaddress;
	unsigned int size;
	int map_id;
	struct list_elem elem;
};

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

// vm
int process_add_file(struct file * file_ptr);
struct file * process_get_file(int fd);
int process_close_file(int fd);
#ifdef VM
int process_map_file(struct file * file_ptr,unsigned int starting_pos, unsigned int size);
int process_unmap_file(int map_id);
#endif

bool install_page (void *upage, void *kpage, bool writable);

#endif /* userprog/process.h */
