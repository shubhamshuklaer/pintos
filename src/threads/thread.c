  #include "threads/thread.h"
  #include <debug.h>
  #include <stddef.h>
  #include <random.h>
  #include <stdio.h>
  #include <string.h>
  #include <stdarg.h>
  #include <list.h>
  #include "threads/flags.h"
  #include "threads/interrupt.h"
  #include "threads/intr-stubs.h"
  #include "threads/palloc.h"
  #include "threads/switch.h"
  #include "threads/synch.h"
  #include "threads/vaddr.h"
  #include "threads/thread_heap.h"
  #include "threads/malloc.h"
  #ifdef USERPROG
  #include "userprog/process.h"
  #endif

  /* Random value for struct thread's `magic' member.
     Used to detect stack overflow.  See the big comment at the top
     of thread.h for details. */
  #define THREAD_MAGIC 0xcd6abf4b




  /* List of all processes.  Processes are added to this list
     when they are first scheduled and removed when they exit. */
  static struct list all_list;


  /* Sleep list */
  static struct list sleep_list;


  /* Idle thread. */
  static struct thread *idle_thread;

  /* Initial thread, the thread running init.c:main(). */
  static struct thread *initial_thread;

  /* Lock used by allocate_tid(). */
  static struct lock tid_lock;

  static int max_tid = 0;

  /* Stack frame for kernel_thread(). */
  struct kernel_thread_frame 
    {
      void *eip;                  /* Return address. */
      thread_func *function;      /* Function to call. */
      void *aux;                  /* Auxiliary data for function. */
    };

  /* Statistics. */
  static long long idle_ticks;    /* # of timer ticks spent idle. */
  static long long kernel_ticks;  /* # of timer ticks in kernel threads. */
  static long long user_ticks;    /* # of timer ticks in user programs. */

  /* Scheduling. */
  #define TIME_SLICE 4            /* # of timer ticks to give each thread. */
  static unsigned thread_ticks;   /* # of timer ticks since last yield. */

  /* If false (default), use round-robin scheduler.
     If true, use multi-level feedback queue scheduler.
     Controlled by kernel command-line option "-o mlfqs". */
  bool thread_mlfqs;

  static void kernel_thread (thread_func *, void *aux);

  static void idle (void *aux UNUSED);
  static struct thread *next_thread_to_run (void);
  static void init_thread (struct thread *, const char *name, int priority);
  static void *alloc_frame (struct thread *, size_t size);
  static void schedule (void);
  void schedule_tail (struct thread *prev);
  static tid_t allocate_tid (void);



  // For scheduler
  /* heap of processes in THREAD_READY state, that is, processes
     that are ready to run but not actually running. */
  static struct thread ** ready_heap;
  // num_threads_ready is num of threads in the ready heap
  // heap capacity is the total size of ready heap array
  static int num_threads_ready,heap_capacity;

  // Insert a thread into ready heap
  static bool insert_in_ready_heap(struct thread *);

  // this is for ensuring that the sort provided by heap is stable sort by
  // ensuring that no two threads have same priority(Actual priority and read heap insertion rank)
  static long long ready_insertion_rank;

  /* Initializes the threading system by transforming the code
     that's currently running into a thread.  This can't work in
     general and it is possible in this case only because loader.S
     was careful to put the bottom of the stack at a page boundary.

     Also initializes the run queue and the tid lock.

     After calling this function, be sure to initialize the page
     allocator before trying to create any threads with
     thread_create().

     It is not safe to call thread_current() until this function
     finishes. */
  void
  thread_init (void) 
  {
    // printf("thread init\n");
    ASSERT (intr_get_level () == INTR_OFF);
    lock_init (&tid_lock);
    list_init (&all_list);
    list_init (&sleep_list);
    /* Set up a thread structure for the running thread. */
    // printf("thread init 1\n");
    initial_thread = running_thread ();
    // printf("thread init 2\n");
    init_thread (initial_thread, "main", PRI_DEFAULT);
    // printf("thread init 3\n");
    initial_thread->status = THREAD_RUNNING;
    // printf("thread init 4\n");
    initial_thread->tid = allocate_tid ();
    // printf("thread init exit\n");
    
    // printf("++ init sema for: '%s' to 0\n", initial_thread->name);

  }

  /* Starts preemptive thread scheduling by enabling interrupts.
     Also creates the idle thread. */
  void
  thread_start (void) 
  {
    // for scheduling
    ready_heap=(struct thread **)malloc(READY_HEAP_MIN_SIZE* sizeof(struct thread *));
    num_threads_ready=0;
    heap_capacity=READY_HEAP_MIN_SIZE;
    ready_insertion_rank=0;
    /* Create the idle thread. */
    struct semaphore idle_started;
    sema_init (&idle_started, 0);
    thread_create ("idle", PRI_MIN, idle, &idle_started);
    /* Start preemptive thread scheduling. */
    intr_enable ();
    /* Wait for the idle thread to initialize idle_thread. */
    sema_down (&idle_started);
    
  }

  /* Called by the timer interrupt handler at each timer tick.
     Thus, this function runs in an external interrupt context. */
  void
  thread_tick (void) 
  {
    struct thread *t = thread_current ();
    struct list_elem *e;
    /* Update statistics. */
    if (t == idle_thread)
      idle_ticks++;
  #ifdef USERPROG
    else if (t->pagedir != NULL)
      user_ticks++;
  #endif
    else
      kernel_ticks++;

    // sleep list
    for (e = list_begin (&sleep_list); e != list_end (&sleep_list);e = list_next (e)){
      struct thread *s = list_entry (e, struct thread, elem);
      s->ticks_left--;
    }
    while(!list_empty(&sleep_list)&&list_entry(list_front(&sleep_list),struct thread,elem)->ticks_left==0){
      thread_unblock(list_entry(list_pop_front(&sleep_list),struct thread, elem));
    }

    /* Enforce preemption. */
    if (++thread_ticks >= TIME_SLICE)
      intr_yield_on_return ();
    // for preemption of the current thread if its priority is less than priority of read_heap[0]
    // We will be implementing it through interrupt later but its necesary so that if interrupt are disabled at the time
    // the new element was pushed in the ready_heap then we will miss the premption so this part will be needed even after \
    // we handle preemption using interrupts..!!
    if(t->priority<ready_heap[0]->priority){
      intr_yield_on_return ();
    }
  }

  /* Prints thread statistics. */
  void
  thread_print_stats (void) 
  {
    printf ("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
            idle_ticks, kernel_ticks, user_ticks);
  }

  /* Creates a new kernel thread named NAME with the given initial
     PRIORITY, which executes FUNCTION passing AUX as the argument,
     and adds it to the ready queue.  Returns the thread identifier
     for the new thread, or TID_ERROR if creation fails.

     If thread_start() has been called, then the new thread may be
     scheduled before thread_create() returns.  It could even exit
     before thread_create() returns.  Contrariwise, the original
     thread may run for any amount of time before the new thread is
     scheduled.  Use a semaphore or some other form of
     synchronization if you need to ensure ordering.

     The code provided sets the new thread's `priority' member to
     PRIORITY, but no actual priority scheduling is implemented.
     Priority scheduling is the goal of Problem 1-3. */
  tid_t
  thread_create (const char *name, int priority,
                 thread_func *function, void *aux) 
  {
    struct thread *t;
    struct kernel_thread_frame *kf;
    struct switch_entry_frame *ef;
    struct switch_threads_frame *sf;
    tid_t tid;
    enum intr_level old_level;

    ASSERT (function != NULL);

    /* Allocate thread. */
    t = palloc_get_page (PAL_ZERO);
    if (t == NULL)
      return TID_ERROR;

    /* Initialize thread. */
    init_thread (t, name, priority);
    tid = t->tid = allocate_tid ();

    /* Prepare thread for first run by initializing its stack.
       Do this atomically so intermediate values for the 'stack' 
       member cannot be observed. */
    old_level = intr_disable ();

    /* Stack frame for kernel_thread(). */
    kf = alloc_frame (t, sizeof *kf);
    kf->eip = NULL;
    kf->function = function;
    kf->aux = aux;

    /* Stack frame for switch_entry(). */
    ef = alloc_frame (t, sizeof *ef);
    ef->eip = (void (*) (void)) kernel_thread;

    /* Stack frame for switch_threads(). */
    sf = alloc_frame (t, sizeof *sf);
    sf->eip = switch_entry;
    sf->ebp = 0;

    intr_set_level (old_level);


#ifdef USERPROG
    /* Set this to be a child thread of current thread */
    int parent_tid = thread_tid();
    // printf("setting child parameters for current process(tid) : %d\n", parent_tid);
    struct thread *parent_thread = thread_current();
    t->parent = parent_thread;
    t->num_child_procs = 0;
    // t->exit_status = 2;
    parent_thread->num_child_procs++;
    // printf("-- init sema for: '%s',to 0\n", name);
    sema_init(&t->me_loading, 1);

    if(is_thread(t->parent)){
      // printf("-+ sema up for : '%s',from %d\n", t->parent->name, t->parent->child_loading.value);
      sema_down(&t->me_loading);
      t->parent->child_loaded_success = false;
    }
    // printf("thread create 1\n");
    if(&t->child_procs){
      // printf("child proc list not null\n");
    }else{
      // printf("child proc list null\n");
    }
    // printf("thread create 2\n");
    list_init(&t->child_procs);
    // printf("thread create 3\n");

    list_push_back(&parent_thread->child_procs, &t->child_proc);
    // printf("thread create 4\n");
    t->signal_parent_on_exit=false;
    sema_init(&t->parent_wait, 0);
#endif
    /* Add to run queue. */
    thread_unblock (t);

    return tid;
  }

  /* Puts the current thread to sleep.  It will not be scheduled
     again until awoken by thread_unblock().

     This function must be called with interrupts turned off.  It
     is usually a better idea to use one of the synchronization
     primitives in synch.h. */
  void
  thread_block (void) 
  {
    ASSERT (!intr_context ());
    ASSERT (intr_get_level () == INTR_OFF);

    thread_current ()->status = THREAD_BLOCKED;
    schedule ();
  }

  /* Transitions a blocked thread T to the ready-to-run state.
     This is an error if T is not blocked.  (Use thread_yield() to
     make the running thread ready.)

     This function does not preempt the running thread.  This can
     be important: if the caller had disabled interrupts itself,
     it may expect that it can atomically unblock a thread and
     update other data. */
  void
  thread_unblock (struct thread *t) 
  {
    enum intr_level old_level;

    ASSERT (is_thread (t));

    old_level = intr_disable ();
    ASSERT (t->status == THREAD_BLOCKED);
    // for scheduler
    // When the first time thread_start calls thread_create for idle thread after initialization
    // of the thread this function thread_unblock is called but the variable idle_thread is still not initialized 
    // so for the first time idle thread will go into ready queue... but its good for us actually
    // since in next_thread_to_run we will return idle_thread if ready queue is empty but idle_thread
    // is still not initialized till that time...!! So this actually helps us..!!
    if(t!=idle_thread){
      insert_in_ready_heap(t);
    }
    t->status = THREAD_READY;
    intr_set_level (old_level);
    

    if(intr_get_level()==INTR_ON){
      if(thread_current()->priority<ready_heap[0]->priority)
        thread_yield();
    }

  }

  void
  thread_listall(){
    struct list_elem *e;

    for (e = list_begin (&all_list); e != list_end (&all_list);e = list_next (e)){
      struct thread *ptr = list_entry (e, struct thread, allelem);
      printf("Thread pointer : %p and tid : %d\n",ptr,ptr->tid);
      // printf("\n-----------------------------------\n");
      // printf("thread name; %s\n", ptr->name);
      // printf("thread exit status: %d\n", ptr->exit_status);
      // printf("thread num child procs: %d\n", ptr->num_child_procs);
      // printf("\n-----------------------------------\n");
      
    }  
  }


  bool
  valid_tid(tid_t tid){
    // printf("tid to be validated : %d\n", tid);
    // printf("max tid: %d\n", max_tid);
    return (tid <= max_tid && tid >=0);
  }

  bool
  active_tid(tid_t tid){
    struct list_elem *e;

    for (e = list_begin (&all_list); e != list_end (&all_list);e = list_next (e)){
      struct thread *ptr = list_entry (e, struct thread, allelem);
      if(ptr->tid == tid){
        return true;
      }
    }
    return false;
  }

  bool
  thread_alive(tid_t tid){
    struct list_elem *e;
    // printf("hello1");
    for (e = list_begin (&all_list); e != list_end (&all_list);e = list_next (e)){
      struct thread *ptr = list_entry (e, struct thread, allelem);
      // printf("all_list ptrs : %p\n",ptr);
      if(ptr->tid == tid){
        return true;
      }
    }
    // printf("hello2");
    return false;
  }

  /* Returns the name of the running thread. */
  const char *
  thread_name (void) 
  {
    return thread_current ()->name;
  }

  /* Returns the running thread.
     This is running_thread() plus a couple of sanity checks.
     See the big comment at the top of thread.h for details. */
  struct thread *
  thread_current (void) 
  {
    struct thread *t = running_thread ();
    
    /* Make sure T is really a thread.
       If either of these assertions fire, then your thread may
       have overflowed its stack.  Each thread has less than 4 kB
       of stack, so a few big automatic arrays or moderate
       recursion can cause stack overflow. */
    ASSERT (is_thread (t));
    ASSERT (t->status == THREAD_RUNNING);

    return t;
  }

  /* Returns the running thread's tid. */
  tid_t
  thread_tid (void) 
  {
    // printf("asking current threads id\n");
    return thread_current ()->tid;
  }

  /* Deschedules the current thread and destroys it.  Never
     returns to the caller. */
  void
  thread_exit (void) 
  {
    // printf("destroying thread with id : %d\n", thread_current()->tid);
    ASSERT (!intr_context ());

  #ifdef USERPROG
    process_exit ();

  #endif
    // printf("%s\n", "process exit");
    /* Remove thread from all threads list, set our status to dying,
       and schedule another process.  That process will destroy us
       when it call schedule_tail(). */
    intr_disable ();
    list_remove (&thread_current()->allelem);
    // thread_listall();
    // printf("%p %s\n",thread_current() ,"removed from all_list");
    thread_current ()->status = THREAD_DYING;
    schedule ();
    NOT_REACHED ();
    // printf("%s\n", "exit thread");
  }

  /* Yields the CPU.  The current thread is not put to sleep and
     may be scheduled again immediately at the scheduler's whim. */
  void
  thread_yield (void) 
  {
    struct thread *cur = thread_current ();
    enum intr_level old_level;
    
    ASSERT (!intr_context ());

    old_level = intr_disable ();
    if (cur != idle_thread) 
      insert_in_ready_heap(cur);
    cur->status = THREAD_READY;
    schedule ();
    intr_set_level (old_level);
  }

  /* Invoke function 'func' on all threads, passing along 'aux'.
     This function must be called with interrupts off. */
  void
  thread_foreach (thread_action_func *func, void *aux)
  {
    struct list_elem *e;

    ASSERT (intr_get_level () == INTR_OFF);
    for (e = list_begin (&all_list); e != list_end (&all_list);
         e = list_next (e))
      {
        struct thread *t = list_entry (e, struct thread, allelem);
        func (t, aux);
      }
  }

  /* Sets the current thread's priority to NEW_PRIORITY. */
  void
  thread_set_priority (int new_priority) 
  {
    struct thread *t;
    t=thread_current ();
    t->base_priority = new_priority;
    update_priority(t);
  }

  /* Returns the current thread's priority. */
  int
  thread_get_priority (void) 
  {
    int temp;
    enum intr_level old_level = intr_disable ();
    temp= thread_current ()->priority ;
    intr_set_level (old_level);
    return temp;
  }

  /* Sets the current thread's nice value to NICE. */
  void
  thread_set_nice (int nice UNUSED) 
  {
    /* Not yet implemented. */
  }

  /* Returns the current thread's nice value. */
  int
  thread_get_nice (void) 
  {
    /* Not yet implemented. */
    return 0;
  }

  /* Returns 100 times the system load average. */
  int
  thread_get_load_avg (void) 
  {
    /* Not yet implemented. */
    return 0;
  }

  /* Returns 100 times the current thread's recent_cpu value. */
  int
  thread_get_recent_cpu (void) 
  {
    /* Not yet implemented. */
    return 0;
  }
  
  /* Idle thread.  Executes when no other thread is ready to run.

     The idle thread is initially put on the ready list by
     thread_start().  It will be scheduled once initially, at which
     point it initializes idle_thread, "up"s the semaphore passed
     to it to enable thread_start() to continue, and immediately
     blocks.  After that, the idle thread never appears in the
     ready list.  It is returned by next_thread_to_run() as a
     special case when the ready list is empty. */
  static void
  idle (void *idle_started_ UNUSED) 
  {
    struct semaphore *idle_started = idle_started_;
    idle_thread = thread_current ();
    sema_up (idle_started);

    for (;;) 
      {
        /* Let someone else run. */
        intr_disable ();
        thread_block ();

        /* Re-enable interrupts and wait for the next one.

           The `sti' instruction disables interrupts until the
           completion of the next instruction, so these two
           instructions are executed atomically.  This atomicity is
           important; otherwise, an interrupt could be handled
           between re-enabling interrupts and waiting for the next
           one to occur, wasting as much as one clock tick worth of
           time.

           See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
           7.11.1 "HLT Instruction". */
        asm volatile ("sti; hlt" : : : "memory");
      }
  }

  /* Function used as the basis for a kernel thread. */
  static void
  kernel_thread (thread_func *function, void *aux) 
  {
    ASSERT (function != NULL);
    // printf("\n", "executing function");
    intr_enable ();       /* The scheduler runs with interrupts off. */
    function (aux);       /* Execute the thread function. */
    thread_exit ();       /* If function() returns, kill the thread. */
  }
  
  /* Returns the running thread. */
  struct thread *
  running_thread (void) 
  {
    uint32_t *esp;

    /* Copy the CPU's stack pointer into `esp', and then round that
       down to the start of a page.  Because `struct thread' is
       always at the beginning of a page and the stack pointer is
       somewhere in the middle, this locates the curent thread. */
    asm ("mov %%esp, %0" : "=g" (esp));
    return pg_round_down (esp);
  }

  /* Returns true if T appears to point to a valid thread. */
  bool
  is_thread (struct thread *t)
  {
    return t != NULL && t->magic == THREAD_MAGIC;
  }

  /* Does basic initialization of T as a blocked thread named
     NAME. */
  static void
  init_thread (struct thread *t, const char *name, int priority)
  {
    ASSERT (t != NULL);
    ASSERT (PRI_MIN <= priority && priority <= PRI_MAX);
    ASSERT (name != NULL);

    memset (t, 0, sizeof *t);
    t->status = THREAD_BLOCKED;
    strlcpy (t->name, name, sizeof t->name);
    t->stack = (uint8_t *) t + PGSIZE;
    t->priority = priority;
    t->base_priority=priority;
    t->magic = THREAD_MAGIC;
    t->waiting_on_lock = NULL;
    list_init(&(t->donations));
    list_push_back (&all_list, &t->allelem);

    // initialize child processes
    // printf("init thread 1\n");  
    t->num_child_procs = 0;
    

    // printf("init thread 2\n");
    // t->child_procs = malloc(sizeof(list));
    // printf("init thread 3\n");
    list_init(&(t->child_procs));
    // printf("init thread 4\n");
    t->parent = NULL;
    // printf("init thread 5\n");

    //For file handling
    list_init(&(t->file_list));
    t->fd=2;
  }

  /* Allocates a SIZE-byte frame at the top of thread T's stack and
     returns a pointer to the frame's base. */
  static void *
  alloc_frame (struct thread *t, size_t size) 
  {
    /* Stack data is always allocated in word-size units. */
    ASSERT (is_thread (t));
    ASSERT (size % sizeof (uint32_t) == 0);

    t->stack -= size;
    return t->stack;
  }

  /* Chooses and returns the next thread to be scheduled.  Should
     return a thread from the run queue, unless the run queue is
     empty.  (If the running thread can continue running, then it
     will be in the run queue.)  If the run queue is empty, return
     idle_thread. */
  static struct thread *
  next_thread_to_run (void) 
  {
    
    if (num_threads_ready==0){
      return idle_thread;
    }else{
      struct thread *t;
      t=pop_top(ready_heap,num_threads_ready,&compare_priority);
      num_threads_ready--; 
      if(heap_capacity>READY_HEAP_MIN_SIZE && heap_capacity>4*num_threads_ready){
        ready_heap=(struct thread **)realloc(ready_heap,(heap_capacity/2)*sizeof(struct thread *));  
      }
      
      ASSERT(is_thread(t));
      return t;
    }

  }

  /* Completes a thread switch by activating the new thread's page
     tables, and, if the previous thread is dying, destroying it.

     At this function's invocation, we just switched from thread
     PREV, the new thread is already running, and interrupts are
     still disabled.  This function is normally invoked by
     thread_schedule() as its final action before returning, but
     the first time a thread is scheduled it is called by
     switch_entry() (see switch.S).

     It's not safe to call printf() until the thread switch is
     complete.  In practice that means that printf()s should be
     added at the end of the function.

     After this function and its caller returns, the thread switch
     is complete. */
  void
  schedule_tail (struct thread *prev) 
  {
    struct thread *cur = running_thread ();
    
    ASSERT (intr_get_level () == INTR_OFF);

    /* Mark us as running. */
    cur->status = THREAD_RUNNING;

    /* Start new time slice. */
    thread_ticks = 0;

  #ifdef USERPROG
    /* Activate the new address space. */
    process_activate ();
  #endif

    /* If the thread we switched from is dying, destroy its struct
       thread.  This must happen late so that thread_exit() doesn't
       pull out the rug under itself.  (We don't free
       initial_thread because its memory was not obtained via
       palloc().) */
    if (prev != NULL && prev->status == THREAD_DYING && prev != initial_thread) 
      {
        ASSERT (prev != cur);
        palloc_free_page (prev);
      }
  }

  /* Schedules a new process.  At entry, interrupts must be off and
     the running process's state must have been changed from
     running to some other state.  This function finds another
     thread to run and switches to it.

     It's not safe to call printf() until schedule_tail() has
     completed. */
  static void
  schedule (void) 
  {
    struct thread *cur = running_thread ();
    struct thread *next = next_thread_to_run ();
    struct thread *prev = NULL;

    ASSERT (intr_get_level () == INTR_OFF);
    ASSERT (cur->status != THREAD_RUNNING);
    ASSERT (is_thread (next));

    if (cur != next)
      prev = switch_threads (cur, next);
    schedule_tail (prev); 
  }

  /* Returns a tid to use for a new thread. */
  static tid_t
  allocate_tid (void) 
  {

    static tid_t next_tid = 1;
    tid_t tid;

    lock_acquire (&tid_lock);
    tid = next_tid++;
    lock_release (&tid_lock);

    // printf("allocating new tid : %d\n", tid);
    max_tid = tid;
    return tid;
  }

  // For scheduler
  bool compare_priority(struct thread *first, struct thread *second){
    if(first->priority>second->priority)
      return true;
    else if(first->priority==second->priority){
      if(first->insertion_rank<second->insertion_rank)
        return true;
      else
        return false;
      
    }else{
      return false;
    }
  }


  bool insert_in_ready_heap(struct thread *t){
    t->insertion_rank=ready_insertion_rank;
    ready_insertion_rank++;
    if(heap_capacity>=num_threads_ready+1){
      insert(ready_heap,t,num_threads_ready,&compare_priority);
      num_threads_ready++;
    }else{
      ready_heap=(struct thread **)realloc(ready_heap,2*heap_capacity*sizeof(struct thread *));
      if(ready_heap==NULL)
        return false;
      else{
        heap_capacity*=2;
        insert(ready_heap,t,num_threads_ready,&compare_priority);
        num_threads_ready++;
      }
    }
  int i;
  for(i=0;i<num_threads_ready;i++)
    // printf("%s--%s-%d\n",thread_current()->name,ready_heap[i]->name,ready_heap[i]->priority);
    return true;
  }

  void update_ready_heap_pos(struct thread *t){
    int i=0;
    bool found=false;
    while(!found&&i<num_threads_ready){
      if(ready_heap[i]==t)
        found=true;
      else
        i++;
    }
    if(found){
      update_pos(ready_heap,i,num_threads_ready,&compare_priority);
    }
  }

  void update_pos_in_heap(struct thread **heap,struct thread *t,int heap_size){
    int i=0;
    bool found=false;
    while(!found&&i<heap_size){
      if(ready_heap[i]==t)
        found=true;
      else
        i++;
    }
    ASSERT(found);
    if(found){
      update_pos(heap,i,heap_size,&compare_priority);
    }
  }


  void thread_yield_if_applicable(void){
    if(intr_get_level()==INTR_ON){
      if(thread_current()->priority<ready_heap[0]->priority)
        thread_yield();
    }
  }





  void update_priority(struct thread *t){
    int max=t->base_priority;
    struct thread *s;
    if(!list_empty(&(t->donations))){
      s=list_entry(list_front(&(t->donations)),struct thread,donation_elem);
      if(s->priority>max)
        max=s->priority;
    }
    t->priority=max;
    if(t->status==THREAD_RUNNING){
      thread_yield_if_applicable();
    }else if(t->status==THREAD_READY){
      update_pos_in_heap(ready_heap,t,num_threads_ready);
    }else if(t->status==THREAD_BLOCKED){
      while(t->waiting_on_lock!=NULL){
        ASSERT(t->waiting_on_lock->holder!=NULL)
        t=t->waiting_on_lock->holder;
        update_priority(t);
      }
    }

  }

  void remove_donation_for_lock(struct thread *t,struct lock * lock){
    struct list_elem *e,*next,*end;
    struct thread *s;
    e=list_begin(&(t->donations));// is is actually &(list->head.next);
    end=list_end(&(t->donations));//it is actually &(list->tail)
    while(e!=end){
      s=list_entry(e,struct thread,donation_elem);
      next=list_next(e);
      if(s->waiting_on_lock==lock){
        list_remove(e);
      }
      e=next;
    }
  }


  bool donation_list_compare(struct list_elem *first,struct list_elem *second, void *unused){
    struct thread *first_thread,*second_thread;
    first_thread=list_entry(first,struct thread,donation_elem);
    second_thread=list_entry(second,struct thread,donation_elem);
    if(first_thread->priority>second_thread->priority)
      return true;
    else
      return false;

  }


  bool elem_list_compare(struct list_elem *first,struct list_elem *second, void *unused){
    struct thread *first_thread,*second_thread;
    first_thread=list_entry(first,struct thread,elem);
    second_thread=list_entry(second,struct thread,elem);
    if(first_thread->priority > second_thread->priority)
      return true;
    else
      return false;

  }



  static bool sleep_list_compare(struct list_elem *first,struct list_elem *second, void *unused){
    struct thread *first_thread,*second_thread;
    first_thread=list_entry(first,struct thread,elem);
    second_thread=list_entry(second,struct thread,elem);
    if(first_thread->ticks_left < second_thread->ticks_left){
      return true;
    }else if(first_thread->ticks_left == second_thread->ticks_left){
      if(first_thread->priority > second_thread->priority)
        return true;
      else
        return false;
    }else{
      return false;
    }

  } 


  void insert_into_sleep_list(int64_t ticks){
    struct thread *t;
    enum intr_level old_level;
    t=thread_current();
    t->ticks_left=ticks;
    list_insert_ordered(&sleep_list,&(t->elem),&sleep_list_compare,NULL);
    old_level = intr_disable ();
    thread_block();
    intr_set_level(old_level);
  }

  
  /* Offset of `stack' member within `struct thread'.
     Used by switch.S, which can't figure it out on its own. */
  uint32_t thread_stack_ofs = offsetof (struct thread, stack);
