#include "vm/frame.h"
#include "vm/page.h"
#include "vm/swap.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include <stdbool.h>
#include <stdio.h>
#include <debug.h>
#include "threads/vaddr.h"

//key is hash_elem
//hash is k_vaddr
static struct hash frame_table;

static struct lock frame_table_lock;
static struct lock frame_evict_lock;

static bool add_frame_table_entry (void * k_vaddr,void * u_vaddr);
static bool remove_table_entry (void * k_vaddr);
static struct frame_table_entry * get_frame_table_entry(void * k_vaddr);
static unsigned ft_hash_k_vaddr(void * k_vaddr);
static struct frame_table_entry * pick_evict_frame();

void frame_table_init(){
    hash_init(&frame_table,&ft_hash_func,&ft_less_func,NULL);
    lock_init(&frame_table_lock);
    lock_init(&frame_evict_lock);
}


void * vm_alloc_frame(enum palloc_flags flags,void *u_vaddr){
    if(!is_user_vaddr(u_vaddr))
        return NULL;
    if(!(flags & PAL_USER))//we only allocate from user pool
        return NULL;
    void *frame;//kernel virtual address of frame
    frame=palloc_get_page(flags);
    if(frame==NULL)
        frame=vm_evict_frame();
    if(frame!=NULL){
        if(!add_frame_table_entry(frame,u_vaddr)){
            palloc_free_page(frame);
            frame=NULL;
        }
    }
    return frame;    
}



void vm_free_frame(void *frame){
    remove_table_entry(frame);
    palloc_free_page(frame);
}


void frame_tick(){
    if(!frame_table_lock.holder){
        //external intrrupts cannot acquire loacks
        //no problem though as interrupt is disabled
        //we just need to check if anyone else is editing the table
        if(thread_current()->exit_lock.holder==NULL){ 
            if(lock_try_acquire(&thread_current()->exit_lock)){
                //so that the process in currently not in exit part
                //couldn't use lock_try_acqire directly because ASSERT(!lock_held_by_current_thread)  
                hash_apply(&frame_table,&update_accessed_history); 
                lock_release(&thread_current()->exit_lock);
            }   
        }
    }
}

void update_accessed_history(struct hash_elem *h,void *aux UNUSED){
    struct frame_table_entry *fte;
    fte=hash_entry(h,struct frame_table_entry,elem);
    if(fte==NULL||fte->owner==NULL||fte->owner->pagedir==NULL)
        return;
    int access_bit = (int)pagedir_is_accessed (fte->owner->pagedir, fte->u_vaddr);
    fte->access_history=fte->access_history/10+access_bit*100000000;
    pagedir_set_accessed(fte->owner->pagedir,fte->u_vaddr,false);
}

void * vm_evict_frame(){
    lock_acquire(&frame_evict_lock);
    struct frame_table_entry * fte_to_evict;
    void * frame_to_evict=NULL;
    fte_to_evict=pick_evict_frame();
    if(!fte_to_evict)
        PANIC("Cannot evict frame");
    struct supp_page_table_entry *spte;    
    bool is_dirty;
    spte=lookup_spt_of_thread(fte_to_evict->u_vaddr,fte_to_evict->owner);
    if(!spte){
        PANIC("spte_lookup_failed");
    }
    is_dirty=pagedir_is_dirty(fte_to_evict->owner->pagedir,fte_to_evict->u_vaddr);
    switch(spte->type){
        case SPTE_FS:
        case SPTE_MMAP:
        case SPTE_ZERO:
            if(is_dirty){
                spte->swap_page=swap_out(fte_to_evict->k_vaddr);
                spte->type=SPTE_SWAP;
            }
            break;
        case SPTE_SWAP:
            spte->swap_page=swap_out(fte_to_evict->k_vaddr);
            spte->type=SPTE_SWAP;
            break; 
        default:
            PANIC("Unknown spte");
    }
    spte->is_loaded=false;
    spte->k_vaddr=NULL;
    frame_to_evict=fte_to_evict->k_vaddr;
    remove_table_entry(frame_to_evict);
    lock_release(&frame_evict_lock);
    return frame_to_evict;
}

static struct frame_table_entry * pick_evict_frame(){
    int i,min_access=200000000;//max value is 10^8
    struct frame_table_entry *fte,*fte_to_evict=NULL;
    struct hash_elem *hash_elem;
    for (i = 0; i < frame_table.bucket_cnt; i++){
      //printf("in bucket %d\n",i);
      struct list *bucket = &frame_table.buckets[i];
      struct list_elem *elem, *next;
      for (elem = list_begin (bucket); elem != list_end (bucket); elem = next){
          next = list_next (elem);
          hash_elem = list_elem_to_hash_elem (elem);
          fte=hash_entry(hash_elem,struct frame_table_entry,elem);
          //printf("fte->k_vaddr %p\n",fte->k_vaddr);
          if(fte->access_history<min_access){
            min_access=fte->access_history;
            fte_to_evict=fte;
          }
      }
    }
    return fte_to_evict;

}

static bool add_frame_table_entry (void * k_vaddr,void * u_vaddr){
    struct frame_table_entry *fte;
    fte=(struct frame_table_entry *)malloc(sizeof(struct frame_table_entry));
    if(fte==NULL)
        return false;
    struct thread *cur=thread_current();
    fte->owner=cur;
    fte->k_vaddr=k_vaddr;
    fte->u_vaddr=u_vaddr;
    fte->access_history=0;
    lock_acquire(&frame_table_lock);
    hash_insert(&frame_table,&fte->elem);
    lock_release(&frame_table_lock);
    return true;
}


static bool remove_table_entry(void *k_vaddr){
    lock_acquire(&frame_table_lock);
    struct frame_table_entry *fte;
    struct hash_elem *elem;
    fte=get_frame_table_entry(k_vaddr); 
    if(fte!=NULL){
        pagedir_clear_page(thread_current()->pagedir,fte->u_vaddr); 
        elem=hash_delete(&frame_table,&fte->elem);
    }
    lock_release(&frame_table_lock);
    if(!fte)
        return false;
    if(!elem)
        return false;
    else
        return true;
}   

static struct frame_table_entry * get_frame_table_entry(void * k_vaddr){
    struct hash_elem *elem;
    struct list *bucket;
    bucket=hash_find_bucket(&frame_table,ft_hash_k_vaddr(k_vaddr));
    struct list_elem *i;
    bool found=false;
    struct frame_table_entry *fte;
    for (i = list_begin (bucket); i != list_end (bucket); i = list_next (i)){
        elem = list_elem_to_hash_elem (i);
        fte=hash_entry(elem,struct frame_table_entry,elem);
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
unsigned ft_hash_func (const struct hash_elem *e, void *aux){
   struct frame_table_entry * fte=hash_entry(e,struct frame_table_entry,elem);
   return (unsigned)(pg_no(fte->k_vaddr));
}

static unsigned ft_hash_k_vaddr(void * k_vaddr){
    return (unsigned)(pg_no(k_vaddr));
}

/* Compares the value of two hash elements A and B, given
   auxiliary data AUX.  Returns true if A is less than B, or
   false if A is greater than or equal to B. */
bool ft_less_func (const struct hash_elem *a,const struct hash_elem *b,void *aux){
   struct frame_table_entry * fte_a=hash_entry(a,struct frame_table_entry,elem);
   struct frame_table_entry * fte_b=hash_entry(b,struct frame_table_entry,elem);
   if(fte_a->k_vaddr < fte_b->k_vaddr)
       return true;
   else
       return false;
}
