# Chapter 5: System Calls

The kernel provides a set of interfaces by which processes running in user-space can interact with the system.

## Communicating with the Kernel

System calls provide a layer between the hardware and user-space processes, which serves three primary purposes:

- Providing an abstracted hardware interface for userspace.
- Ensuring system security and stability.
- A single common layer between user-space and the rest of the system allows for the virtualized system provided to processes.

In Linux, system calls are the only means user-space has of interfacing with the kernel and the only legal entry point into the kernel other than exceptions and traps.

## API, POSIX, and the C Library

Applications are typically programmed against an Application Programming Interface (API) implemented in user-space, not directly to system calls, because no direct correlation is needed between the interfaces used by applications and the actual interface provided by the kernel.

The relationship between a POSIX API, the C library, and system calls:

![img](./pic/ch5_1.png)

POSIX is composed of a series of standards from the IEEE that aim to provide a portable operating system standard roughly based on Unix. On most Unix systems, the POSIX-defined API calls have a strong correlation to the system calls.

The system call interface in Linux, as with most Unix systems, is provided in part by the C library. The C library implements the main API on Unix systems, including the standard C library and the system call interface. The C library additionally provides the majority of the POSIX API.

A meme related to interfaces in Unix is "Provide mechanism, not policy". In other words, Unix system calls exist to provide a specific function in an abstract sense. The manner in which the function is used is not any of the kernel’s business.

## Syscalls

System calls (often called syscalls in Linux) are typically accessed via function calls defined in the C library. System calls have a defined behavior.

### System Call Numbers

In Linux, each system call is assigned a unique `syscall number` that is used to reference a specific system call. When a user-space process executes a system call, the syscall number identifies which syscall was executed; the process does not refer to the syscall by name.

The kernel keeps a list of all registered system calls in the system call table, stored in `sys_call_table`, on x86-64 it is defined in `arch/x86/kernel/syscall_64.c`.

### System Call Performance

System calls in Linux are faster than in many other operating systems, because of:

- Linux’s fast context switch times: entering and exiting the kernel is a streamlined and simple affair.
- Simplicity of the system call handler and the individual system calls themselves.

## System Call Handler

It is not possible for user-space applications to execute kernel code directly. They cannot simply make a function call to a method existing in kernel-space because the kernel exists in a protected memory space.

The mechanism to signal the kernel is a software interrupt: Incur an exception, and the system will switch to kernel mode and execute the exception handler. The exception handler is actually the system call handler. The defined software interrupt on x86 is interrupt number 128, which is incurred via the int $0x80 instruction. The system call handler is the aptly named function `system_call()`.

### Denoting the Correct System Call

Simply entering kernel-space alone is not sufficient: the system call number must be passed into the kernel.

On x86, the syscall number is passed to the kernel via the `eax` register.

The `system_call()` function checks the validity of the given system call number by comparing it to `NR_syscalls`. If it is larger than or equal to `NR_syscalls`, the function returns `-ENOSYS`. Otherwise, the specified system call is invoked:

```c
call *sys_call_table(,%rax,8)
```

Because each element in the system call table is 64 bits (8 bytes), the kernel multiplies the given system call number by eight to arrive at its location in the system call table. On x86-32, the code is similar, with the 8 replaced by 4.

![img](./pic/ch5_2.png)

### Parameter Passing

In addition to the system call number, most syscalls require that one or more parameters be passed to them. The parameters are stored in registers. On x86-32, the registers `ebx`, `ecx`, `edx`, `esi`, and `edi` contain, in order, the first five arguments. In the unlikely case of six or more arguments, a single register is used to hold a pointer to user-space where all the parameters are stored.

The return value is sent to user-space also via register. On x86, it is written into the `eax` register.

## System Call Implementation

### Implementing System Calls

The first step in implementing a system call is defining its purpose and the syscall should have exactly one purpose. Multiplexing syscalls (a single system call that does wildly different things depending on a flag argument) is discouraged in Linux. `ioctl()` is an example of what not to do.

The system call should have a clean and simple interface with the smallest number of arguments possible. They must not change.

### Verifying the Parameters

System calls must carefully verify all their parameters to ensure that they are valid, legal and correct to guarantee the system’s security and stability.

One of the most important checks is the validity of any pointers that the user provides. Before following a pointer into user-space, the system must ensure that:

- The pointer points to a region of memory in user-space. Processes must not be able to trick the kernel into reading data in kernel-space on their behalf.
- The pointer points to a region of memory in the process’s address space. The process must not be able to trick the kernel into reading someone else’s data.
- The process must not be able to bypass memory access restrictions. If reading, the memory is marked readable. If writing, the memory is marked writable. If executing, the memory is marked executable.

The kernel provides two methods for performing the requisite checks and the desired copy to and from user-space:

- `copy_to_user()`: Writing into user-space. It takes three parameters, The first is the destination memory address in the process’s address space. The second is the source pointer in kernel-space. The third argument is the size in bytes of the data to copy.
- `copy_from_user()`: Reading from user-space. The function reads from the second parameter into the first parameter the number of bytes specified in the third parameter.

Both of these functions return the number of bytes they failed to copy on error. On success, they return zero. It is standard for the syscall to return `-EFAULT` in the case of such an error.

A final possible check is for valid permission. The new system enables specific access checks on specific resources. A call to `capable()` with a valid capabilities flag returns nonzero if the caller holds the specified capability and zero otherwise. By default, the superuser possesses all capabilities and nonroot possesses none. See `<linux/capability.h>` for a list of all capabilities and what rights they entail.

## System Call Context

The kernel is in process context during the execution of a system call. The `current` pointer points to the current task, which is the process that issued the syscall. In process context, the kernel is capable of sleeping and is fully preemptible. When the system call returns, control continues in `system_call()`, which ultimately switches to user-space and continues the execution of the user process.

### Final Steps in Binding a System Call

It is trivial to register an official system call after it is written:

1. Add an entry to the end of the system call table (for most architectures, `entry.S`). This needs to be done for each architecture that supports the system call. The position of the syscall in the table, starting at zero, is its system call number.
2. For each supported architecture, define the syscall number in `<asm/unistd.h>`.
3. Compile the syscall into the kernel image (as opposed to compiling as a module). This can be as simple as putting the system call in a relevant file in `kernel/`, such as `sys.c`, which is home to miscellaneous system calls.

### Accessing the System Call from User-Space

The C library provides support for system calls. User applications can pull in function prototypes from the standard headers and link with the C library to use your system call.

Linux provides a set of macros for wrapping access to system calls. It sets up the register contents and issues the trap instructions. These macros are named `_syscalln()`, where n is between 0 and 6. The number corresponds to the number of parameters passed into the syscall, because the macro needs to know how many parameters to push into registers.

For example, consider the system call `open()`, defined as:

```c
long open(const char *filename, int flags, int mode)
```

The syscall macro to use this system call without explicit library support would be:

```c
#define __NR_open 5
_syscall3(long, open, const char *, filename, int, flags, int, mode)
```

The application can simply call `open()`.

For each macro, there are 2 + 2 × n parameters.The first parameter corresponds to the return type of the syscall. The second is the name of the system call. Next follows the type and name for each parameter in order of the system call. The `__NR_open` define is in `<asm/unistd.h>`; it is the system call number. The `_syscall3` macro expands into a C function with inline assembly; the assembly performs the steps discussed in the previous section to push the system call number and parameters into the correct registers and issue the software interrupt to trap into the kernel.

### Why Not to Implement a System Call

The pros of implementing a new interface as a syscall:

- System calls are simple to implement and easy to use.
- System call performance on Linux is fast.

The cons:

- You need a syscall number, which needs to be officially assigned to you.
- After the system call is in a stable series kernel, it is written in stone.The interface cannot change without breaking user-space applications.
- Each architecture needs to separately register the system call and support it.
- System calls are not easily used from scripts and cannot be accessed directly from the filesystem.
- Because you need an assigned syscall number, it is hard to maintain and use a system call outside of the master kernel tree.
- For simple exchanges of information, a system call is overkill.

The alternatives:

- Implement a device node and `read()` and `write()` to it. Use `ioctl()` to manipulate specific settings or retrieve specific information.
- Certain interfaces, such as semaphores, can be represented as file descriptors and manipulated as such.
- Add the information as a file to the appropriate location in sysfs.
