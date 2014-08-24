#ifndef THREADS__HEAP_H
#define THREADS__HEAP_H

/*
 Implemented in project 1
 thread_compare -- pass a pointer to the function whose return valuse will be bool
 if true then the first thread will go prior than second thread...!!
 i.e priority of first thread is higher or num ticks left(timer_sleep) is lower depending on what you compare
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "threads/thread.h"


typedef bool (*thread_compare)(struct thread *,struct thread *); 
struct thread * pop_top(struct thread ** heap,int heap_size,thread_compare comp);
bool insert(struct thread ** heap,struct thread * element,int heap_size,thread_compare comp);
bool update_pos(struct thread ** heap,int location,int heap_size,thread_compare comp);


#endif /* threads/heap.h */