#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <stdio.h>
// Initializes swap system
void swap_init();

// Put the current frame into the swap
// swap_out means to put the contents into a swap file
int swap_out(void * frame);

// takes a page from swap to a frame 
// does not remove the page from swap
bool swap_in(int swap_page_no,void *frame);

// removes a swap page
void swap_remove(int swap_page_no);
#endif /* vm/swap.h */
