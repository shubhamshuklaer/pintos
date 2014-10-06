#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/threads.h"
#include "threads/palloc.h"
#include "vm/page.h"
#include <hash.h>
struct frame_table_entry{
    void * k_vaddr;//kernel virtual address for the frame..!!
    void * u_vaddr;//user virtual address to which frame belongs..!!
    tid_t tid; 
    struct hash_elem hash_elem;
}

void frame_table_init();
void * vm_alloc_frame(void *u_vaddr);
void vm_free_frame(void *);
void vm_free_tid_frames(tid_t);


bool ft_less_func (const struct hash_elem *a,const struct hash_elem *b,void *aux);

unsigned ft_hash_func (const struct hash_elem *e, void *aux);

#endif //vm/frame.h
