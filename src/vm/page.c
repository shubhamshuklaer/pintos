#include "vm/page.h"
#include <stdio.h>
#include <stdbool.h>
#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include <debug.h>


bool spte_install_fs(void * u_vaddr, struct file * f,off_t offset,
        uint32_t read_bytes, uint32_t zero_bytes,bool writable){
   struct thread * t;
   t=thread_current(); 
   struct supp_page_table_entry * spte=(struct supp_page_table_entry *)malloc(sizeof(struct supp_page_table_entry));
   if(spte==NULL)
       return false;
   spte->u_vaddr=u_vaddr;
   spte->writable=writable;
   spte->file=f;
   spte->offset=offset;
   spte->read_bytes=read_bytes;
   spte->zero_bytes=zero_bytes;
   spte->type=SPTE_FS;
   spte->is_loaded=false;
   spte->k_vaddr=NULL;
   spte->swap_page=-1;
   if(!hash_insert(&t->supp_page_table,spte))
        return true;
   else
        return false;   

}

void destroy_spte(struct hash_elem *e, void *aux UNUSED){
   struct supp_page_table_entry *spte=hash_entry(e,struct supp_page_table_entry,elem);
   if(spte->is_loaded){
       vm_free_frame(spte->k_vaddr);  
   }
   free(spte);
}

void free_process_resources(){
    struct thread *t;
    t=thread_current();
    hash_destroy(&t->supp_page_table,&destroy_spte); 
}

bool spt_less_func (const struct hash_elem *a,const struct hash_elem *b,void *aux){
   struct supp_page_table_entry * spte_a=hash_entry(a,struct supp_page_table_entry,elem);
   struct supp_page_table_entry * spte_b=hash_entry(b,struct supp_page_table_entry,elem);
   if(spte_a->u_vaddr < spte_b->u_vaddr)
       return true;
   else
       return false;

}



unsigned spt_hash_func (const struct hash_elem *e, void *aux){
   struct supp_page_table_entry * spte=hash_entry(e,struct supp_page_table_entry,elem);
   return hash_int((int)(spte->u_vaddr));
}
