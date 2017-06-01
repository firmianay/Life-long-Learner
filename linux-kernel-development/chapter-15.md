# Chapter 15: The Process Address Space
The kernel also has to manage the memory of user-space process. This memory is called the `process address space`, which is the representation of memory given to each user-space process on the system. Linux is a virtual memory operating system, and thus the resource of memory is virtualized among the processes on the system.

## Address Space
