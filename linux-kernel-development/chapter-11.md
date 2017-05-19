# Chapter 11: Timers and Time Management
A large number of kernel function are time-driven, as opposed to even-driven.

## Kernel Notion of Time
The hardware provides a `system timer` that the kernel uses to gauge the passing of time. The system timer goes off at a preprogrammed frequency, called the `tick rate`. When the system timer goes off, it issues an interrupt that the kernel handles via a special interrupt handler. The period is called a `tick` and is equal to `1/(tick rate)` seconds.

Some of the work executed periodically by the timer interrupt:
- Updating the system uptime
- Updating the time of day
- On an SMP system, ensuring that the scheduler runqueues are balanced and, if not, balancing them
- Running any dynamic timers that have expired
- Updating resource usage and processor time statistics


## The Tick Rate: HZ
The frequency of the system timer is programmed on system boot based on a static preprocessor define, `HZ`.

The kernel defines the value in `<asm/param.h>`. The tick rate has a frequency of HZ hertz and a period of 1/HZ seconds.

### The Ideal HZ Value
##### Advantages with a Larger HZ
- Kernel timers execute with finer resolution and increased accuracy.
- System calls such as `poll()` and `select()` that optionally employ a timeout value execute with improved precision.
- Measurements, such as resource usage or the system uptime, are recorded with a finer resolution.
- Process preemption occurs more accurately.

##### Disadvangates with a Larger HZ
A higher tick rate implies more frequent timer interrupts, which implies higher overhead, because the processor must spend more time executing the timer interrupt handler.


## Jiffies
The global variable `jiffies` holds the number of ticks that have occurred since the system booted. The kernel initializes `jiffies` to a special initial value, causing the variable to overflow more often, catching bugs. When the actual value of `jiffies` is sought, this “offset” is first subtracted.

### Internal Representation of Jiffies
The `jiffies` variable has always been an `unsigned long`, and therefore 32 bits in size on 32-bit architectures and 64-bits on 64-bit architectures.

jiffies is defined as an unsigned long in `<linux/jiffies.h>`:
```c
extern unsigned long volatile jiffies;

extern u64 jiffies_64;
```
The `ld(1)` script used to link the main kernel image then `overlays` the jiffies variable over the start of the `jiffies_64` variable:
```c
jiffies = jiffies_64;
```
Thus, `jiffies` is the lower 32 bits of the full 64-bit `jiffies_64` variable. Because most code uses `jiffies` simply to measure elapses in time, most code cares about only the lower 32 bits. The time management code uses the entire 64 bits, however, and thus prevents overflow of the full 64-bit value.

### Jiffies Wraparound
The jiffies variable, experiences overflow when its value is increased beyond its maximum storage limit. When the tick count is equal to this maximum and it is incremented, it wraps around to zero.

The kernel provides four macros for comparing tick counts that correctly handle wraparound in the tick count. They are in `<linux/jiffies.h>`. Listed here are
simplified versions of the macros:
```c
#define time_after(unknown, known) ((long)(known) - (long)(unknown) < 0)
#define time_before(unknown, known) ((long)(unknown) - (long)(known) < 0)
#define time_after_eq(unknown, known) ((long)(unknown) - (long)(known) >= 0)
#define time_before_eq(unknown, known) ((long)(known) - (long)(unknown) >= 0)
```
The unknown parameter is typically jiffies and the known parameter is the value against which you want to compare.

### User-Space and HZ
The kernel defined `USER_HZ`, which is the `HZ` value that user-space expects. The function `jiffies_to_clock_t()`, defined in `kernel/time.c`, is then used to scale a tick count in terms of `HZ` to a tick count in terms of `USER_HZ`. The function `jiffies_64_to_clock_t()` is provided to couvert a 64-bit jiffies value from `HZ` to `USER_HZ` units.


## Hardware Clocks and Timers
### Real-Time Clock
The real-time clock (RTC) provides a nonvolatile device for storing the system time. Its primary importance is only during boot, when the `xtime` variable is initialized.

### System Timer
The system timer is to provide a mechanism for driving an interrupt at a periodic rate. On x86, the primary system timer is the programmable interrupt timer (PIT).


## The Timer Interrupt Handler
The timer interrupt is broken into two pieces: an architecture-dependent and an architecture-independent routine.

The architecture-dependent routine is registered as the interrupt handler for the system timer and, thus, runs when the timer interrupt hits.

Most handlers perform at least the following work:
- Obtain the `xtime_lock` lock, which protects access to `jiffies_64` and the wall time value, `xtime`.
- Acknowledge or reset the system timer as required.
- Periodically save the updated wall time to the real time clock.
- Call the architecture-independent timer routine, `tick_periodic()`.

The architecture-independent routine, `tick_periodic()`, performs much more work:
- Increment the `jiffies_64` count by one.
- Update resource usages, such as consumed system and user time, for the currently running process.
- Run any dynamic timers that have expired.
- Execute `scheduler_tick()`.
- Update the wall time, which is stored in `xtime`.
- Calculate the infamous load average.

```c
static void tick_periodic(int cpu)
{
    if (tick_do_timer_cpu == cpu) {
          write_seqlock(&xtime_lock);

          /* Keep track of the next tick event */
          tick_next_period = ktime_add(tick_next_period, tick_period);

          do_timer(1);
          write_sequnlock(&xtime_lock);
    }

    update_process_times(user_mode(get_irq_regs()));
    profile_tick(CPU_PROFILING);
}
```

`do_timer()` is responsible for actually performing the increment to `jiffies_64`:
```c
void do_timer(unsigned long ticks)
{
    jiffies_64 += ticks;
    update_wall_time();
    calc_global_load();
}
```
The function `update_wall_time()` updates the wall time in accordance with the elapsed ticks, whereas `calc_global_load()` updates the system’s load average statistics.

When `do_timer()` ultimately returns, `update_process_times()` is invoked to update various statistics that a tick has elapsed, noting via `user_tick` whether it occurred in user-space or kernel-space:
```c
void update_process_times(int user_tick)
{
    struct task_struct *p = current;
    int cpu = smp_processor_id();

    /* Note: this timer irq context must be accounted for as well. */
    account_process_tick(p, user_tick);
    run_local_timers();
    rcu_check_callbacks(cpu, user_tick);
    printk_tick();
    scheduler_tick();
    run_posix_cpu_timers(p);
}
```

The `account_process_tick()` function does the actual updating of the process’s times:
```c
void account_process_tick(struct task_struct *p, int user_tick)
{
        cputime_t one_jiffy_scaled = cputime_to_scaled(cputime_one_jiffy);
        struct rq *rq = this_rq();

        if (user_tick)
                account_user_time(p, cputime_one_jiffy, one_jiffy_scaled);
        else if ((p != rq->idle) || (irq_count() != HARDIRQ_OFFSET))
                account_system_time(p, HARDIRQ_OFFSET, cputime_one_jiffy,
                                    one_jiffy_scaled);
        else
                account_idle_time(cputime_one_jiffy);
}
```

Next, the `run_local_timers()` function marks a softirq to handle the execution of any expired timers.

Finally, the `scheduler_tick()` function decrements the currently running process’s timeslice and sets `need_resched` if needed.


## The Time of Day
The current time of day (the wall time) is defined in `kernel/time/timekeeping.c`:
```c
struct timespec xtime;
```
The timespec data structure is defined in `<linux/time.h>` as:
```c
struct timespec {
        __kernel_time_t tv_sec;   /* seconds */
        long tv_nsec;             /* nanoseconds */
};
```

Reading or writing the `xtime` variable requires the `xtime_lock` lock, which is not a normal spinlock but a seqlock.
```c
write_seqlock(&xtime_lock);

/* update xtime ... */

write_sequnlock(&xtime_lock);
```
Reading `xtime` requires the use of the `read_seqbegin()` and `read_seqretry()` functions:
```c
unsigned long seq;

do {
        unsigned long lost;
        seq = read_seqbegin(&xtime_lock);

        usec = timer->get_offset();
        lost = jiffies - wall_jiffies;
        if (lost)
                usec += lost * (1000000 / HZ);
        sec = xtime.tv_sec;
        usec += (xtime.tv_nsec / 1000);
} while (read_seqretry(&xtime_lock, seq));
```
This loop repeats until the reader is assured that it read the data without an intervening write. If the timer interrupt occurred and updated xtime during the loop, the returned sequence number is invalid and the loop repeats.

The primary user-space interface for retrieving the wall time is `gettimeofday()`, which is implemented as `sys_gettimeofday()` in `kernel/time.c`:
```c
asmlinkage long sys_gettimeofday(struct timeval *tv, struct timezone *tz)
{
        if (likely(tv)) {
                struct timeval ktv;
                do_gettimeofday(&ktv);
                if (copy_to_user(tv, &ktv, sizeof(ktv)))
                        return -EFAULT;
        }
        if (unlikely(tz)) {
                if (copy_to_user(tz, &sys_tz, sizeof(sys_tz)))
                return -EFAULT;
        }
        return 0;
}
```


## Timers
Timers is a tool for delaying work a specified amount of time.

### Using Timers
Timers are represented by `struct timer_list`, which is defined in `<linux/timer.h>`:
```c
struct timer_list {
        struct list_head entry;           /* entry in linked list of timers */
        unsigned long expires;            /* expiration value, in jiffies */
        void (*function)(unsigned long);  /* the timer handler function */
        unsigned long data;               /* lone argument to the handler */
        struct tvec_t_base_s *base;       /* internal timer field, do not touch */
};
```

The kernel provides a family of timer-related interfaces to make timer management easy. Everything is declared in `<linux/timer.h>`. Most of the actual implementation is in `kernel/timer.c`.

The first step in creating a timer is defining it:
```c
struct timer_list my_timer;
```

Next, the timer’s internal values must be initialized. This is done via a helper function and must be done prior to calling any timer management functions on the timer:
```c
init_timer(&my_timer);
```

Now you fill out the remaining values as required:
```c
my_timer.expires = jiffies + delay;   /* timer expires in delay ticks */
my_timer.data = 0;                    /* zero is passed to the timer handler */
my_timer.function = my_function;      /* function to run when timer expires */
```

As you can see from the `timer_list definition`, the function must match this prototype:
```c
void my_timer_function(unsigned long data);
```
The data parameter enables you to register multiple timers with the same handler, and differentiate between them via the argument. If you do not need the argument, you can simply pass zero.

Finally, you activate the timer:
```c
add_timer(&my_timer);
```

Typically, timers are run fairly close to their expiration; however, they might be delayed until the first timer tick after their expiration. Consequently, timers cannot be used to implement any sort of hard real-time processing.

The kernel implements a function, `mod_timer()`, which changes the expiration of a given timer:
```c
mod_timer(&my_timer, jiffies + new_delay);    /* new expiration */
```
The `mod_timer()` function can operate on timers that are initialized but not active, too. If the timer is inactive, `mod_timer()` activates it. The function returns zero if the timer were inactive and one if the timer were active. In either case, upon return from `mod_timer()`, the timer is activated and set to the new expiration.

If you need to deactivate a timer prior to its expiration, use the `del_timer()` function:
```c
del_timer(&my_timer);
```
The function works on both active and inactive timers. If the timer is already inactive, the function returns zero; otherwise, the function returns one. Note that you do not need to call this for timers that have expired because they are automatically deactivated.

A potential race condition that must be guarded against exists when deleting timers. When `del_timer()` returns, it guarantees only that the timer is no longer active. On a multiprocessing machine, however, the timer handler might already be executing on another processor. To deactivate the timer and wait until a potentially executing handler for the timer exits, use `del_timer_sync()`:
```c
del_timer_sync(&my_timer);
```

Unlike `del_timer()`, `del_timer_sync()` cannot be used from interrupt context. In almost all cases, you should use `del_timer_sync()` over `del_timer()`.

### Timer Implementation
After the timer interrupt, the timer interrupt handler runs `update_process_times()`, which calls `run_local_timers()`:
```c
void run_local_timers(void)
{
        hrtimer_run_queues();
        raise_softirq(TIMER_SOFTIRQ);   /* raise the timer softirq */
        softlockup_tick();
}
```
The `TIMER_SOFTIRQ` softirq is handled by `run_timer_softirq()`. This function runs all the expired timers on the current processor.

The kernel partitions timers into five groups based on their expiration value for efficience.


## Delaying Execution
### Busy Looping
`Busy looping` onlyy used when the time you want to delay is some integer multiple of the tick rate on precision is not important.
```c
unsigned long timeout = jiffies + 10;     /* ten ticks */

while (time_before(jiffies, timeout))
        ;
```
The loop continues until jiffies is larger than delay , which occurs only after 10 clock ticks have passed.

A better solution would be to reschedule your process to allow the processor to accomplish other work while your code waits:
```c
unsigned long delay = jiffies + 5*HZ;

while (time_before(jiffies, delay))
        cond_resched();
```
The call to `cond_resched()` schedules a new process, but only if need_resched is set. Note that because this approach invokes the scheduler, you can only make use of it from process context.

### Small Delays
The kernel provides three functions for microsecond, nanosecond, and millisecond delays, defined in `<linux/delay.h>` and `<asm/delay.h>`, which do not use `jiffies`:
```c
void udelay(unsigned long usecs)
void ndelay(unsigned long nsecs)
void mdelay(unsigned long msecs)
```

### schedule_timeout()
A more optimal method of delaying execution is to use `schedule_timeout()`. This call puts your task to sleep until at least the specified time has elapsed.
```c
/* set task’s state to interruptible sleep */
set_current_state(TASK_INTERRUPTIBLE);

/* take a nap and wake up in “s” seconds */
schedule_timeout(s * HZ);
```
If the code does not want to process signals, you can use `TASK_UNINTERRUPTIBLE` instead. The task must be in one of these two states before `schedule_timeout()` is called or else the task will not go to sleep.

##### schedule_timeout() Implementation
```c
signed long schedule_timeout(signed long timeout)
{
        timer_t timer;
        unsigned long expire;

        switch (timeout)
        {
          case MAX_SCHEDULE_TIMEOUT:
                  schedule();
                  goto out;
          default:
                  if (timeout < 0)
                  {
                          printk(KERN_ERR “schedule_timeout: wrong timeout “
                              “value %lx from %p\n”, timeout,
                              __builtin_return_address(0));
                          current->state = TASK_RUNNING;
                          goto out;
                  }
        }

        expire = timeout + jiffies;

        init_timer(&timer);
        timer.expires = expire;
        timer.data = (unsigned long) current;
        timer.function = process_timeout;

        add_timer(&timer);
        schedule();
        del_timer_sync(&timer);

        timeout = expire - jiffies;
out:
        return timeout < 0 ? 0 : timeout;
}
```

When the timer expires, it runs `process_timeout()`:
```c
void process_timeout(unsigned long data)
{
        wake_up_process((task_t *) data);
}
```
This function puts the task in the `TASK_RUNNING` state and places it back on the runqueue.

##### Sleeping on a Wait Queue, with a Timeout
Sometimes it is desirable to wait for a specific event or wait for a specified time to elapse. In those cases, code might simply call `schedule_timeout()` instead of schedule() after placing itself on a wait queue.
