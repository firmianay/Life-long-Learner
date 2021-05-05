# Chapter 9: An Introduction to Kernel Synchronization

## Critical Regions and Race Conditions

Code paths that access and manipulate shared data are called `critical regions`.

It is a bug if it is possible for two threads of execution to be simultaneously executing within the same critical region. When this occur, it is called a `race condition`.

## Locking

What is needed is a way of making sure that only one thread manipulates the data structure at a time—a mechanism for preventing access to a resource while another thread of execution is in the marked region. A lock provides such a mechanism.

Linux alone implements a handful of different locking mechanisms.The most significant difference between the various mechanisms is the behavior when the lock is unavailable because another thread already holds it.

### Causes of Concurrency

Causes of concurrency in kernel:

- `Interrupts`: An interrupt can occur asynchronously at almost any time, interrupting the currently executing code.
- `Softirqs and tasklets`: The kernel can raise or schedule a softirq or tasklet at almost any time, interrupting the currently executing code.
- `Kernel preemption`: Because the kernel is preemptive, one task in the kernel can preempt another.
- `Sleeping and synchronization with user-space`: A task in the kernel can sleep and thus invoke the scheduler, resulting in the running of a new process.
- `Symmetrical multiprocessing`: Two or more processors can execute kernel code at exactly the same time.

With a clear picture of what data needs protection, it is not hard to provide the locking to keep the system stable. Rather, the hard part is identifying these conditions and realizing that to prevent concurrency, you need some form of protection. So, always design proper locking into your code from the beginning.

### Knowing What to Protect

Most global kernel data structures require locking. A good rule of thumb is that if another thread of execution can access the data, the data needs some sort of locking; if anyone else can see it, lock it. Remember to lock data, not code.

Ask yourself these questions whenever you write kernel code:

- Is the data global? Can a thread of execution other than the current one access it?
- Is the data shared between process context and interrupt context? Is it shared between two different interrupt handlers?
- If a process is preempted while accessing this data, can the newly scheduled process access the same data?
- Can the current process sleep (block) on anything? If it does, in what state does that leave any shared data?
- What prevents the data from being freed out from under me?
- What happens if this function is called again on another processor?
- Given the proceeding points, how am I going to ensure that my code is safe from concurrency?

## Deadlocks

A `deadlock` is a condition involving one or more threads of execution and one or more resources, such that each thread waits for one of the resources, but all the resources are already held. The threads all wait for each other, but they never make any progress toward releasing the resources that they already hold. Therefore, none of the threads can continue.

The simplest example of a deadlock is the self-deadlock. If a thread of execution attempts to acquire a lock it already holds, it has to wait for the lock to be released. But it will never release the lock, because it is busy waiting for the lock, and the result is deadlock:

```text
acquire lock
acquire lock, again
wait for lock to become available
...
```

Similarly, consider n threads and n locks. If each thread holds a lock that the other thread wants, all threads block while waiting for their respective locks to become available. The most common example is with two threads and two locks, which is often called the deadly embrace or the `ABBA` deadlock:

Thread 1 | Thread 2
-------- | --------
acquire lock A | acquire lock B
try to acquire lock B | try to acquire lock A
wait for lock B | wait for lock A

you can write deadlock-free code following the rules below:

- Implement lock ordering. Nested locks must always be obtained in the same order.
- Prevent starvation.
- Do not double acquire the same lock.
- Design for simplicity.

The first point is most important and worth stressing. If two or more locks are acquired at the same time, they must always be acquired in the same order.

## Contention and Scalability

The term `lock contention`, or simply `contention`, describes a lock currently in use but that another thread is trying to acquire. A lock that is `highly contended` often has threads waiting to acquire it.

`Scalability` is a measurement of how well a system can be expanded.

The granularity of locking is a description of the size or amount of data that a lock protects.

 Start simple and grow in complexity only as needed. Simplicity is key.
