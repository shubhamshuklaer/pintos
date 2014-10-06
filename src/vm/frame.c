#include "vm/frame.h"
#include "vm/page.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include <hash.h>
#include <stdbool.h>

//Frames form same tid will be in same bucket
//key is hash_elem
//hash is k_vaddr
static struct hash frame_table;

static struct lock frame_table_lock;

static bool add_frame_table_entry (void * k_vaddr,void * u_vaddr);
static bool remove_frame_table_entry (void * k_vaddr);
static struct vm_frame_table_entry * get_frame_table_entry(void * k_vaddr);

void frame_table_init(){
    hash_init(&frame_table,&ft_hash_func,&ft_less_func,NULL);
    lock_init(&frame_table_lock);
}


void * vm_alloc_frame(enum palloc_flags flags,void *u_vaddr){
    if(!is_user_vaddr(u_vaddr))
        return NULL;
    if(!(flags & PAL_USER))//we only allocate from user pool
        return NULL;
    void *frame;//kernel virtual address of frame
    frame=palloc_get_page(flags);
    if(frame!=NULL){
        if(!add_frame_table_entry(frame,u_vaddr))
            palloc_free_page(frame);
        frame=NULL;
    }
    return frame;    
}



void vm_free_frame(void *frame){
    remove_frame_table_entry(frame);
    palloc_free_page(frame);
}

void vm_free_tid_frames(tid_t){

}


static bool add_frame_table_entry (void * k_vaddr,void * u_vaddr){
    struct frame_table_entry *fte;
    fte=(struct frame_table_entry *)malloc(sizeof(frame_table_entry));
    if(fte==NULL)
        return false;
    struct thread *cur=thread_current();
    fte->tid=cur->tid;
    fte->k_vaddr=k_vaddr;
    fte->u_vaddr=u_vaddr;
    lock_acquire(&frame_table_lock);
    hash_insert(&frame_table,&fte->hash_elem);
    lock_release(&frame_table_lock);
    return true;
}


static bool remove_frame_table_entry(void *k_vaddr){
    lock_acquire(&frame_table_lock);
    struct vm_frame_table_entry *fte;
    fte=get_frame_table_entry(k_vaddr); 
    if(fte!=NULL)
        hash_elem=hash_delete(&frame_table,&fte->hash_elem);
    lock_release(&frame_table_lock);
    if(!fte)
        return false;
    if(!hash_elem)
        return false;
    else
        return true;
}   

static struct vm_frame_table_entry * get_frame_table_entry(void * k_vaddr){
    struct hash_elem *hash_elem;
    struct list *bucket;
    bucket=hash_find_bucket(&frame_table,unsigned(k_vaddr));
    struct list_elem *i;
    bool found=false;
    struct vm_frame_table_entry *fte;
    for (i = list_begin (bucket); i != list_end (bucket); i = list_next (i)){
        hash_elem = list_elem_to_hash_elem (i);
        fte=hash_entry(e,struct vm_frame_table_entry,hash_elem);
        if(fte->k_vaddr==k_vaddr){
            found=true;
            break;
        }
    }
    if(found)
       return fte;
    else
       return NULL; 

}

/* Computes and returns the hash value for hash element E, given
   auxiliary data AUX. */
unsigned ft_hash_func (const struct hash_elem *e, void *UNUSED){
   struct vm_frame_table_entry * fte=hash_entry(e,struct vm_frame_table_entry,hash_elem);
   return (unsigned)(fte->k_vaddr);
}

/* Compares the value of two hash elements A and B, given
   auxiliary data AUX.  Returns true if A is less than B, or
   false if A is greater than or equal to B. */
bool ft_less_func (const struct hash_elem *a,const struct hash_elem *b,void *UNUSED){
   struct vm_frame_table_entry * fte_a=hash_entry(a,struct vm_frame_table_entry,hash_elem);
   struct vm_frame_table_entry * fte_b=hash_entry(b,struct vm_frame_table_entry,hash_elem);
   if(fte_a->k_vaddr < fte_b->k_vaddr)
       return true;
   else
       return false;
}

