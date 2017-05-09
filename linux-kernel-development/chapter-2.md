# Chapter 2: Getting Started with the Kernel

## Obtaining the Kernel Source
The current Linux source code is always available in both a complete tarball (an archive created with the tar command) and an incremental patch from the official home of the Linux kernel, http://www.kernel.org.

Use Git to get a copy:
```sh
$ git clone git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux-2.6.git
```

Or download a tarball and then uncompress it:
```sh
$ tar xvjf linux-x.y.z.tar.bz2
```

The kernel source si typically installed in `/usr/src/linux`.

Using patches:
```sh
$ patch -p1 < ../patch-x.y.z
```
Generally, a patch to a given version of the kernel is applied against the previous version.

## The Kernel Source Tree
Directory | Description
--------- | -----------
arch | Architecture-specific source
block | Block I/O layer
crypto | Crypto API
Documentation | Kernel source documentation
drivers | Device drivers
firmware | Device firmware needed to use certain drivers
fs | The VFS and the individual filesystems
include | Kernel headers
init | Kernel boot and initialization
ipc | Interprocess communication code
kernel | Core subsystems, such as the scheduler
lib | Helper routines
mm | Memory management subsystem and the VM
net | Networking subsystem
samples | Sample, demonstrative code
scripts | Scripts used to build the kernel
security | Linux Security Module
sound | Sound subsystem
usr | Early user-space code (called initramfs)
tools | Tools helpful for developing Linux
virt | Virtualization infrastructure

## Building the Kernel
### Config the Kernel
Creates a configuration by yourself:
```sh
$ make config   #text-based
$ make menuconfig    #ncurses-based
$ make gconfig    #gtk+-based
```
Creates a configuration based on the defaults for your architecture:
```sh
$ make defconfig
```

After making changes to your configuration file, or when using an existing configuration file on a new kernel tree, you can validate and update the configuration:
```sh
$ make oldconfig
```
You should always run this before Building a kernel.

After the kernel configuration is set, you can build it with a single command:
```sh
$ make
```

### Spawning Multiple Build Jobs
To build the kernel with multiple make jobs:
```sh
$ make -jn
```
Here, `n` is the number of jobs to spawn.

### Installing the New Kernel
How it is installed is architecture- and boot loader-dependent.
As an example, on an x86 system using grub, you would copy `arch/i386/boot/bzImage` to `/boot`, name it something like `vmlinuz- version`, and edit `/boot/grub/grub.conf`, adding a new entry for the new kernel. Systems using `LILO` to boot would instead edit `/etc/lilo.conf` and then rerun `lilo`.
As root, simply run:
```sh
% make modules_install
```

## A Beast of a Different Nature
- The kernel has access to neither the C library nor the standard C headers.
- The kernel is coded in GNU C.
- The kernel lacks the memory protection afforded to user-space.
- The kernel cannot easily execute floating-point operations.
- The kernel has a small per-process fixed-size stack.
- Because the kernel has asynchronous interrupts, is preemptive, and supports SMP, synchronization and concurrency are major concerns within the kernel.
- Portability is important.

### No libc or Standard Headers
Unlike a user-space application, the kernel is not linked against the standard C library, or any other library. The primary reason is speed and size.

Many of the usual libc functions are implemented inside the kernel. For example, the common string manipulation functions are the `lib/string.c`. Just include the header file `<linux/string.h>` and have at them.

The base files are located in the `include/` directory in the root of the kernel source tree. A set of Architecture-specific header files are located in `arch/<architecture>/include/asm` in the kernel source tree.

Of the missing functions, the most familiar is `printf()`. The kernel does not have access to `printf()`, but it does provide `printk()`, which works pretty much the same as its more familiar cousin. The `printk()` function copies the formatted string into the kernel log buffer, which is normally read by the `syslog` program. Usage is similar to :
```c
printk("Hello world! A string '%s' and an integer '%d'\n", str, i);
```
One notable difference between `printf()` and `printk()` is that `printk()` enables you to specify a priority flag. This flag is used by `syslogd` to decide where to display kernel messages.
```c
printk(KERN_ERR "this is an error!\n");
```
Note there is no comma between `KERN_ERR` and the printed message. The priority flag is a preprocessor-define representing a string literal, which is concatenated onto the printed message during compilation.

### GNU C
The kernel is not programmed in strict ANSI C. The kernel developers use both ISO C99 and GNU C extensions to the C language.

##### Inline Functions
An inline function is inserted inline into each function call site. This eliminates the overhead of function invocation and return (register saving and restore) and allows for potentially greater optimization as the compiler can optimize both the caller and called function as one. Kernel developers use inline functions for small time-critical functions.

An inline function is declared when the keywords `static` and `inline` are used as part of the function definition. For example:
```c
static inline void wolf(unsigned long tail_size)
```

##### Inline Assembly
The gcc C compiler enables the embedding of assembly instructions in otherwise normal C functions.

The `asm()` compiler directive is used to inline assembly code:
```c
unsigned int low, high;
asm volatile("rdtsc" : "=a" (low), "=d" (high));
/* low and high now contain the lower and upper 32-bits of the 64-bit tsc */
```

##### Branch Annotation
The gcc C compiler has a built-in directive that optimizes conditional branches as either very likely taken or very unlikely taken.The compiler uses the directive to appropriately optimize the branch.The kernel wraps the directive in easy-to-use macros, `likely()` and `unlikely()`.
```c
/* we predict 'error' is nearly always zero ... */
if (unlikely(error)) {
/* ... */
}

/* we predict 'success' is nearly always nonzero ... */
if (likely(success)) {
/* ... */
}
```

### No Memory Protection
When a user-space application attempts an illegal memory access, the kernel can trap the error, send the `SIGSEGV` signal, and kill the process. If the kernel attempts an illegal memory access, however, the results are less controlled. Memory violations in the kernel result in an oops, which is a major kernel error.

### No (Easy) Use of Floating Point
Unlike user-space, the kernel does not have the luxury of seamless support for floating point because it cannot easily trap itself. Using a floating point inside the kernel requires manually saving and restoring the floating point registers. Except in the rare cases, no floating-point operations are in the kernel.

### Small, Fixed-Size Stack
User-space has a large stack that can dynamically grow. The kernel stack is neither large nor dynamic; it is small and fixed in size. The exact size of the kernel's stack varies by architecture.

### Synchronization and Concurrency
A number of properties of the kernel allow for concurrent access of shared resources and thus require synchronization to prevent races:
- Linux is a preemptive multitasking operating system. Processes are scheduled and rescheduled at the whim of the kernel’s process scheduler.The kernel must synchronize between these tasks.
- Linux supports symmetrical multiprocessing (SMP).Therefore, without proper protection, kernel code executing simultaneously on two or more processors can concurrently access the same resource.
- Interrupts occur asynchronously with respect to the currently executing code. Therefore, without proper protection, an interrupt can occur in the midst of accessing a resource, and the interrupt handler can then access the same resource.
- The Linux kernel is preemptive. Therefore, without protection, kernel code can be preempted in favor of different code that then accesses the same resource.

Typical solutions to race conditions include spinlocks and semaphores.

### Importance of Protability
Linux is a portable operating system and should remain one. This means that architecture-independent C code must correctly compile and run on a wide range of systems, and that architecturedependent code must be properly segregated in system-specific directories in the kernel source tree.
