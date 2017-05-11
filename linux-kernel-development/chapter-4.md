# Chapter 4: Process Scheduling

`process scheduler`: Divides the finite resource of processor time between the runnable processes on a system. The scheduler is responsible for best utilizing the system and giving users the impression that multiple processes are executing simultaneously.


## Multitasking
A `multitasking` operating system is one that can simultaneously interleave execution of more than one process.

Multitasking operating systems come in two flavors: `cooperative multitasking`and `preemptive multitasking`. Linux implements preemptive multitasking.

`preemption`: the act of involuntarily suspending a running process.

`timeslice` of a process: the time the process runs before it is preempted is usually predetermined.


## Linux's Process Scheduler
Beginning in the 2.6 kernel series, developers introduced new process schedulers aimed at improving the interactive performance of the `O(1)` scheduler. The most notable of these was the `Rotating Staircase Deadline scheduler`, which introduced the concept of fair scheduling, borrowed from queuing theory, to Linux's process scheduler. This concept was the inspiration for the O(1) scheduler's eventual replacement in kernel version 2.6.23, the `Completely Fair Scheduler (CFS)`.


## Policy
Policy is the behavior of the scheduler that determines what runs when.

### I/O-Bound Versus Processor-Bound Processes
Processes can be classified as either `I/O-bound` or `processor-bound`.
- `I/O-bound processes`: spend much of its time submitting and waiting on I/O requests.(eg. GUI)
- `processor-bound processes`:  spend much of their time executing code.(eg. MATLAB)

The scheduling policy in a system must attempt to satisfy two conflicting goals: fast process response time (low latency) and maximal system utilization (high throughput). Linux, aiming to provide good interactive response and desktop performance, optimizes for process response (low latency), thus favoring I/O-bound processes over processor-bound processes.

### Process Priority
A common type of scheduling algorithm is `priority-based` scheduling.

The Linux kernel implements two separate priority ranges:
- `nise value`: a number from -20 to +19 with a default of 0. Larger nice values correspond to a lower priority. In Linux, it is a control over the `proportion` of timeslice.
- `real-time priority`: configurable values that by default range from 0 to 99. Higher real-time priority values correspond to a greater priority.

### Timeslice
The timeslice is the numeric value that represents how long a task can run until it is preempted.

Linux's CFS scheduler does not directly assign timeslices to processes, but assigns processes a `proportion` of the processor. Under the new CFS scheduler, the decision is a function of how much of a proportion of the processor the newly runnable processor has consumed. If it has consumed a smaller proportion of the processor than the currently executing process, it runs immediately, preempting the current process. If not, it is scheduled to run at a later time.


## The Linux Scheduling Algorithm
### Scheduler Classes
The Linux scheduler is modular, enabling different algorithms to schedule different types of processes. This modularity is called `scheduler classes`. The base scheduler code, which is defined in `kernel/sched.c`, iterates over each scheduler class in order of priority.The highest priority scheduler class that has a runnable process wins, selecting who runs next.

The Completely Fair Scheduler (CFS) is the registered scheduler class for normal processes, called `SCHED_NORMAL` in Linux. CFS is defined in `kernel/sched_fair.c`.

### Fair Scheduling
CFS is based on a simple concept: Model process scheduling as if the system had an ideal, perfectly multitasking processor.

Put generally, the proportion of processor time that any process receives is determined only by the relative difference in niceness between it and the other runnable processes. The nice values, instead of yielding additive increases to timeslices, yield geometric differences.The absolute timeslice allotted any nice value is not an absolute number, but a given proportion of the processor. CFS is called a fair scheduler because it gives each process a fair share (a proportion) of the processor’s time.
