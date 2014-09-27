#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING       /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                      /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

#define READY_HEAP_MIN_SIZE 32          /* The minimum size of scheduler's ready heap */

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    int base_priority;                  /* Priority before donation */              
    long long insertion_rank;           /* used for mainitaining FIFO in heap for same values*/
    int64_t ticks_left;                 /* For sleep list */
    struct lock *waiting_on_lock;       /* For priority donation */
    struct list_elem allelem;           /* List element for all threads list. */
    struct list_elem donation_elem;     /* for adding to donation list */
    struct list donations;              /* List containing priority doners */
    
    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */
    
    /* Shared between thread.c and process.c */
    struct thread *parent;
    int num_child_procs;
    struct list child_procs;
    struct list_elem child_proc;
    
    /* file handling*/
    struct list file_list;
    int fd;


#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
struct thread *running_thread (void);

tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);


// For updating the position of a thread in ready heap in case the 
// properties of thread(priority) change while the thread is in ready heap
void update_ready_heap_pos(struct thread *t);

// If this function returns true the first thread goes up in the ready_heap
bool compare_priority(struct thread *first, struct thread *second);

// The function was made non static to make it available for debugging in other places
bool is_thread (struct thread *t);

// Just a generalized version of update_ready_heap_pos just we can pass the heap pointer too
void update_pos_in_heap(struct thread **heap,struct thread *t,int heap_size);

// Yield the thread if interrupts are off and the priority of thread is lesser than that of a 
// ready_thread
void thread_yield_if_applicable(void);

// Updates the priority of the thread based on the priority donations and the state of the thread
// eg. for running thread it yields the thread if applicable after updating priority
// for ready thread call update_ready_heap_pos
// for blocked thread updates the priority of all the threads for which it provides donations as well
// as all the threads which are affected by the updation of priority of all these threads by recursivley 
// calling itself for different threads
void update_priority(struct thread *t);

// Remove the priority donations from a thread for a particular lock
void remove_donation_for_lock(struct thread *t, struct lock * lock);

// The compare function used in list sorting where the list_elem is elem
bool elem_list_compare(struct list_elem *first,struct list_elem *second, void *unused);
// The compare function used in list sorting where the list_elem is donation_elem... 
// i.e its used for sorting the donations list..!!
bool donation_list_compare(struct list_elem *first,struct list_elem *second, void *unused);

// Insert the current thread into sleep list
void insert_into_sleep_list(int64_t ticks);
#endif /* threads/thread.h */
