#include "vm/page.h"
#include <stdio.h>
#include <stdbool.h>
#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include <debug.h>
#include "filesys/file.h"
#include "userprog/process.h"
#include <string.h>

static unsigned spt_hash_u_vaddr(void * u_vaddr);


bool spte_install_fs(void * u_vaddr, char  * file_name,off_t offset,
        uint32_t read_bytes, uint32_t zero_bytes,bool writable){
   struct thread * t;
   t=thread_current(); 
   struct supp_page_table_entry * spte=(struct supp_page_table_entry *)malloc(sizeof(struct supp_page_table_entry));
   if(spte==NULL){
       //printf("spte entry memory alloc failed");
       return false;
   }
   //if(u_vaddr==NULL)//0 is not null
     //  return false;
   spte->u_vaddr=u_vaddr;
   spte->writable=writable;
   spte->offset=offset;
   spte->read_bytes=read_bytes;
   spte->zero_bytes=zero_bytes;
   spte->type=SPTE_FS;
   spte->is_loaded=false;
   spte->k_vaddr=NULL;
   spte->swap_page=-1;
   spte->magic=1234;
   spte->file_name = malloc(strlen(file_name)+1);
   strlcpy(spte->file_name,file_name, strlen(file_name)+1);
   //printf("file size %d\n",f->inode->data.length);
   //printf("upaage %p\n",u_vaddr);
   if(!hash_insert(&t->supp_page_table,&spte->elem)){
        return true;
   }else{
        //printf("supp_page_table %p, page_entry u_vaddr %p\n",&t->supp_page_table,spte->u_vaddr);
       // printf("hash_insert failed\n");
        return false;
   }   

}


bool spte_install_zero(void * u_vaddr,bool writable){
   struct thread * t;
   t=thread_current(); 
   struct supp_page_table_entry * spte=(struct supp_page_table_entry *)malloc(sizeof(struct supp_page_table_entry));
   if(spte==NULL){
       //printf("spte entry memory alloc failed");
       return false;
   }
   //if(u_vaddr==NULL)//0 is not null
     //  return false;
   spte->u_vaddr=u_vaddr;
   spte->writable=writable;
   spte->offset=-1;
   spte->read_bytes=-1;
   spte->zero_bytes=-1;
   spte->type=SPTE_ZERO;
   spte->is_loaded=false;
   spte->k_vaddr=NULL;
   spte->swap_page=-1;
   spte->file_name = NULL;
   //printf("file size %d\n",f->inode->data.length);
   //printf("upaage %p\n",u_vaddr);
   if(!hash_insert(&t->supp_page_table,&spte->elem)){
        return true;
   }else{
        //printf("supp_page_table %p, page_entry u_vaddr %p\n",&t->supp_page_table,spte->u_vaddr);
       // printf("hash_insert failed\n");
        return false;
   } 
}




void destroy_spte(struct hash_elem *e, void *aux UNUSED){
   struct supp_page_table_entry *spte=hash_entry(e,struct supp_page_table_entry,elem);
   if(spte->is_loaded){
       vm_free_frame(spte->k_vaddr);  
   }
   free(spte->file_name);
   free(spte);
}

void free_process_resources(){
    struct thread *t;
    t=thread_current();
    hash_destroy(&t->supp_page_table,&destroy_spte); 
}


struct supp_page_table_entry * lookup_supp_page_table(void * u_vaddr){
    struct hash_elem *elem;
    struct list *bucket;
    struct thread * t=thread_current();
    bucket=hash_find_bucket(&t->supp_page_table,spt_hash_u_vaddr(u_vaddr));
    struct list_elem *i;
    bool found=false;
    struct supp_page_table_entry *spte;
    for (i = list_begin (bucket); i != list_end (bucket); i = list_next (i)){
        elem = list_elem_to_hash_elem (i);
        spte=hash_entry(elem,struct supp_page_table_entry,elem);
        if(spte->u_vaddr==u_vaddr){
            found=true;
            break;
        }
    }
    if(found)
       return spte;
    else
       return NULL; 

}


bool load_spte(struct supp_page_table_entry *spte){
    if(!spte)
        return false;
    if(spte->is_loaded)
        return false;
    //printf("upaage %p\n",spte->u_vaddr);
    uint8_t *kpage = vm_alloc_frame( PAL_USER ,spte->u_vaddr);
    if (kpage == NULL)
       return false;
    
    switch(spte->type){
        case SPTE_FS : ;//this is an empty statement as a label can only be part of a statement and a declaration is not a statement
            struct file *file;
            file = filesys_open (spte->file_name);
            if(!file){
                vm_free_frame(kpage);
                return false;
            }
            int bytes_read=file_read_at(file, kpage, spte->read_bytes,spte->offset);
            if (bytes_read != (int) spte->read_bytes){
                vm_free_frame(kpage);
                //printf("bytes read %d \t bytes should be read %d \n",bytes_read,spte->read_bytes);
                file_close(file);
                return false; 
            }
            memset(kpage + spte->read_bytes, 0, spte->zero_bytes);
            file_close(file);
            break;
        case SPTE_ZERO :
            memset(kpage,0,PGSIZE);
            break;
        default :
            vm_free_frame(kpage);
            return false;
    }
    // Add the page to the process's address space
    if (!install_page (spte->u_vaddr, kpage,spte->writable)) {
          vm_free_frame(kpage);
          return false; 
    }
    spte->is_loaded=true;
    spte->k_vaddr=kpage;
    return true;
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

static unsigned spt_hash_u_vaddr(void * u_vaddr){
   return hash_int((int)u_vaddr);
}
