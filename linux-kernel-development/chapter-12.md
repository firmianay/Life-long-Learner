# Chapter 12: Memory Management

## Pages
The kernel treats physical pages as the basic unit of memory management. Most 32-bit architectures have 4KB pages, whereas most 64-bit architectures have 8KB pages.

The kernel represents every physical page on the system with a `struct page` structure. This structure is defined in `<linux/mm_types.h>`. The following is a simplified the definition:
```c
struct page {
    unsigned long flags;
    atomic_t _count;
    atomic_t _mapcount;
    unsigned long private;
    struct address_space *mapping;
    pgoff_t index;
    struct list_head lru;
    void *virtual;
};
```

The `flags` field stores the status of the page. Bit flags represent the various values, so at least 32 different flags are simultaneously available. The flag values are defined in `<linux/page-flags.h>`.

The `_count` field stores the usage count of the page. When this count reaches negative one, no one is using the page, and it becomes available for use in a new allocation. Kernel check this field with `page_count()`, it returns zero to indicate free and a positive nonzero integer when the page is in use.

The `virtual` field is the page's virtual address.

The important point to understand is that the `page` structure is associated with physical pages, not virtual pages. The data structure's goal is to describe physical memory, not the data contained therein.

An instance of this structure is allocated for each physical page in the system.


## Zones
The kernel divides pages into different zones and use the zones to group pages of similar properties.

Linux has to deal with two shortcomings of hardware with respect to memory addressing:
- Some hardware devices can perform DMA (direct memory access) to only certain memory addresses.
- Some architectures can physically addressing larger amounts of memory than they can virtually address. Consequently, some memory is not permanently mapped into the kernel address space.

Due to these contraints, Linux has four primary memory zones defined in `<linux/mmzone.h>`:
- `ZONE_DMA`: This zone contains pages that can undergo DMA.
- `ZONE_DMA32`: Like ZOME_DMA, this zone contains pages that can undergo DMA. Unlike ZONE_DMA, these pages are accessible only by 32-bit devices. On some architectures, this zone is a larger subset of memory.
- `ZONE_NORMAL`: This zone contains normal, regularly mapped, pages.
- `ZONE_HIGHMEM`: This zone contains "high memory", which are pages not permanently mapped into the kernel’s address space.

The layout of the memory zones is architecture-dependent. The following table is a listing of each zone and its consumed pages on x86-32:

Zone | Description | Physical Memory
---- | ----------- | ---------------
ZONE_DMA | DMA-able pages | < 16MB
ZONE_NORMAL | Normally addressable pages | 16–896MB
ZONE_HIGHMEM | Dynamically mapped pages | > 896MB

But Intel’s x86-64 can fully map and handle 64-bits of memory. Thus, x86-64 has no `ZONE_HIGHMEM` and all physical memory is contained within `ZONE_DMA` and `ZONE_NORMAL`.

Each zone is represented by `struct zone`, which is defined in `<linux/mmzone.h>`:
```c
struct zone {
    unsigned long watermark[NR_WMARK];
    unsigned long lowmem_reserve[MAX_NR_ZONES];
    struct per_cpu_pageset pageset[NR_CPUS];
    spinlock_t lock;
    struct free_area free_area[MAX_ORDER]
    spinlock_t lru_lock;
    struct zone_lru {
        struct list_head list;
        unsigned long nr_saved_scan;
    } lru[NR_LRU_LISTS];
    struct zone_reclaim_stat reclaim_stat;
    unsigned long pages_scanned;
    unsigned long flags;
    atomic_long_t vm_stat[NR_VM_ZONE_STAT_ITEMS];
    int prev_priority;
    unsigned int inactive_ratio;
    wait_queue_head_t *wait_table;
    unsigned long wait_table_hash_nr_entries;
    unsigned long wait_table_bits;
    struct pglist_data *zone_pgdat;
    unsigned long zone_start_pfn;
    unsigned long spanned_pages;
    unsigned long present_pages;
    const char *name;
};
```
The `lock` field is a spin lock that protects the structure from concurrent access. It protects just the structure and not all the pages that reside in the zone. A specific lock does not protect individual pages.

The `watermark` array holds the minimum, low, and high watermarks for this zone. The kernel uses watermarks to set benchmarks for suitable per-zone memory consumption.

The name `field` is a NULL-terminated string representing the name of this zone. The kernel initializes this value during boot in `mm/page_alloc.c`, and the three zones are given the names `DMA`, `Normal`, and `HighMem`.


## Getting Pages
The kernel provides one low-level mechanism for requesting memory, along with several interfaces to access it. All these interfaces allocate memory with page-sized granularity and are declared in `<linux/gfp.h>`. The core function is:
```c
struct page * alloc_pages(gfp_t gfp_mask, unsigned int order)
```
This allocates 2<sup>order</sup> (`1 << order`) contiguous physical pages and returns a pointer to the first page's `page` structure; on error it returns `NULL`.

You can convert a given page to its logical address with the function:
```c
void * page_address(struct page *page)
```
This returns a pointer to the logical address where the given physical page currently resides.

If you have no need for the actual `struct page`, you can call:
```c
unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order)
```
This function works the same as `alloc_pages()`, except that it directly returns the logical address of the first requested page. Because the pages are contiguous, the other pages simply follow from the first.

If you need only one page, two functions are implemented as wrappers:
```c
struct page * alloc_page(gfp_t gfp_mask)
unsigned long __get_free_page(gfp_t gfp_mask)
```
These functions work the same but pass zero for the order (2<sup>0</sup> = one page).

### Getting Zeroed Pages
If you need the returned page filled with zeros, use the function:
```c
unsigned long get_zeroed_page(unsigned int gfp_mask)
```
This function works the same as `__get_free_page()`, except that the allocated page is then zero-filled (every bit of every byte is unset).

A listing of all the low-level page allocation methods:

Flag | Description
---- | -----------
alloc_page(gfp_mask) | Allocates a single page and returns a pointer to its first page’s `page` structure
alloc_pages(gfp_mask, order) | Allocates 2<sup>order</sup> pages and returns a pointer to the first page’s `page` structure
__get_free_page(gfp_mask) | Allocates a single page and returns a pointer to its logical address
__get_free_pages(gfp_mask, order) | Allocates 2<sup>order</sup> pages and returns a pointer to the first page’s logical address
get_zeroed_page(gfp_mask) | Allocates a single page, zero its contents and returns a pointer to its logical address

### Freeing Pages
A family of functions enables you to free allocated pages when you no longer need them:
```c
void __free_pages(struct page *page, unsigned int order)
void free_pages(unsigned long addr, unsigned int order)
void free_page(unsigned long addr)
```
Be careful to free only pages you allocate. Passing the wrong struct page or address, or the incorrect order, can result in corruption.


## kmalloc()
The `kmalloc()` function obtaines kernel memory in byte-sized chunks. The function declared in `<linux/slab.h>`:
```c
void * kmalloc(size_t size, gfp_t flags)
```
The function returns a pointer to a region of memory that is at least `size` bytes in length. The region of memory allocated is physically contiguous. On error, it returns `NULL`.

### gfp_mask Flags
Flags are represented by the `gfp_t` type, which is defined in `<linux/types.h>` as an `unsigned int`. `gfp` stands for `__get_free_pages()`.

The flags are broken up into three categories:
- `action modifiers`: specify how the kernel is supposed to allocate the requested memory.
- `zone modifiers`: specify from where to allocate memory.
- `type`: specify a combination of action and zone modifiers for a certain type of memory alocation.

All the flags are declared in `<linux/gfp.h>`. The file `<linux/slab.h>` includes this header.

##### Action Modifiers

Flag | Description
---- | -----------
__GFP_WAIT | The allocator can sleep.
__GFP_HIGH | The allocator can access emergency pools.
__GFP_IO | The allocator can start disk I/O.
__GFP_FS | The allocator can start filesystem I/O.
__GFP_COLD | The allocator should use cache cold pages.
__GFP_NOWARN | The allocator does not print failure warnings.
__GFP_REPEAT | The allocator repeats the allocation if it fails, but the allocation can potentially fail.
__GFP_NOFAIL | The allocator indefinitely repeats the allocation. The allocation cannot fail.
__GFP_NORETRY | The allocator never retries if the allocation fails.
__GFP_NOMEMALLOC | The allocator does not fall back on reserves.
__GFP_HARDWALL | The allocator enforces "hardwall" cpuset boundaries.
__GFP_RECLAIMABLE | The allocator marks the pages reclaimable.
__GFP_COMP | The allocator adds compound.

##### Zone Modifiers
There are only three zone modifiers because there are only three zones other than `ZONE_NORMAL`, as in the following table:

Flag | Description
---- | -----------
__GFP_DMA | Allocates only from `ZONE_DMA`
__GFP_DMA32 | Allocates only from `ZONE_DMA32`
__GFP_HIGHMEM | Allocates from `ZONE_HIGHMEM` or `ZONE_NORMAL`

##### Type Flags
The table below is a list of the type flags:

Flag | Description
---- | -----------
GFP_ATOMIC | The allocation is high priority and must not sleep. This is the flag to use in interrupt handlers, in bottom halves, while holding a spinlock, and in other situations where you cannot sleep.
GFP_NOWAIT | Like `GFP_ATOMIC`, except that the call will not fallback on emergency memory pools. This increases the liklihood of the memory allocation failing.
GFP_NOIO | This allocation can block, but must not initiate disk I/O. This is the flag to use in block I/O code when you cannot cause more disk I/O, which might lead to some unpleasant recursion.
GFP_NOFS | This allocation can block and can initiate disk I/O, if it must, but it will not initiate a filesystem operation. This is the flag to use in filesystem code when you cannot start another filesystem operation.
GFP_KERNEL | This is a normal allocation and might block. This is the flag to use in process context code when it is safe to sleep. The kernel will do whatever it has to do to obtain the memory requested by the caller. This flag should be your default choice.
GFP_USER | This is a normal allocation and might block. This flag is used to allocate memory for user-space processes.
GFP_HIGHUSER | This is an allocation from `ZONE_HIGHMEM` and might block. This flag is used to allocate memory for user-space processes.
GFP_DMA | This is an allocation from `ZONE_DMA`. Device drivers that need DMA-able memory use this flag, usually in combination with one of the preceding flags.

The following table shows which modifiers are associated with each type flag:

Flag | Modifier Flags
---- | --------------
GFP_ATOMIC |  <code>__GFP_HIGH</code>
GFP_NOWAIT | <code>0<code>
GFP_NOIO | <code>__GFP_WAIT<code>
GFP_NOFS | <code>(&#95;&#95;GFP_WAIT &#124; &#95;&#95;GFP_IO)</code>
GFP_KERNEL | <code>(&#95;&#95;GFP_WAIT &#124; &#95;&#95;GFP_IO &#124; &#95;&#95;GFP_FS)</code>
GFP_USER | <code>(&#95;&#95;GFP_WAIT &#124; &#95;&#95;GFP_IO &#124; &#95;&#95;GFP_FS)</code>
GFP_HIGHUSER | <code>(&#95;&#95;GFP_WAIT &#124; &#95;&#95;GFP_IO &#124; &#95;&#95;GFP_FS &#124; &#95;&#95;GFP_HIGHMEM)</code>
GFP_DMA | <code>__GFP_DMA</code>

Below is a list of the common situations and the flags to use.

Situation | Solution
--------- | --------
Process context, can sleep | Use `GFP_KERNEL`.
Process context, cannot sleep | Use `GFP_ATOMIC`, or perform your allocations with `GFP_KERNEL` at an earlier or later point when you can sleep.
Interrupt handler | Use `GFP_ATOMIC`.
Softirq | Use `GFP_ATOMIC`.
Tasklet | Use `GFP_ATOMIC`.
Need DMA-able memory, can sleep | Use <code>(GFP_DMA  &#124; GFP_KERNEL)</code>.
Need DMA-able memory, cannot sleep | Use <code>(GFP_DMA &#124; GFP_ATOMIC)</code>, or perform your allocation at an earlier point when you can sleep.

### kfree()
The counterpart to `kmalloc()` is `kfree()`, declared in `<linux/slab.h>`:
```c
void kfree(const void *ptr)
```
The `kfree()` method frees a block of memory previously allocated with `kmalloc()`.


## vmalloc()
The `vmalloc()` function works in a similar fashion to `kmalloc()`, except `vmalloc()` allocates memory that is only virtually contiguous and not necessarily physically contiguous. This is similar to user-space `malloc()`.

Most kernel code uses `knalloc()` and not `vmalloc()` to obtain memory for performance. `vmalloc()` is used only when absolutely necessary to obtain large regions of memory.

The `vmalloc()` function is declared in <linux/vmalloc.h> and defined in `mm/vmalloc.c`:
```c
void * vmalloc(unsigned long size)
```
The function returns a pointer to at least `size` bytes of virtually contiguous memory. On error, the function returns `NULL`.

To free an allocation obtained via `vmalloc()`:
```c
void vfree(const void *addr)
```

## Slab Layer
