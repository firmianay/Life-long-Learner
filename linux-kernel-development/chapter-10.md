# Chapter 10: Kernel Synchronization Methods

## Atomic Operations

atomic operations provide instructions that execute atomically, without interruption.

The kernel provides two sets of interfaces for atomic operations: one that operates on integers and another that operates on individual bits.

### Atomic Integer Operations

The atomic integer methods operate on a special data type, `atomic_t`. This special type is used, as opposed to having the functions work directly on the C `int` type, for several reasons:

1. Having the atomic functions accept only the `atomic_t` type ensures that the atomic operations are used only with these special types. Likewise, it also ensures that the data types are not passed to any non-atomic functions.
2. The use of `atomic_t` ensures the compiler does not optimize access to the value, it is important the atomic operations receive the correct memory address and not an alias.
3. Use of `atomic_t` can hide any architecture-specific differences in its implementation.

The `atomic_t` type is defined in `<linux/types.h>`:

```c
typedef struct {
    volatile int counter;
} atomic_t;
```

The declarations needed to use the atomic integer operations are in `<asm/atomic.h>`.

Atomic Integer Operation | Description
------------------------ | -----------
ATOMIC_INIT(int i) | At declaration, initialize to `i`.
int atomic_read(atomic_t *v) | Atomically read the integer value of `v`.
void atomic_set(atomic_t *v, int i) | Atomically set `v` equal to `i`.
void atomic_add(int i, atomic_t *v) | Atomically add `i` to `v`.
void atomic_sub(int i, atomic_t *v) | Atomically subtract `i` from `v`.
void atomic_inc(atomic_t *v) | Atomically add one to `v`.
void atomic_dec(atomic_t *v) | Atomically subtract one from `v`.
int atomic_sub_and_test(int i, atomic_t *v) | Atomically subtract `i` from `v` and return true if the result is zero; otherwise false.
int atomic_add_negative(int i, atomic_t *v) | Atomically add `i` to `v` and return true if the result is negative; otherwise false.
int atomic_add_return(int i, atomic_t *v) | Atomically add `i` to `v` and return the result.
int atomic_sub_return(int i, atomic_t *v) | Atomically subtract `i` from `v` and return the result.
int atomic_inc_return(int i, atomic_t *v) | Atomically increment `v` by one and return the result.
int atomic_dec_return(int i, atomic_t *v) | Atomically decrement `v` by one and return the result.
int atomic_dec_and_test(atomic_t *v) | Atomically decrement `v` by one and return true if zero; false otherwise.
int atomic_inc_and_test(atomic_t *v) | Atomically increment `v` by one and return true if the result is zero; false otherwise.

The atomic operations are typically implemented as inline functions with inline assembly. In the case where a specific function is inherently atomic, the given function is usually just a macro.

### 64-Bit Atomic operations

`atomic_t` is 32-bit even on 64-bit architectures. Instead, the `atomic64_t` type provides a 64-bit atomic integer.

As with `atomic_t`, the `atomic64_t` type is just a simple wrapper around an integer type a long:

```c
typedef struct {
    volatile long counter;
} atomic64_t;
```

Atomic Integer Operation | Description
------------------------ | -----------
ATOMIC64_INIT(long i) | At declaration, initialize to `i`.
long atomic64_read(atomic64_t *v) | Atomically read the integer value of `v`.
void atomic64_set(atomic64_t *v, int i) | Atomically set `v` equal to `i`.
void atomic64_add(int i, atomic64_t *v) | Atomically add `i` to `v`.
void atomic64_sub(int i, atomic64_t *v) | Atomically subtract `i` from v.
void atomic64_inc(atomic64_t *v) | Atomically add one to `v`.
void atomic64_dec(atomic64_t *v) | Atomically subtract one from `v`.
int atomic64_sub_and_test(int i, atomic64_t *v) | Atomically subtract `i` from `v` and return true if the result is zero; otherwise false.
int atomic64_add_negative(int i, atomic64_t *v) | Atomically add `i` to `v` and return true if the result is negative; otherwise false.
long atomic64_add_return(int i, atomic64_t *v) | Atomically add `i` to `v` and return the result.
long atomic64_sub_return(int i, atomic64_t *v) | Atomically subtract `i` from `v` and return the result.
long atomic64_inc_return(int i, atomic64_t *v) | Atomically increment `v` by one and return the result.
long atomic64_dec_return(int i, atomic64_t *v) | Atomically decrement `v` by one and return the result.
int atomic64_dec_and_test(atomic64_t *v) | Atomically decrement `v` by one and return true if zero; false otherwise.
int atomic64_inc_and_test(atomic64_t *v) | Atomically increment `v` by one and return true if the result is zero; false otherwise.

### Atomic Bitwise Operations

The functions that operate at the bit level are architecture-specific and defined in `<asm/bitops.h>`.

The bitwise functions operate on generic memory addresses. The arguments are a pointer and a bit number. Bit zero is the least significant bit of the given address.

A listing of the standard atomic bit operations:

Atomic Bitwise Operation | Description
------------------------ | -----------
void set_bit(int nr, void *addr) | Atomically set the `nr`-th bit starting from `addr`.
void clear_bit(int nr, void *addr) | Atomically clear the `nr`-th bit starting from `addr`.
void change_bit(int nr, void *addr) | Atomically flip the value of the `nr`-th bit starting from `addr`.
int test_and_set_bit(int nr, void *addr) | Atomically set the `nr`-th bit starting from `addr` and return the previous value.
int test_and_clear_bit(int nr, void *addr) | Atomically clear the `nr`-th bit starting from `addr` and return the previous value.
int test_and_change_bit(int nr, void *addr) | Atomically flip the `nr`-th bit starting from `addr` and return the previous value.
int test_bit(int nr, void *addr) | Atomically return the value of the `nr`-th bit starting from `addr`.

Conveniently, nonatomic versions of all the bitwise functions are also provided. They behave identically to their atomic siblings, except they do not guarantee atomicity, and their names are prefixed with double underscores.

The kernel also provides routines to find the first set (or unset) bit starting at a given address:

```c
int find_first_bit(unsigned long *addr, unsigned int size)
int find_first_zero_bit(unsigned long *addr, unsigned int size)
```

Both functions take a pointer as their first argument and the number of bits in total to search as their second. They return the bit number of the first set or first unset bit.

## Spin Locks

A spin lock is a lock that can be held by at most one thread of execution. If a thread of execution attempts to acquire a spin lock while it is already held, which is called `contended`, the thread busy loops, `spins`—waiting for the lock to become available.

The fact that a contended spin lock causes threads to spin while waiting for the lock to become available is salient. It is not wise to hold a spin lock for a long time. So spin lock is a lightweight single-holder lock that should be held for short durations.

### Spin Lock Methods

Spin locks are architecture-dependent and implemented in assembly. The architecture-dependent code is defined in `<asm/spinlock.h>`. The actual usable interfaces are defined in `<linux/spinlock.h>`. The basic use of a spin lock is:

```c
DEFINE_SPINLOCK(mr_lock);
spin_lock(&mr_lock);
/* critical region ... */
spin_unlock(&mr_lock);
```

Only one thread is allowed in the critical region at a time.This provides the needed protection from concurrency on multiprocessing machines. On uniprocessor machines, the locks compile away and do not exist; they simply act as markers to disable and enable kernel preemption. If kernel preempt is turned off, the locks compile away entirely.

Noted that the linux kernel's spin locks are not recursive.

If a lock is used in an interrupt handler, you must also disable local interrupts before obtaining the lock. Otherwise, it is possible for a double-acquire deadlock. Note that you need to disable interrupts only on the `current` processor.

The kernel provides an interface that conveniently disables interrupts and acquires the lock. Usage is:

```c
DEFINE_SPINLOCK(mr_lock);
unsigned long flags;

spin_lock_irqsave(&mr_lock, flags);
/* critical region ... */
spin_unlock_irqrestore(&mr_lock, flags);
```

The routine `spin_lock_irqsave()` saves the current state of interrupts, disables them locally, and then obtains the given lock. Conversely, `spin_unlock_irqrestore()` unlocks the given lock and returns interrupts to their previous state. Note that the flags variable is seemingly passed by value. This is because the lock routines are implemented partially as macros.

On uniprocessor systems, the previous example must still disable interrupts to prevent an interrupt handler from accessing the shared data, but the lock mechanism is compiled away. The lock and unlock also disable and enable kernel preemption, respectively.

The configure option `CONFIG_DEBUG_SPINLOCK` enables a handful of debugging checks in the spin lock code.

### Other Spin Lock Methods

Method | Description
------ | -----------
spin_lock() | Acquires given lock
spin_lock_irq() | Disables local interrupts and acquires given lock
spin_lock_irqsave() | Saves current state of local interrupts, disables local interrupts, and acquires given lock
spin_unlock() | Releases given lock
spin_unlock_irq() | Releases given lock and enables local interrupts
spin_unlock_irqrestore() | Releases given lock and restores local interrupts to given previous state
spin_lock_init() | Dynamically initializes given spinlock_t
spin_trylock() | Tries to acquire given lock; if unavailable, returns nonzero
spin_is_locked() | Returns nonzero if the given lock is currently acquired, otherwise it returns zero

### Spin Locks and Bottom Halves

certain locking precautions must be taken when working with bottom halves.The function `spin_lock_bh()` obtains the given lock and disables all bottom halves.The function `spin_unlock_bh()` performs the inverse.

Because a bottom half might preempt process context code, if data is shared between a bottom-half process context, you must protect the data in process context with both a lock and the disabling of bottom halves. Likewise, because an interrupt handler might preempt a bottom half, if data is shared between an interrupt handler and a bottom half, you must both obtain the appropriate lock and disable interrupts.

Recall that two tasklets of the same type do not ever run simultaneously. Thus, there is no need to protect data used only within a single type of tasklet. If the data is shared between two different tasklets, you must obtain a normal spin lock before accessing the data in the bottom half. You do not need to disable bottom halves because a tasklet never preempts another running tasklet on the same processor.

With softirqs, regardless of whether it is the same softirq type, if data is shared by softirqs, it must be protected with a lock. Recall that softirqs, even two of the same type, might run simultaneously on multiple processors in the system. A softirq never preempts another softirq running on the same processor, however, so disabling bottom halves is not needed.

## Reader-Writer Spin Locks

Linux kernel provides reader-writer spin locks. Reader-writer spin locks provide separate reader and writer variants of the lock. One or more readers can concurrently hold the reader lock.The writer lock, conversely, can be held by at most one writer with no concurrent readers.

The reader-writer spin lock is initialized via:

```c
DEFINE_RWLOCK(mr_rwlock);
```

Then, in the reader code path:

```c
read_lock(&mr_rwlock);
/* critical section (read only) ... */
read_unlock(&mr_rwlock);
```

Finally, in the writer code path:

```c
write_lock(&mr_rwlock);
/* critical section (read and write) ... */
write_unlock(&mr_lock);
```

Normally, the readers and writers are in entirely separate code paths.

Note that you cannot “upgrade” a read lock to a write lock. For example, executing these two functions as shown will deadlock:

```c
read_lock(&mr_rwlock);
write_lock(&mr_rwlock);
```

See Table for a full listing of the reader-writer spin lock methods:

Method | Description
------ | -----------
read_lock() | Acquires given lock for reading
read_lock_irq() | Disables local interrupts and acquires given lock for reading
read_lock_irqsave() | Saves the current state of local interrupts, disables local interrupts, and acquires the given lock for reading
read_unlock() | Releases given lock for reading
read_unlock_irq() | Releases given lock and enables local interrupts
read_unlock_irqrestore() | Releases given lock and restores local interrupts to the given previous state
write_lock() | Acquires given lock for writing
write_lock_irq() | Disables local interrupts and acquires the given lock for writing
write_lock_irqsave() | Saves current state of local interrupts, disables local interrupts, and acquires the given lock for writing
write_unlock() | Releases given lock
write_unlock_irq() | Releases given lock and enables local interrupts
write_unlock_irqrestore() | Releases given lock and restores local interrupts to given previous state
write_trylock() | Tries to acquire given lock for writing; if unavailable, returns nonzero
rwlock_init() | Initializes given rwlock_t

A final important consideration in using the Linux reader-writer spin locks is that they favor readers over writers.

## Semaphores

Semaphores in Linux are sleeping locks. When a task attempts to acquire a semaphore that is unavailable, the semaphore places the task onto a wait queue and puts the task to sleep. The processor is then free to execute other code. When the semaphore becomes available, one of the tasks on the wait queue is awakened so that it can then acquire the semaphore.

The uses of semaphores versus spin locks.:

- Because the contending tasks sleep while waiting for the lock to become available, semaphores are well suited to locks that are held for a long time.
- Conversely, semaphores are not optimal for locks that are held for short periods because the overhead of sleeping, maintaining the wait queue, and waking back up can easily outweigh the total lock hold time.
- Because a thread of execution sleeps on lock contention, semaphores must be obtained only in process context because interrupt context is not schedulable.
- You can sleep while holding a semaphore because you will not deadlock when another process acquires the same semaphore.
- You cannot hold a spin lock while you acquire a semaphore, because you might have to sleep while waiting for the semaphore, and you cannot sleep while holding a spin lock.

### Counting and Binary Semaphores

A feature of semaphores is that they can allow for an arbitrary number of simultaneous lock holders. Whereas spin locks permit at most one task to hold the lock at a time, the number of permissible simultaneous holders of semaphores can be set at declaration time.This value is called the `usage count` or simply the `count`.

- `binary semaphore`: the count is equal to one.
- `counting semaphore`: the count initialized to a nonzero value greater than one.

- `up`: used to release a semaphore upon completion of a critical region.
- `down`: used to acquire a semaphore by decrementing the count by one.

### Creating and Initializing Semaphores

The semaphore implementation is architecture-dependent and defined in `<asm/semaphore.h>`. The struct semaphore type represents semaphores. Statically declared semaphores are created via the following:

```c
struct semaphore name;
sema_init(&name, count);
```

As a shortcut to create the more common mutex, use the following:

```c
static DECLARE_MUTEX(name);
```

More frequently, semaphores are created dynamically, often as part of a larger structure.
In this case, to initialize a dynamically created semaphore to which you have only an indirect pointer reference, just call `sema_init()`, where `sem` is a pointer and `count` is the usage count of the semaphore:

```c
sema_init(sem, count);
```

Similarly, to initialize a dynamically created mutex:

```c
init_MUTEX(sem);
```

### Using Semaphores

Method | Description
------ | -----------
sema_init(struct semaphore *, int) | Initializes the dynamically created semaphore to the given count
init_MUTEX(struct semaphore *) | Initializes the dynamically created semaphore with a count of one
init_MUTEX_LOCKED(struct semaphore *) | Initializes the dynamically created semaphore with a count of zero (so it is initially locked)
down_interruptible (struct semaphore *) | Tries to acquire the given semaphore and enter interruptible sleep if it is contended
down(struct semaphore *) | Tries to acquire the given semaphore and enter uninterruptible sleep if it is contended
down_trylock(struct semaphore *) | Tries to acquire the given semaphore and immediately return nonzero if it is contended
up(struct semaphore *) | Releases the given semaphore and wakes a waiting task, if any

## Reader-Writer Semaphores

Reader-writer semaphores are represented by the `struct rw_semaphore` type, which is declared in `<linux/rwsem.h>`. Statically declared reader-writer semaphores are created via the following:

```c
static DECLARE_RWSEM(name);
```

Reader-writer semaphores created dynamically are initialized via:

```c
init_rwsem(struct rw_semaphore *sem)
```

All reader-writer semaphores's usage count is one.

All reader-writer locks use uninterruptible sleep, so there is only one version of each `down()`:

```c
static DECLARE_RWSEM(mr_rwsem);

/* attempt to acquire the semaphore for reading ... */
down_read(&mr_rwsem);

/* critical region (read only) ... */

/* release the semaphore */
up_read(&mr_rwsem);
/* ... */

/* attempt to acquire the semaphore for writing ... */
down_write(&mr_rwsem);

/* critical region (read and write) ... */

/* release the semaphore */
up_write(&mr_sem);
```

Implementations of `down_read_trylock()` and `down_write_trylock()` are provided. Each has one parameter: a pointer to a reader-writer semaphore. They both return nonzero if the lock is successfully acquired and zero if it is currently contended.

Reader-writer semaphores have a unique method that their reader-writer spin lock cousins do not have: `downgrade_write()`. This function atomically converts an acquired write lock to a read lock.

## Mutexes

The term "mutex" is a generic name to refer to any sleeping lock that enforces mutual exclusion. But now, "mutex" also a specific type of sleeping lock that implements mutual exclusion.

The mutex is represented by `struct mutex`. It behaves similar to a semaphore with a count of one.

To statically define a mutex:

```c
DEFINE_MUTEX(name);
```

To dynamically initialize a mutex:

```c
mutex_init(&mutex);
```

A listing of the basic mutex methods:

Method | Description
------ | -----------
mutex_lock(struct mutex *) | Locks the given mutex; sleeps if the lock is unavailable
mutex_unlock(struct mutex *) | Unlocks the given mutex
mutex_trylock(struct mutex *) | Tries to acquire the given mutex; returns one if successful and the lock is acquired and zero otherwise
mutex_is_locked(struct mutex *) | Returns one if the lock is locked and zero otherwise

The mutex has a stricter, narrower use case:

- Only one task can hold the mutex at a time. That is, the usage count on a mutex is always one.
- Whoever locked a mutex must unlock it. Most use cases, cleanly lock and unlock from the same context.
- Recursive locks and unlocks are not allowed.
- A process cannot exit while holding a mutex.
- A mutex cannot be acquired by an interrupt handler or bottom half, even with `mutex_trylock()`.
- A mutex can be managed only via the official API.

The most useful aspect of the new struct mutex is that, via a special debugging mode, the kernel can programmatically check for and warn about violations of these constraints.

### Semaphores Versus Mutexes

Unless one of mutex’s additional con-
straints prevent you from using them, prefer the new mutex type to semaphores.

### Spin Locks Versus Mutexes

Only a spin lock can be used in interrupt context, whereas only a mutex can be held while a task sleeps.

Requirement | Recommended Lock
----------- | ----------------
Low overhead locking | Spin lock is preferred
Short lock hold time | Spin lock is preferred
Long lock hold time | Mutex is preferred
Need to lock from interrupt context | Spin lock is required
Need to sleep while holding lock | Mutex is required

## Completion Variables

Using `completion variables` is an easy way to synchronize between two tasks in the kernel when one task needs to signal to the other that an event has occurred.

Completion variables are represented by the `struct completion` type, which is defined in `<linux/completion.h>`. A statically created completion variable is created and initialized via:

```c
DECLARE_COMPLETION(mr_comp);
```

Method | Description
------ | -----------
init_completion(struct completion *) | Initializes the given dynamically created completion variable
wait_for_completion(struct completion *) | Waits for the given completion variable to be signaled
complete(struct completion *) | Signals any waiting tasks to wake up

A common usage is to have a completion variable dynamically created as a member of a data structure. Kernel code waiting for the initialization of the data structure calls `wait_for_completion()`. When the initialization is complete, the waiting tasks are awakened via a call to `completion()`.

## Sequential Locks

The `squential lock` provides a simple mechanism for reading and writing shared data. It works by maintaining a sequence counter. Whenever the data in question is written to, a lock is obtained and a sequence number is incremented. Prior to and after reading the data, the sequence number is read. If the values are the same, a write did not begin in the middle of the read. Further, if the values are even, a write is not underway.

To define a seq lock:

```c
seqlock_t mr_seq_lock = DEFINE_SEQLOCK(mr_seq_lock);
```

The write path is:

```c
write_seqlock(&mr_seq_lock);
/* write lock is obtained... */
write_sequnlock(&mr_seq_lock);
```

This looks like normal spin lock code. The oddness comes in with the read path, which is quite a bit different:

```c
unsigned long seq;
do {
      seq = read_seqbegin(&mr_seq_lock);
      /* read data here ... */
} while (read_seqretry(&mr_seq_lock, seq));
```

Seq locks favor writers over readers.

Seq locks are ideal when your locking needs meet most or all these requirements:

- Your data has a lot of readers.
- Your data has few writers.
- Although few in number, you want to favor writers over readers and never allow readers to starve writers.
- Your data is simple.

## Preemption Disabling

the kernel preemption code uses spin locks as markers of nonpreemptive regions. If a spin lock is held, the kernel is not preemptive.

Sometimes we do not require a spin lock, instead, `preempt_disable()` can disables kernel preemption. The call is nestable; you can call it any number of times. For each call, a corresponding call to `preempt_enable()` is required. The final corresponding call to `preempt_enable()` reenables preemption.

The preemption count stores the number of held locks and `preempt_disable()` calls. If the number is zero, the kernel is preemptive. If the value is one or greater, the kernel is not preemptive.

A listing of kernel preemption-related functions:

Function | Description
-------- | -----------
preempt_disable() | Disables kernel preemption by incrementing the preemption counter
preempt_enable() | Decrements the preemption counter and checks and services any pending reschedules if the count is now zero
preempt_enable_no_resched() | Enables kernel preemption but does not check for any pending reschedules
preempt_count() | Returns the preemption count

## Ordering and Barriers

When dealing with synchronization between multiple processors or with hardware devices, it is sometimes a requirement that memory-reads (loads) and memory-writes (stores) issue in the order specified in your program code.

A full listing of the memory and compiler barrier methods provided by all architectures in the Linux kernel:

Barrier | Description
------- | -----------
rmb() | Prevents loads from being reordered across the barrier
read_barrier_depends() | Prevents data-dependent loads from being reordered across the barrier
wmb() | Prevents stores from being reordered across the barrier
mb() | Prevents load or stores from being reordered across the barrier
smp_rmb() | Provides an rmb() on SMP, and on UP provides a `barrier()`
smp_read_barrier_depends() | Provides a `read_barrier_depends()` on SMP, and provides a `barrier()` on UP
smp_wmb() | Provides a `wmb()` on SMP, and provides a `barrier()` on UP
smp_mb() | Provides an `mb()` on SMP, and provides a `barrier()` on UP
barrier() | Prevents the compiler from optimizing stores or loads across the barrier
