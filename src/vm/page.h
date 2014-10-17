#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "filesys/file.h"
#include <stdio.h>

#define STACK_MAX_SIZE 30

enum spte_type{
    SPTE_FS,//page in file system as a file
    SPTE_SWAP,//page in swap memory
    SPTE_ZERO,//all zero page
    SPTE_MMAP//for memory mapped files
};

struct supp_page_table_entry{
    void * u_vaddr;
    bool writable;
    bool is_loaded;
    enum spte_type type;

    //pointer to the frame if its loaded
    void * k_vaddr;
   
    //used for file entry
    char * file_name;
    off_t offset;
    uint32_t read_bytes;
    uint32_t zero_bytes; 
    
    //mmap
    struct file * file_ptr;
    //swap entry
    int swap_page; 
    struct hash_elem elem;
    int magic;
};

void free_process_resources();


//Adds a entry of type SPTE_FS into the current thread's supplement page table
bool spte_install_fs(void * u_vaddr, char * file_name,off_t offset,
        uint32_t read_bytes, uint32_t zero_bytes,bool writable);

//Adds a entry of type SPTE_ZERO into the current thread's supplement page table
bool spte_install_zero(void * u_vaddr,bool writable);

//Loads a page identified by supp_page_table_entry into the main memory
bool load_spte(struct supp_page_table_entry *);

//Hash functions.. the supplement page table is a hash..!!
bool spt_less_func (const struct hash_elem *a,const struct hash_elem *b,void *aux);
unsigned spt_hash_func (const struct hash_elem *e, void *aux);

//Searches the current thread's supplement page table
struct supp_page_table_entry * lookup_supp_page_table(void * u_vaddr);

// grow stack - return true if success
bool grow_stack(void *u_vaddr);

//install mmap entry
bool spte_install_mmap(void * u_vaddr, struct file * f,off_t offset,
        uint32_t read_bytes, uint32_t zero_bytes,bool writable);


#endif /* vm/page.h */
