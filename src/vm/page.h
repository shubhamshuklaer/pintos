#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "filesys/file.h"
#include <stdio.h>


enum spte_type{
    SPTE_FS,
    SPTE_SWAP,
    SPTE_ZERO,
    SPTE_MMAP
};

struct supp_page_table_entry{
    void * u_vaddr;
    bool writable;
    bool is_loaded;
    enum spte_type type;

    //pointer to the frame if its loaded
    void * k_vaddr;
   
    //used for file entry
    struct file *file;
    off_t offset;
    uint32_t read_bytes;
    uint32_t zero_bytes; 
   
    //swap entry
    int swap_page; 
    struct hash_elem elem;
};

void free_process_resources();

bool spte_install_fs(void * u_vaddr, struct file * f,off_t offset,
        uint32_t read_bytes, uint32_t zero_bytes,bool writable);

bool spt_less_func (const struct hash_elem *a,const struct hash_elem *b,void *aux);
unsigned spt_hash_func (const struct hash_elem *e, void *aux);

#endif /* vm/page.h */
