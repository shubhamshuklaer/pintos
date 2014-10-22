#include "vm/swap.h"
#include "vm/frame.h"
#include <stdbool.h>
#include <bitmap.h>
#include "devices/disk.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include <debug.h>

#define NUM_SEC_PER_PAGE (PGSIZE/DISK_SECTOR_SIZE)

static struct bitmap *swap_free;
static struct disk *swap_disk;
static int num_sec_per_page;
static struct lock swap_lock;

void swap_init(){
    swap_disk=disk_get(1,1);    
    if(swap_disk==NULL)
        PANIC("Could not get swap disk");
    swap_free=bitmap_create((size_t)(disk_size(swap_disk)/NUM_SEC_PER_PAGE));
    if(swap_free==NULL)
        PANIC("Could not create swap_free bitmap");
    lock_init(&swap_lock);
    lock_acquire(&swap_lock);
    bitmap_set_all(swap_free,true); 
    lock_release(&swap_lock);
}

//put into swap disk
int swap_out(void * frame){
    lock_acquire(&swap_lock);
    int swap_page_no=bitmap_scan(swap_free,0,1,true);
    if(swap_page_no==BITMAP_ERROR){
        printf("swap_page_no %d bitmap_error %d\n",swap_page_no,BITMAP_ERROR);
        PANIC("No free swap slot available");
    } 
    bitmap_set(swap_free,swap_page_no,false);
    lock_release(&swap_lock);
    int i;
    for(i=0;i<NUM_SEC_PER_PAGE;i++){
       disk_write(swap_disk,swap_page_no*NUM_SEC_PER_PAGE+i,frame+i*DISK_SECTOR_SIZE); 
    }
    return swap_page_no;
}


bool swap_in(int swap_page_no,void *frame){
    if(bitmap_test(swap_free,swap_page_no))
        PANIC("Swapping in from empty page");
    if(frame==NULL)
        return false;
    int i;
    for(i=0;i<NUM_SEC_PER_PAGE;i++){
       disk_read(swap_disk,swap_page_no*NUM_SEC_PER_PAGE+i,frame+i*DISK_SECTOR_SIZE); 
    }
    lock_acquire(&swap_lock);
    bitmap_set(swap_free,swap_page_no,true);
    lock_release(&swap_lock);
    return true;
}


void swap_remove(int swap_page_no){
    lock_acquire(&swap_lock);
    bitmap_set(swap_free,swap_page_no,true);
    lock_release(&swap_lock);
}
