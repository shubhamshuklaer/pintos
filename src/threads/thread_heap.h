#ifndef THREADS__HEAP_H
#define THREADS__HEAP_H

/*
 Implemented in project 1
 thread_compare -- pass a pointer to the function whose return valuse will be bool
 if true then the first thread will move up in the heap than the second

 heap_size i.e the number of elements in the heap needs to be updated after calling 
 the pop_top and insert function

*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "threads/thread.h"


typedef bool (*thread_compare)(struct thread *,struct thread *); 
struct thread * pop_top(struct thread ** heap,int heap_size,thread_compare comp);
bool insert(struct thread ** heap,struct thread * element,int heap_size,thread_compare comp);
// in a heap if if the properties of a thread changes, calling this function  will
// percolate the thread to the correct position according to the new properties
bool update_pos(struct thread ** heap,int location,int heap_size,thread_compare comp);
bool build_heap(struct thread ** heap,int heap_size,thread_compare comp);
// This is the function that will be recursively called by build_heap..!! It will assume the elements below 
// int location to be in a heap and then percolate down the int location element to its correct position
bool heapify(struct thread ** heap,int location,int heap_size,thread_compare comp);


#endif /* threads/heap.h */