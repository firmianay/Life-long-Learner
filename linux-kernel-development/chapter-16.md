# Chapter 16: The Page Cache and Page Writeback

The Linux kernel implements a disk cache called the `page cache`. The goal of this cache is to minimize disk I/O by storing data in physical memory that would otherwise require disk access.

Two factors comingle to make disk caches a critical component of any modern operating system:

- disk access is several orders of magnitude slower than memory access.
- data accessed once will, with a high likelihood, find itself accessed again in the near future. This principle is called `temporal locality`.

## Approaches to Caching

The page cache consists of physical pages in RAM, the contents of which correspond to physical blocks on a disk. The size of the page cache is dynamic. We call the storage device being cached the `backing store`. When ever the kernel begins a read operation, it first checks if the requisite data is in the page cache. If it is, the kernel can forgo accessing the disk and read the data directly out of RAM. This is called a `cache hit`. If the data is not in the cache, called a `cache miss`, the kernel must schedule block I/O operations to read the data off the disk. After the data is read off the disk, the kernel populates the page cache with the data so that any subsequent reads can occur out of the cache.

### Write Caching

When a process writes to disk, the strategy in Linux is called `write-back`. In a write-back cache, processes perform write operations directly into the page cache. The backing store is not immediately or directly updated. Instead, the written-to pages in the page cache are marked as `dirty` and are added to a `dirty list`. Periodically, pages in the dirty list are written back to disk in a process called `writeback`, bringing the on-disk copy in line with the in-memory cache. The pages are then marked as no longer dirty.

### Cache Eviction

The strategy that decides what to remove, is called `cache eviction`.

#### Least Recently Used

One of the more successful algorithms is called `least recently used`, or LRU. An LRU eviction strategy requires keeping track of when each page is accessed and evicting the pages with the oldest timestramp. However, one particular failure of the LRU strategy is that many files are accessed once and then never again.

#### The Two-List Strategy

Linux implements a modified version of LRU, called the `two-list strategy`. Linux keeps two list: the `active list` and the `inactive list`. Pages on the active list are considered "hot" and are not available for eviction. Pages on the inactive list are available for cache eviction. Pages are placed on the active list only when they are accessed `while already residing` on the inactive list. Both list are maintained in a pseudo-LRU manner: Items are added to the tail and removed from the head. The lists are kept in balance. This two-list approach is known as LRU/2, it can be generalized to n-lists, called LRU/n.

## The Linux Page Cache

### The `address_space` Object

The Linux page cache used a new Object to manage entries in the cache and page I/O operations. That Object is the `address_space` structure.

```c
struct address_space {
        struct inode            *host;              /* owning inode */
        struct radix_tree_root  page_tree;          /* radix tree of all pages */
        spinlock_t              tree_lock;          /* page_tree lock */
        unsigned int            i_mmap_writable;    /* VM_SHARED ma count */
        struct prio_tree_root   i_mmap;             /* list of all mappings */
        struct list_head        i_mmap_nonlinear;   /* VM_NONLINEAR ma list */
        spinlock_t              i_mmap_lock;        /* i_mmap lock */
        atomic_t                truncate_count;     /* truncate re count */
        unsigned long           nrpages;            /* total number of pages */
        pgoff_t                 writeback_index;    /* writeback start offset */
        struct address_space_operations   *a_ops;   /* operations table */
        unsigned long           flags;              /* gfp_mask and error flags */
        struct backing_dev_info *backing_dev_info;  /* read-ahead information */
        spinlock_t              private_lock;       /* private lock */
        struct list_head        private_list;       /* private list */
        struct address_space    *assoc_mapping;     /* associated buffers */
};
```

### `address_space` operations

The `a_ops` field points to the address space operations table, which is represented by `struct address_space_operations` and is defined in `<linux/fs.h>`:

```c
struct address_space_operations {
        int (*writepage)(struct page *, struct writeback_control *);
        int (*readpage) (struct file *, struct page *);
        int (*sync_page) (struct page *);
        int (*writepages) (struct address_space *,
                           struct writeback_control *);
        int (*set_page_dirty) (struct page *);
        int (*readpages) (struct file *, struct address_space *,
                          struct list_head *, unsigned);
        int (*write_begin)(struct file *, struct address_space *mapping,
                           loff_t pos, unsigned len, unsigned flags,
                           struct page **pagep, void **fsdata);
        int (*write_end)(struct file *, struct address_space *mapping,
                           loff_t pos, unsigned len, unsigned copied,
                           struct page *page, void *fsdata);
        sector_t (*bmap) (struct address_space *, sector_t);
        int (*invalidatepage) (struct page *, unsigned long);
        int (*releasepage) (struct page *, int);
        int (*direct_IO) (int, struct kiocb *, const struct iovec *,
                          loff_t, unsigned long);
        int (*get_xip_mem) (struct address_space *, pgoff_t, int,
                            void **, unsigned long *);
        int (*migratepage) (struct address_space *,
                            struct page *, struct page *);
        int (*launder_page) (struct page *);
        int (*is_partially_uptodate) (struct page *,
                                      read_descriptor_t *,
                                      unsigned long);
        int (*error_remove_page) (struct address_space *,
                                  struct page *);
};
```

These function points at the functions that implement page I/O for this cached object. Each backing store describes how it interacts with the page cache via its own `address_space_operations`. The `readpage()` and `writepage()` methods are most important.

Starting with a page read operation. First, the Linux kernel attempts to find the request data in the page cache. The `find_get_page()` method is used to perform this check, it is passed an `address_space` and page offset. These values search the page cache for the desired data:

```c
page = find_get_page(mapping, index);
```

Here, `mapping` is the given `address_space` and `index` is the desired offset into the file, in pages. If the page does not exist in the cache, `find_get_page` returns `NULL` and a new page is allocated and added to the page cache:

```c
struct page *page;
int error;

/* allocate the page ... */
page = page_cache_alloc_cold(mapping);
if (!page)
        /* error allocating memory */

/* ... and then add it to the page cache */
error = add_to_page_cache_lru(page, mapping, index, GFP_KERNEL);
if (error)
        /* error adding page to page cache */
```

Finally, the requested data can be read from disk, added to the page cache, and returned to the user:

```c
error = mapping->a_ops->readpage(file, page);
```

Write operations are a bit different. For file mappings, whenever a page is modified, the VM simply calls:

```c
SetPageDirty(page);
```

The kernel later writes the page out via the `writepage()` method. Write operations on specific files are more complicated. The generic write path in `mm/filemap.c` performs the following steps:

```c
page = __grab_cache_page(mapping, index, &cached_page, &lru_pvec);
status = a_ops->prepare_write(file, page, offset, offset+bytes);
page_fault = filemap_copy_from_user(page, offset, buf, bytes);
status = a_ops->commit_write(file, page, offset, offset+bytes);
```

First, the page cache is searched for the desired page. If it is not in the cache, an entry is allocated and added. Next, the kernel sets up the write request and the data is copied from user-space into a kernel buffer. Finally, the data is written to disk.

### Radix Tree

Each `address_space` has a unique radix tree stored as `page_tree`. A radix tree is a type of binary tree. The radix tree enable quick searching for the desired page, given only the file offset. Page cache searching functions such as `find_get_page()` call `radix_tree_lookup()`, which performs a search on the given tree for the given object.

The core radix tree code is available in generic form in `lib/radix-tree.c`. Users of the radix tree need to include `<linux/radix-tree.h>`.

## The Buffer Cache

Individual disk blocks also tie into the page cache, by way of block I/O buffers. Buffers act as descriptors that map pages in memory to disk blocks; thus, the page cache also reduces disk access during block I/O operations by both caching disk blocks and buffering block I/O operations until later. This caching is often referred to as the buffer cache, although as implemented it is not a separate cache but is part of the page cache.

## The Flusher Threads

When data in the page cache is newer than the data on the backing store, we call that data `dirty`. Dirty pages that accumulate in memory eventually need to be written back to disk. Dirty page writeback occurs in three situations:

- When free memory shrinks below a specified threshold, the kernel writes dirty data back to disk. When clean, the kernel can evict the data from the cache and then shrink the cache, freeing up more memory.
- When dirty data grows older than a specific threshold, sufficiently old data is written back to disk to ensure that dirty data does not remain dirty indefinitely.
- When a user process invokes the `sync()` and `fsync()` system calls, the kernel performs writeback on demand.

A gang of kernel threads, the `flusher threads`, performs all three jobs.

The system administrator can set these values either in `/proc/sys/vm` or via sysctl:

Variable | Description
-------- | -----------
dirty_background_ratio | As a percentage of total memory, the number of pages at which the flusher threads begin writeback of dirty data.
dirty_expire_interval | In milliseconds, how old data must be to be written out the next time a flusher thread wakes to perform periodic writeback.
dirty_ratio | As a percentage of total memory, the number of pages a process generates before it begins writeback of dirty data.
dirty_writeback_interval | In milliseconds, how often a flusher thread should wake up to write data back out to disk.
laptop_mode | A Boolean value controlling laptop mode. See the following section.

The flusher code lives in `mm/page-writeback.c` and `mm/backing-dev.c` and the writeback mechanism lives in `fs/fs-writeback.c`.
