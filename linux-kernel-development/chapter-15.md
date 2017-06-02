# Chapter 15: The Process Address Space
The kernel also has to manage the memory of user-space process. This memory is called the `process address space`, which is the representation of memory given to each user-space process on the system. Linux is a virtual memory operating system, and thus the resource of memory is virtualized among the processes on the system.

## Address Space
The process address space consists of the virtual memory addressable by a process and the addresses within the virtual memory that the process is allowed to use. Each process is given a `flat` address space that is unique to each process. A memory address in another process's address space. Both processes can have different data at the same address space in their respective address space. Alternatively, processes can elect to share their address space with other processes. We know these processes as `threads`.

A memory address is a given value within the address space. Intervals of legal address are called `memory areas`. The process, through the kernel, can dynamically add and remove memory areas to its address space.

The process can access a memory address only in a valid memory area. Memory areas have associated permissions, such as readable, writable, and executable, that the associated process must respect.

Memory areas can contain all sorts of goodies:
- A memory map of the executable file's code, called the `text section`.
- A memory map of the executable file's initialized global variables, called the `data section`.
- A memory map of the zero page containing uninitialized global variables, called the `bss section`.
- A memory map of the zero page used for the process's user-space stack.
- An additional text, data, and bss section for each shared library, such as the C library and dynamic linker, loaded into the process's address space.
- Any memory mapped files.
- Any shared memory segments.
- Any anonymous memory mappings, such as those associated with `malloc()`.

All valid addresses in the process address space exist in exactly one area; memory areas do not overlap.

## The Memory Descriptor
The kernel represents a process's address space with a data structure called the `memory descriptor`. This structure contains all the information related to the process address space.

The memory descriptor is represented by `struct mm_struct` and defined in `<linux/mm_types.h>`:
```
struct mm_struct {
    struct vm_area_struct     *mmap;            /* list of memory areas */
    struct rb_root            mm_rb;            /* red-black tree of VMAs */
    struct vm_area_struct     *mmap_cache;      /* last used memory area */
    unsigned long             free_area_cache;  /* 1st address space hole */
    pgd_t                     *pgd;             /* page global directory */
    atomic_t                  mm_users;         /* address space users */
    atomic_t                  mm_count;         /* primary usage counter */
    int                       map_count;        /* number of memory areas */
    struct rw_semaphore       mmap_sem;         /* memory area semaphore */
    spinlock_t                page_table_lock;  /* page table lock */
    struct list_head          mmlist;           /* list of all mm_structs */
    unsigned long             start_code;       /* start address of code */
    unsigned long             end_code;         /* final address of code */
    unsigned long             start_data;       /* start address of data */
    unsigned long             end_data;         /* final address of data */
    unsigned long             start_brk;        /* start address of heap */
    unsigned long             brk;              /* final address of heap */
    unsigned long             start_stack;      /* start address of stack */
    unsigned long             arg_start;        /* start of arguments */
    unsigned long             arg_end;          /* end of arguments */
    unsigned long             env_start;        /* start of environment */
    unsigned long             env_end;          /* end of environment */
    unsigned long             rss;              /* pages allocated */
    unsigned long             total_vm;         /* total number of pages */
    unsigned long             locked_vm;        /* number of locked pages */
    unsigned long             saved_auxv[AT_VECTOR_SIZE]; /* saved auxv */
    cpumask_t                 cpu_vm_mask;      /* lazy TLB switch mask */
    mm_context_t              context;          /* arch-specific data */
    unsigned long             flags;            /* status flags */
    int                       core_waiters;     /* thread core dump waiters */
    struct core_state         *core_state;      /* core dump support */
    spinlock_t                ioctx_lock;       /* AIO I/O list lock */
    struct hlist_head         ioctx_list;       /* AIO I/O list */
};
```

### Allocating a Memory Descriptor
The memory descriptor associated with a given task is stored in the `mm` field of the task's process descriptor. Thus, `current->mm` is the current process's memory descriptor. The `copy_mm()` function copies a parent's memory descriptor to its child during `fork()`. The `mm_struct` structure is allocated from the `mm_cachep` slab cache via the `allocate_mm()` macro in `kernel/fork.c`. Normally, each process receives a unique `mm_struct` and thus a unique process address space.

Processes may share their address spaces with their children via the `CLONE_VM` flag to `clone()`. The process is then called a `thread`. This is essentially the only difference between normal processes and so-called threads in Linux; the Linux kernel does not otherwise differentiate between them. Threads are regular processes to the kernel that merely share certain resources. When `CLONE_VM` is specified, `allocate_mm()` is not called, and the process's `mm` field is set to point to the memory descriptor of its parent.

### Destroying a Memory Descriptor
When the process associated with a specific address space exits, `exit_mm()`, defined in `kernel/exit.c`, is invoked. This function performs some housekeeping and updates some statistics. It then calls `mmput()`, which decrements the memory descriptor's `mm_users` user counter. If the user count reaches zero, `mmdrop()` is called to decrement the `mm_count` usage counter. If that counter is finally zero, the `free_mm()` macro is invoked to return the `mm_struct` to the `mm_cachep` slab cache via `kmem_cache_free()`, because the memory descriptor does not have any users.

### The `mm_struct` and Kernel Threads
Kernel threads do not have a process address space and therefore do not have an associated memory descriptor. Thus, the `mm` field of a kernel thread's process descriptor is `NULL`. This is the definition of a kernel thread: processes that have no user context.

Kernel threads do not ever access any user-space memory and do not have any pages in user-space, they do not deserve their own memory descriptor and page tables. Despite this, kernel threads need some of the data, such as the page tables, even to access kernel memory. To provide kernel threads the needed data, without wasting memory on a memory descriptor and page tables, or wasting processor cycles to switch to a new address space whenever a kernel thread begins running, kernel threads use the memory descriptor of whatever task ran previously.

Whenever a process is scheduled, the process address space referenced by the process’s `mm` field is loaded. The `active_mm` field in the process descriptor is then updated to refer to the new address space. Kernel threads do not have an address space and mm is `NULL`. Therefore, when a kernel thread is scheduled, the kernel notices that `mm` is `NULL` and keeps the previous process’s address space loaded. The kernel then updates the `active_mm` field of the kernel thread’s process descriptor to refer to the previous process’s memory descriptor. The kernel thread can then use the previous process’s page tables as needed. Because kernel threads do not access user-space memory, they make use of only the information in the address space pertaining to kernel memory, which is the same for all processes.


## Virtual Memory Areas
The memory area structure, `vm_area_struct`, represents memory areas. It is defined in `<linux/mm_types.h>`. In the Linux kernel, memory areas are often called `virtual memory areas` (VMAs).

The `vm_area_struct` structure describes a single memory area over a contiguous interval in a given address space. The kernel treats each memory area as a unique memory object. Each memory area prossesses certain properties. In this manner, each VMA structure can represent different types of memory areas.
```
struct vm_area_struct {
    struct mm_struct        *vm_mm;         /* associated mm_struct */
    unsigned                long vm_start;  /* VMA start, inclusive */
    unsigned long           vm_end;         /* VMA end , exclusive */
    struct vm_area_struct   *vm_next;       /* list of VMA's */
    pgprot_t                vm_page_prot;   /* access permissions */
    unsigned long           vm_flags;       /* flags */
    struct rb_node          vm_rb;          /* VMA's node in the tree */
    union { /* links to address_space->i_mmap or i_mmap_nonlinear */
    struct {
            struct list_head        list;
            void                    *parent;
            struct vm_area_struct   *head;
        } vm_set;
        struct prio_tree_node prio_tree_node;
    } shared;
    struct list_head              anon_vma_node;    /* anon_vma entry */
    struct anon_vma               *anon_vma;        /* anonymous VMA object */
    struct vm_operations_struct   *vm_ops;          /* associated ops */
    unsigned long                 vm_pgoff;         /* offset within file */
    struct file                   *vm_file;         /* mapped file, if any */
    void                          *vm_private_data; /* private data */
};
```
Each memory descriptor is associated with a unique interval in the process's address space. Intervals in different memory areas in the same address space cannot overlap. The `vm_mm` field points to this VMA's associated `mm_struct`.

### VMA Flags
The `vm_flags` field contains bit flags, defined in `<linux/mm.h>`, that specify the behavior of and provide information about the pages contained in the memory area. `vm_flags` contains information that relates to the memory area as a whole (each page), not specific individual pages.

Flag | Effect on the VMA and Its Pages
---- | -------------------------------
VM_READ | Pages can be read from.
VM_WRITE | Pages can be written to.
VM_EXEC | Pages can be executed.
VM_SHARED | Pages are shared.
VM_MAYREAD | The `VM_READ` flag can be set.
VM_MAYWRITE | The `VM_WRITE` flag can be set.
VM_MAYEXEC | The `VM_EXEC` flag can be set.
VM_MAYSHARE | The `VM_SHARE` flag can be set.
VM_GROWSDOWN | The area can grow downward.
VM_GROWSUP | The area can grow upward.
VM_SHM | The area is used for shared memory.
VM_DENYWRITE | The area maps an unwritable file.
VM_EXECUTABLE | The area maps an executable file.
VM_LOCKED | The pages in this area are locked.
VM_IO | The area maps a device's I/O space.
VM_SEQ_READ | The pages seem to be accessed sequentially.
VM_RAND_READ | The pages seem to be accessed randomly.
VM_DONTCOPY | This area must not be copied on `fork()`.
VM_DONTEXPAND | This area cannot grow via `mremap()`.
VM_RESERVED | This area must not be swapped out.
VM_ACCOUNT | This area is an accounted VM object.
VM_HUGETLB | This area uses hugetlb pages.
VM_NONLINEAR | This area is a nonlinear mapping.

### VMA Operations
The `vm_ops` field in the `vm_area_struct` structure points to the table of operations associated with a given memory area, which the kernel can invoke to manipulate the VMA. The `vm_area_struct` acts as a generic object for representing any type of memory area, and the operations table describes the specific methods that can operate on this particular instance of the object.

The operations table is represented by struct `vm_operations_struct` and is defined in `<linux/mm.h>`:
```
struct vm_operations_struct {
    void (*open) (struct vm_area_struct *);
    void (*close) (struct vm_area_struct *);
    int (*fault) (struct vm_area_struct *, struct vm_fault *);
    int (*page_mkwrite) (struct vm_area_struct *vma, struct vm_fault *vmf);
    int (*access) (struct vm_area_struct *, unsigned long ,
    void *, int, int);
};
```

- `void open(struct vm_area_struct *area)`
 - This function is invoked when the given memory area is added to an address space.
- `void close(struct vm_area_struct *area)`
 - This function is invoked when the given memory area is removed from an address space.
- `int fault(struct vm_area_sruct *area, struct vm_fault *vmf)`
 - This function is invoked by the page fault handler when a page that is not present in physical memory is accessed.
- `int page_mkwrite(struct vm_area_sruct *area, struct vm_fault *vmf)`
 - This function is invoked by the page fault handler when a page that was read-only is being made writable.
- `int access(struct vm_area_struct *vma, unsigned long address, void *buf, int len, int write)`
 - This function is invoked by `access_process_vm()` when `get_user_pages()` fails.

### Lists and Trees of Memory Areas
Memory areas are accessed via both the `mmap` and the `mm_rb` field of the memory descriptor. These two data structures independently point to all the memory area objects associated with the memory descriptor. In fact, they both contain pointers to the same `vm_area_struct` structures, merely represented in different ways.
- `mmap`: links together all the memory area objects in a single linked list. Each `vm_area_struct` structure is linked into the list via its `vm_next` field. The areas are sorted by ascending address. The first memory area is the `vm_area_struct` structure to which `mmap` points.The last structure points to `NULL`.
- `mm_rb`: links together all the memory area objects in a red-black tree. The root of the red-black tree is `mm_rb`, and each `vm_area_struct` structure in this address space is linked to the tree via its `vm_rb` field.

The linked list is used when every node needs to be traversed. The red-black tree is used when locating a specific memory area in the address space.


## Manipulating Memory Areas
