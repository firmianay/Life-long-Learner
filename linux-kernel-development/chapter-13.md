# Chapter 13: The Virtual Filesystem
The `Virtual Filesystem` (VFS) is the subsystem of the kernel that implements the file and filesystem-related interfaces to user-space. All filesystems rely on the VFS not only to coexist but also to interoperate. This enables programs to use standard Unix system calls to read and write to different filesystems, even on different media.

## Common Filesystem interfaces
The VFS enables system calls work between different filesystems and media. New filesystems and new Virtual of storage media can find their way into Linux, and programs need not be rewritten or even recompiled.


## Filesystem Abstraction
Ther kernel implements an abstraction layer around its low-level filesystem interface. This abstraction layer works by defining the basic conceptual interfaces and data structures that all filesystems support.


## Unix Filesystem
Unix has provided four basic filesystem-related abstractions:
- `file`: A `file` is an ordered string of bytes. The first byte marks the beginning of the file, and the last byte marks the end of the file. Each file is assigned a human-readable name for identification by both the system and the user.
- `directory entries`: Files are organized in directories. A `directory` is analogous to a folder and usually contrains related files. Directories can also contain other directories, called subdirectories. In this fashion, directories may be nested to form paths. Each component of a path is called a `directory entry`, all of them called `dentries`. A directory is a file to the VFS.
- `inodes`: Information about a file called `file metadata` and is stored in a separate data structure form the file, called the `inode`.
- `mount points`: filesystems are mounted at a specific mount point in a global hierarchy known as a `namespace`. This enables all mounted filesystems to appear as entries in a single tree.


## VFS Objects and Their Data Structures
The VFS is object-oriented. A family of data structures represents the common file model. These data structures are skin to objects.

The four primary object types of the VFS are:
- The `superblock` object, which represents a specific mounted filesystem.
- The `inode` object, which represents a specific file.
- The `dentry` object, which represents a directory entry, which is a single component of a path.
- The `file` object, which represents an open file are associated with a process.

An `operations` object is contained within each of these primary objects. These objects describe the methods that the kernel invokes agains the primary objects:
- The `super_operations` object, which contains the methods that the kernel can invoke on a specific filesystem, such as `write_inode()` and `sync_fs()`.
- The `inode_operations` object, which contains the methods that the kernel can invoke on a specific file, such as `create()` and `link()`.
- The `dentry_operations` object, which contains the methods that the kernel can invoke on a specific directory entry, such as `d_compare()` and `d_delete()`.
- The `file_operations` object, which contains the methods that a process can invoke on an open file, such as `read()` and `write()`.

The operations objects are implemented as a structure of pointers to functions that operate on the parent object.


## The Superblock Object
The superblock object is implemented by each filesystem and is used to store information describing that specific filesystem. This object usually corresponds to the filesystem superblock or the filesystem control block, which is stored in a special sector on disk. Filesystems that are not disk-based generate the superblock on-the-fly and store it in memory.

The superblock object is represented by `struct super_block` and defined in `<linux/fs.h>`:
```c
struct super_block {
        struct list_head          s_list;             /* list of all superblocks */
        dev_t                     s_dev;              /* identifier */
        unsigned long             s_blocksize;        /* block size in bytes */
        unsigned char             s_blocksize_bits;   /* block size in bits */
        unsigned char             s_dirt;             /* dirty flag */
        unsigned long long        s_maxbytes;         /* max file size */
        struct file_system_type   s_type;             /* filesystem type */
        struct super_operations   s_op;               /* superblock methods */
        struct dquot_operations   *dq_op;             /* quota methods */
        struct quotactl_ops       *s_qcop;            /* quota control methods */
        struct export_operations  *s_export_op;       /* export methods */
        unsigned long             s_flags;            /* mount flags */
        unsigned long             s_magic;            /* filesystem’s magic number */
        struct dentry             *s_root;            /* directory mount point */
        struct rw_semaphore       s_umount;           /* unmount semaphore */
        struct semaphore          s_lock;             /* superblock semaphore */
        int                       s_count;            /* superblock ref count */
        int                       s_need_sync;        /* not-yet-synced flag */
        atomic_t                  s_active;           /* active reference count */
        void                      *s_security;        /* security module */
        struct xattr_handler      **s_xattr;          /* extended attribute handlers */
        struct list_head          s_inodes;           /* list of inodes */
        struct list_head          s_dirty;            /* list of dirty inodes */
        struct list_head          s_io;               /* list of writebacks */
        struct list_head          s_more_io;          /* list of more writeback */
        struct hlist_head         s_anon;             /* anonymous dentries */
        struct list_head          s_files;            /* list of assigned files */
        struct list_head          s_dentry_lru;       /* list of unused dentries */
        int                       s_nr_dentry_unused; /* number of dentries on list */
        struct block_device       *s_bdev;            /* associated block device */
        struct mtd_info           *s_mtd;             /* memory disk information */
        struct list_head          s_instances;        /* instances of this fs */
        struct quota_info         s_dquot;            /* quota-specific options */
        int                       s_frozen;           /* frozen status */
        wait_queue_head_t         s_wait_unfrozen;    /* wait queue on freeze */
        char                      s_id[32];           /* text name */
        void                      *s_fs_info;         /* filesystem-specific info */
        fmode_t                   s_mode;             /* mount permissions */
        struct semaphore          s_vfs_rename_sem;   /* rename semaphore */
        u32                       s_time_gran;        /* granularity of timestamps */
        char                      *s_subtype;         /* subtype name */
        char                      *s_options;         /* saved mount options */
};
```

The code for creating, managing, and destroying superblock objects lives in `fs/super.c`. A superblock object is created and initialized via the `alloc_super()` function. When mounted, a filesystem invokes this function, reads its superblock off of the disk, and fills in its superblock object.


## Superblock Operations
The most important item in the superblock object is `s_op`, which is a pointer to the superblock operations table. The superblock operations table is represented by `struct super_operations` and is defined in `<linux/fs.h>`, which looks like this:
```c
struct super_operations {
        struct inode *(*alloc_inode)(struct super_block *sb);
        void (*destroy_inode)(struct inode *);
        void (*dirty_inode) (struct inode *);
        int (*write_inode) (struct inode *, int);
        void (*drop_inode) (struct inode *);
        void (*delete_inode) (struct inode *);
        void (*put_super) (struct super_block *);
        void (*write_super) (struct super_block *);
        int (*sync_fs)(struct super_block *sb, int wait);
        int (*freeze_fs) (struct super_block *);
        int (*unfreeze_fs) (struct super_block *);
        int (*statfs) (struct dentry *, struct kstatfs *);
        int (*remount_fs) (struct super_block *, int *, char *);
        void (*clear_inode) (struct inode *);
        void (*umount_begin) (struct super_block *);
        int (*show_options)(struct seq_file *, struct vfsmount *);
        int (*show_stats)(struct seq_file *, struct vfsmount *);
        ssize_t (*quota_read)(struct super_block *, int, char *, size_t, loff_t);
        ssize_t (*quota_write)(struct super_block *, int, const char *, size_t, loff_t);
        int (*bdev_try_to_free_page)(struct super_block*, struct page*, gfp_t);
};
```
Each item in this structure is a pointer to a function that operates on a superblock object. The superblock operations perform low-level operations on the filesystem and its inodes.

When a filesystem needs to perform an operation on its superblock, it follows the pointers from its superblock object to the desired method. For example:
```c
sb->s_op->write_super(sb);
```

The following are some of the superblock operations that are specified by `super_operations`:

- `struct inode * alloc_inode(struct super_block *sb)`
    - Creates and initializes a new inode object under the given superblock.
- `void destroy_inode(struct inode *inode)`
    - Deallocates the given inode.
- `void dirty_inode(struct inode *inode)`
    - Invoked by the VFS when an inode is dirtied (modified). Journaling filesystems such as ext3 and ext4 use this function to perform journal updates.
- `void write_inode(struct inode *inode, int wait)`
    - Writes the given inode to disk. The `wait` parameter specifies whether the operation should be synchronous.
- `void drop_inode(struct inode *inode)`
    - Called by the VFS when the last reference to an inode is dropped. Normal Unix filesystems do not define this function, in which case the VFS simply deletes the inode.
- `void delete_inode(struct inode *inode)`
    - Deletes the given inode from the disk.
- `void put_super(struct super_block *sb)`
    - Called by the VFS on unmount to release the given superblock object.The caller must hold the `s_lock` lock.
- `void write_super(struct super_block *sb)`
    - Updates the on-disk superblock with the specified superblock.The VFS uses this function to synchronize a modified in-memory superblock with the disk. The caller must hold the `s_lock` lock.
- `int sync_fs(struct super_block *sb, int wait)`
    - Synchronizes filesystem metadata with the on-disk filesystem. The `wait` parameter specifies whether the operation is synchronous.
- `void write_super_lockfs(struct super_block *sb)`
    - Prevents changes to the filesystem, and then updates the on-disk superblock with the specified superblock. It is currently used by LVM (the Logical Volume Manager).
- `void unlockfs(struct super_block *sb)`
    - Unlocks the filesystem against changes as done by `write_super_lockfs()`.
- `int statfs(struct super_block *sb, struct statfs *statfs)`
    - Called by the VFS to obtain filesystem statistics. The statistics related to the given filesystem are placed in `statfs`.
- `int remount_fs(struct super_block *sb, int *flags, char *data)`
    - Called by the VFS when the filesystem is remounted with new mount options.The caller must hold the `s_lock` lock.
- `void clear_inode(struct inode *inode)`
    - Called by the VFS to release the inode and clear any pages containing related data.
- `void umount_begin(struct super_block *sb)`
    - Called by the VFS to interrupt a mount operation. It is used by network filesystems, such as NFS.


## The Inode Object
The inode Object represents all the information needed by the kernel to manipulate a file or directory. For Unix-style filesystems, this information is simply read from the on-disk inode.

The inode object is represented by `struct inode` and is defined in `<linux/fs.h>`.
```c
struct inode {
        struct hlist_node       i_hash;               /* hash list */
        struct list_head        i_list;               /* list of inodes */
        struct list_head        i_sb_list;            /* list of superblocks */
        struct list_head        i_dentry;             /* list of dentries */
        unsigned long           i_ino;                /* inode number */
        atomic_t                i_count;              /* reference counter */
        unsigned int            i_nlink;              /* number of hard links */
        uid_t                   i_uid;                /* user id of owner */
        gid_t                   i_gid;                /* group id of owner */
        kdev_t                  i_rdev;               /* real device node */
        u64                     i_version             /* versioning number */
        loff_t                  i_size                /* file size in bytes */
        seqcount_t              i_size_seqcount;      /* serializer for i_size */
        struct timespec         i_atime;              /* last access time */
        struct timespec         i_mtime;              /* last modify time */
        struct timespec         i_ctime;              /* last change time */
        unsigned int            i_blkbits;            /* block size in bits */
        blkcnt_t                i_blocks;             /* block size in bits */
        unsigned short          i_bytes;              /* bytes consumed */
        umode_t                 i_mode;               /* access permissions */
        spinlock_t              i_lock;               /* spinlock */
        struct rw_semaphore     i_alloc_sem;          /* nests inside of i_sem */
        struct semaphore        i_sem;                /* inode semaphore */
        struct inode_operations *i_op;                /* inode ops table */
        struct file_operations  *i_fop;               /* default inode ops */
        struct super_block      *i_sb;                /* associated superblock */
        struct file_lock        *i_flock;             /* file lock list */
        struct address_space    *i_mapping;           /* associated mapping */
        struct address_space    i_data;               /* mapping for device */
        struct dquot            *i_dquot[MAXQUOTAS];  /* disk quotas for inode */
        struct list_head        i_devices;            /* list of block devices */
        union {
            struct pipe_inode_info  *i_pipe;          /* pipe information */
            struct block_device     *i_bdev;          /* block device driver */
            struct cdev             *i_cdev;          /* character device driver */
        };
        unsigned long           i_dnotify_mask;       /* directory notify mask */
        struct dnotify_struct   *i_dnotify;           /* dnotify */
        struct list_head        inotify_watches;      /* inotify watches */
        struct mutex            inotify_mutex;        /* protects inotify_watches */
        unsigned long           i_state;              /* state flags */
        unsigned long           dirtied_when;         /* first dirtying time */
        unsigned int            i_flags;              /* filesystem flags */
        atomic_t                i_writecount;         /* count of writers */
        void                    *i_security;          /* security module */
        void                    *i_private;           /* fs private pointer */
};
```
An inode represents each file on a filesystem, but the inode object is constructed in memory only as file are accessed.


## Inode Operations
Inode Operations are invoked via:
```c
i->i_op->truncate(i)
```
`i` is a reference to a particular inode, the `truncate()` operation defined by the filesystem on which `i` exists is called on the given inode.

The `inode_operations` structure is defined in `<linux/fs.h>`:
```c
struct inode_operations {
    int (*create) (struct inode *, struct dentry *, int, struct nameidata *);
    struct dentry * (*lookup) (struct inode *, struct dentry *, struct nameidata *);
    int (*link) (struct dentry *, struct inode *, struct dentry *);
    int (*unlink) (struct inode *, struct dentry *);
    int (*symlink) (struct inode *, struct dentry *, const char *);
    int (*mkdir) (struct inode *, struct dentry *, int);
    int (*rmdir) (struct inode *, struct dentry *);
    int (*mknod) (struct inode *, struct dentry *, int, dev_t);
    int (*rename) (struct inode *, struct dentry *, struct inode *, struct dentry *);
    int (*readlink) (struct dentry *, char __user *, int);
    void * (*follow_link) (struct dentry *, struct nameidata *);
    void (*put_link) (struct dentry *, struct nameidata *, void *);
    void (*truncate) (struct inode *);
    int (*permission) (struct inode *, int);
    int (*setattr) (struct dentry *, struct iattr *);
    int (*getattr) (struct vfsmount *mnt, struct dentry *, struct kstat *);
    int (*setxattr) (struct dentry *, const char *, const void *, size_t, int);
    ssize_t (*getxattr) (struct dentry *, const char *, void *, size_t);
    ssize_t (*listxattr) (struct dentry *, const char *);
    int (*removexattr) (struct dentry *, const char *);
    void (*truncate_range) (struct inode *, loff_t, loff_t);
    long (*fallocate) (struct inode *inode, int mode, loff_t offset, loff_t len);
    int (*fiemap) (struct inode *, struct fiemap_extent_info *, u64 start, u64 len);
};
```

The following interfaces constitute the various functions that the VFS may perform, or ask a specific filesystem to perform, on a given inode:
- `int create(struct inode *dir, struct dentry *dentry, int mode)`
    - TheVFS calls this function from the `creat()` and `open()` system calls to create a new inode associated with the given dentry object with the specified initial access mode.
- `struct dentry * lookup(struct inode *dir, struct dentry *dentry)`
    - This function searches a directory for an inode corresponding to a filename specified in the given dentry.
- `int link(struct dentry *old_dentry, struct inode *dir, struct dentry *dentry)`
    - Invoked by the link() system call to create a hard link of the file `old_dentry` in the directory dir with the new filename `dentry`.
- `int unlink(struct inode *dir, struct dentry *dentry)`
    - Called from the `unlink()` system call to remove the inode specified by the directory entry dentry from the directory `dir`.
- `int symlink(struct inode *dir, struct dentry *dentry, const char *symname)`
    - Called from the `symlink()` system call to create a symbolic link named symname to the file represented by dentry in the directory `dir`.
- `int mkdir(struct inode *dir, struct dentry *dentry, int mode)`
    - Called from the `mkdir()` system call to create a new directory with the given initial mode.
- `int rmdir(struct inode *dir, struct dentry *dentry)`
    - Called by the `rmdir()` system call to remove the directory referenced by dentry from the directory `dir`.
- `int mknod(struct inode *dir, struct dentry *dentry, int mode, dev_t rdev)`
    - Called by the `mknod()` system call to create a special file (device file, named pipe, or socket). The file is referenced by the device rdev and the directory entry dentry in the directory `dir`. The initial permissions are given via `mode`.
- `int rename(struct inode *old_dir, struct dentry *old_dentry, struct inode *new_dir, struct dentry *new_dentry)`
    - Called by the VFS to move the file specified by `old_dentry` from the `old_dir` directory to the directory `new_dir`, with the filename specified by `new_dentry`.
- `int readlink(struct dentry *dentry, char *buffer, int buflen)`
    - Called by the `readlink()` system call to copy at most `buflen` bytes of the full path associated with the symbolic link specified by `dentry` into the specified buffer.
- `int follow_link(struct dentry *dentry, struct nameidata *nd)`
    - Called by the VFS to translate a symbolic link to the inode to which it points. The link pointed at by `dentry` is translated, and the result is stored in the `nameidata` structure pointed at by `nd`.
- `int put_link(struct dentry *dentry, struct nameidata *nd)`
    - Called by the VFS to clean up after a call to `follow_link()`.
- `void truncate(struct inode *inode)`
    - Called by the VFS to modify the size of the given file. Before invocation, the inode’s `i_size` field must be set to the desired new size.
- `int permission(struct inode *inode, int mask)`
    - Checks whether the specified access mode is allowed for the file referenced by `inode`. This function returns zero if the access is allowed and a negative error code otherwise. Most filesystems set this field to `NULL` and use the generic VFS method, which simply compares the mode bits in the inode’s objects to the given mask.
- `int setattr(struct dentry *dentry, struct iattr *attr)`
    - Called from `notify_change()` to notify a “change event” after an inode has been modified.
- `int getattr(struct vfsmount *mnt, struct dentry *dentry, struct kstat *stat)`
    - Invoked by the VFS upon noticing that an inode needs to be refreshed from disk. Extended attributes allow the association of key/values pairs with files.
- `int setxattr(struct dentry *dentry, const char *name, const void *value, size_t size, int flags)`
    - Used by the VFS to set the extended attribute name to the value value on the file referenced by dentry .
- `ssize_t getxattr(struct dentry *dentry, const char *name, void *value, size_t size)`
    - Used by the VFS to copy into value the value of the extended attribute `name` for the specified file.
- `ssize_t listxattr(struct dentry *dentry, char *list, size_t size)`
    - Copies the list of all attributes for the specified file into the buffer `list`.
- `int removexattr(struct dentry *dentry, const char *name)`
    - Removes the given attribute from the given file.


## The Dentry Object
Dentry objects are all components in a path, including files.

Dentry objects are represented by `struct dentry` and defined in `<linux/dcache.h>`.
```c
struct dentry {
    atomic_t                  d_count;        /* usage count */
    unsigned int              d_flags;        /* dentry flags */
    spinlock_t                d_lock;         /* per-dentry lock */
    int                       d_mounted;      /* is this a mount point? */
    struct inode              *d_inode;       /* associated inode */
    struct hlist_node         d_hash;         /* list of hash table entries */
    struct dentry             *d_parent;      /* dentry object of parent */
    struct qstr               d_name;         /* dentry name */
    struct list_head          d_lru;          /* unused list */
    union {
        struct list_head      d_child;        /* list of dentries within */
        struct rcu_head       d_rcu;          /* RCU locking */
      } d_u;
    struct list_head          d_subdirs;      /* subdirectories */
    struct list_head          d_alias;        /* list of alias inodes */
    unsigned long             d_time;         /* revalidate time */
    struct dentry_operations  *d_op;          /* dentry operations table */
    struct super_block        *d_sb;          /* superblock of file */
    void                      *d_fsdata;      /* filesystem-specific data */
    unsigned char             d_iname[DNAME_INLINE_LEN_MIN];  /* short name */
};
```
Unlike the previous two objects, the dentry object does not correspond to any sort of on-disk data structure. The VFS creates it on-the-fly from a string representation of a path name. Because the dentry object is not physically stored on the disk, no flag in `struct dentry` specifies whether the object is modified.

### Dentry State
A valid dentry object can be in one of three states:
- `used`: A used dentry corresponds to a valid inode and indicates that there are one or more users of the object.
- `unused`: An unused dentry corresponds to a valid inode, but the VFS is not currently using the dentry object.
- `negative`: A negative dentry is not associated with a valid inode because either the inode was deleted or the path name was resolved quickly.

### The Dentry Cache
The kernel caches dentry objects in the dentry cache or, simply, the `dcache`. The dentry cache consists of three parts:
- List of "used" dentries linked off their associated inode via the `i_dentry` field of the inode object.
- A doubly linked "least recently used" list of unused and negative dentry objects.
- A hash table and hashing function used to quickly resolve a given path into the associated dentry object.

The hash table is represented by the `dentry_hashtable` array. Each element is a pointer to a list of dentries that hash to the same value. The size of this array depends on the amount of physical RAM in the system. The actual hash value is determined by `d_hash)()`. Hash table lookup is performed via `d_lookup()`.


## Dentry Operations
The dentry_operations structure specifies the methods that the VFS invokes on directory entries on a given filesystem. The `dentry_operations` structure is defined in `<linux/dcache.h>`:
```c
struct dentry_operations {
        int (*d_revalidate) (struct dentry *, struct nameidata *);
        int (*d_hash) (struct dentry *, struct qstr *);
        int (*d_compare) (struct dentry *, struct qstr *, struct qstr *);
        int (*d_delete) (struct dentry *);
        void (*d_release) (struct dentry *);
        void (*d_iput) (struct dentry *, struct inode *);
        char *(*d_dname) (struct dentry *, char *, int);
};
```

The methods are as follows:
- `int d_revalidate(struct dentry *dentry, struct nameidata *)`
    - Determines whether the given dentry object is valid. The VFS calls this function whenever it is preparing to use a dentry from the dcache.
- `int d_hash(struct dentry *dentry, struct qstr *name)`
    - Creates a hash value from the given dentry. The VFS calls this function whenever it adds a dentry to the hash table.
- `int d_compare(struct dentry *dentry, struct qstr *name1, struct qstr *name2)`
    - Called by the VFS to compare two filenames, `name1` and `name2`.
- `int d_delete(struct dentry *dentry)`
    - Called by the VFS when the specified dentry object's `d_count` reaches zero. This function requires the `dcache_lock` and the dentry's `d_lock`.
- `void d_release(struct dentry *dentry)`
    - Called by the VFS when the specified dentry is going to be freed.
- `void d_iput(struct dentry *dentry, struct inode *inode)`
    - Called by the VFS when a dentry object loses its associated inode.


## The File Object
The file object is used to represent a file opened by a process. The file object is the in-memory representation of an open file. The object is created in response to the `open()` system call and destroyed in response to the `close()` system call. Because multiple processes can open and manipulate a file at the same time, there can be multiple file objects in existence for the same file. The file object merely represents a process's view of an open file. The object points back to the dentry that actually represents the open file. The inode and dentry objects are unique.

The file object is represented by `struct file` and is defined in `<linux/sf.h>`:
```c
struct file {
        union {
            struct list_head        fu_list;            /* list of file objects */
            struct rcu_head         fu_rcuhead;         /* RCU list after freeing */
        } f_u;
        struct path                 f_path;             /* contains the dentry */
        struct file_operations      *f_op;              /* file operations table */
        spinlock_t                  f_lock;             /* per-file struct lock */
        atomic_t                    f_count;            /* file object's usage count */
        unsigned int                f_flags;            /* flags specified on open */
        mode_t                      f_mode;             /* file access mode */
        loff_t                      f_pos;              /* file offset (file pointer) */
        struct fown_struct          f_owner;            /* owner data for signals */
        const struct cred           *f_cred;            /* file credentials */
        struct file_ra_state        f_ra;               /* read-ahead state */
        u64                         f_version;          /* version number */
        void                        *f_security;        /* security module */
        void                        *private_data;      /* tty driver hook */
        struct list_head            f_ep_links;         /* list of epoll links */
        spinlock_t                  f_ep_lock;          /* epoll lock */
        struct address_space        *f_mapping;         /* page cache mapping */
        unsigned long               f_mnt_write_state;  /* debugging state */
};
```
The file object does not actually correspond to any on-disk data. Therefore, no flag in the object represents whether the object is dirty and needs to be written back to disk. The file object does point to its associated dentry object via the `f_dentry` pointer. The dentry in turn points to the associated inode, which reflects whether the file itself is dirty.


## File Operations
The file object methods are specified in `file_operations` and defined in `<linux/fs.h>`:
```c
struct file_operations {
        struct module *owner;
        loff_t (*llseek) (struct file *, loff_t, int);
        ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
        ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
        ssize_t (*aio_read) (struct kiocb *, const struct iovec *,
                             unsigned long, loff_t);
        ssize_t (*aio_write) (struct kiocb *, const struct iovec *,
                              unsigned long, loff_t);
        int (*readdir) (struct file *, void *, filldir_t);
        unsigned int (*poll) (struct file *, struct poll_table_struct *);
        int (*ioctl) (struct inode *, struct file *, unsigned int,
                      unsigned long);
        long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
        long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
        int (*mmap) (struct file *, struct vm_area_struct *);
        int (*open) (struct inode *, struct file *);
        int (*flush) (struct file *, fl_owner_t id);
        int (*release) (struct inode *, struct file *);
        int (*fsync) (struct file *, struct dentry *, int datasync);
        int (*aio_fsync) (struct kiocb *, int datasync);
        int (*fasync) (int, struct file *, int);
        int (*lock) (struct file *, int, struct file_lock *);
        ssize_t (*sendpage) (struct file *, struct page *,
                             int, size_t, loff_t *, int);
        unsigned long (*get_unmapped_area) (struct file *,
                                            unsigned long,
                                            unsigned long,
                                            unsigned long,
                                            unsigned long);
        int (*check_flags) (int);
        int (*flock) (struct file *, int, struct file_lock *);
        ssize_t (*splice_write) (struct pipe_inode_info *,
                                 struct file *,
                                 loff_t *,
                                 size_t,
                                 unsigned int);
        ssize_t (*splice_read) (struct file *,
                                loff_t *,
                                struct pipe_inode_info *,
                                size_t,
                                unsigned int);
        int (*setlease) (struct file *, long, struct file_lock **);
};
```

- `loff_t llseek(struct file *file, loff_t offset, int origin)`
    - Updates the file pointer to the given offset. It is called via the `llseek()` system call.
- `ssize_t read(struct file *file, char *buf, size_t count, loff_t *offset)`
    - Reads `count` bytes from the given file at position `offset` into `buf`. The file pointer is then updated. This function is called by the `read()` system call.
- `ssize_t aio_read(struct kiocb *iocb, char *buf, size_t count, loff_t offset)`
    - Begins an asynchronous read of `count` bytes into `buf` of the file described in `iocb`. This function is called by the `aio_read()` system call.
- `ssize_t write(struct file *file, const char *buf, size_t count, loff_t *offset)`
    - Writes `count` bytes from buf into the given file at position `offset`. The file pointer is then updated. This function is called by the `write()` system call.
- `ssize_t aio_write(struct kiocb *iocb, const char *buf, size_t count, loff_t offset)`
    - Begins an asynchronous write of `count` bytes into `buf` of the file described in `iocb`. This function is called by the `aio_write()` system call.
- `int readdir(struct file *file, void *dirent, filldir_t filldir)`
    - Returns the next directory in a directory listing. This function is called by the `readdir()` system call.
- `unsigned int poll(struct file *file, struct poll_table_struct *poll_table)`
    - Sleeps, waiting for activity on the given file. It is called by the `poll()` system call.
- `int ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)`
    - Sends a command and argument pair to a device. It is used when the file is an open device node. This function is called from the `ioctl()` system call. Callers must hold the BKL.
- `int unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)`
    - Implements the same functionality as `ioctl()` but without needing to hold the BKL. The VFS calls `unlocked_ioctl()` if it exists in lieu of `ioctl()` when user-space invokes the `ioctl()` system call. Thus filesystems need implement only one, preferably `unlocked_ioctl()`.
- `int compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)`
    - Implements a portable variant of `ioctl()` for use on 64-bit systems by 32-bit applications. This function is designed to be 32-bit safe even on 64-bit architectures, performing any necessary size conversions. New drivers should design their ioctl commands such that all are portable, and thus enable `compat_ioctl()` and `unlocked_ioctl()` to point to the same function. Like `unlocked_ioctl()`, `compat_ioctl()` does not hold the BKL.
- `int mmap(struct file *file, struct vm_area_struct *vma)`
    - Memory maps the given file onto the given address space and is called by the `mmap()` system call.
- `int open(struct inode *inode, struct file *file)`
    - Creates a new file object and links it to the corresponding inode object. It is called by the `open()` system call.
- `int flush(struct file *file)`
    - Called by the VFS whenever the reference count of an open file decreases. Its purpose is filesystem-dependent.
- `int release(struct inode *inode, struct file *file)`
    - Called by the VFS when the last remaining reference to the file is destroyed.
- `int fsync(struct file *file, struct dentry *dentry, int datasync)`
    - Called by the `fsync()` system call to write all cached data for the file to disk.
- `int aio_fsync(struct kiocb *iocb, int datasync)`
    - Called by the aio_fsync() system call to write all cached data for the file associated with `iocb` to disk.
- `int fasync(int fd, struct file *file, int on)`
    - Enables or disables signal notification of asynchronous I/O.
- `int lock(struct file *file, int cmd, struct file_lock *lock)`
    - Manipulates a file lock on the given file.
- `ssize_t readv(struct file *file, const struct iovec *vector, unsigned long count, loff_t *offset)`
    - Called by the `readv()` system call to read from the given file and put the results into the count buffers described by `vector`. The file offset is then incremented.
- `ssize_t writev(struct file *file, const struct iovec *vector, unsigned long count, loff_t *offset)`
    - Called by the `writev()` system call to write from the count buffers described by `vector` into the file specified by `file`. The file offset is then incremented.
- `ssize_t sendfile(struct file *file, loff_t *offset, size_t size, read_actor_t actor, void *target)`
    - Called by the `sendfile()` system call to copy data from one file to another. It performs the copy entirely in the kernel and avoids an extraneous copy to user-space.
- `ssize_t sendpage(struct file *file, struct page *page, int offset, size_t size, loff_t *pos, int more)`
    - Used to send data from one file to another.
- `unsigned long get_unmapped_area(struct file *file, unsigned long addr, unsigned long len, unsigned long offset, unsigned long flags)`
    - Gets unused address space to map the given file.
- `int check_flags(int flags)`
    - Used to check the validity of the flags passed to the `fcntl()` system call when the SETFL command is given.
- `int flock(struct file *filp, int cmd, struct file_lock *fl)`
    - Used to implement the `flock()` system call, which provides advisory locking.


## Data Structures Associated with Filesystems
The kernel uses other standard data structures to manage data related to filesystems. The first object is used to describe a specific variant of a filesystem. The second data structure describes a mounted instance of a filesystem.

The `file_system_type` structure, defined in `<linux/fs.h>`:
```c
struct file_system_type {
        const char              *name;        /* filesystem's name */
        int                     fs_flags;     /* filesystem type flags */

        /* the following is used to read the superblock off the disk */
        struct super_block      *(*get_sb) (struct file_system_type *, int,
                                          char *, void *);

        /* the following is used to terminate access to the superblock */
        void                    (*kill_sb) (struct super_block *);

        struct module           *owner;       /* module owning the filesystem */
        struct file_system_type *next;      /* next file_system_type in list */
        struct list_head        fs_supers;    /* list of superblock objects */

        /* the remaining fields are used for runtime lock validation */
        struct lock_class_key   s_lock_key;
        struct lock_class_key   s_umount_key;
        struct lock_class_key   i_lock_key;
        struct lock_class_key   i_mutex__key;
        struct lock_class_key   i_mutex__dir_key;
        struct lock_class_key   i_alloc_sem_key;
};
```
The `get_sb()` function reads the superblock from the disk and populates the superblock object when the filesystem is loaded. The remaining functions describe the filesystem's properties. There is only one `file_system_type` per filesystem, regardless of how many instances of the filesystem are mounted on the system, or whether the filesystem is even mounted at all.

When the filesystem is actually mounted, at which point the `vfsmount` structure is created. This structure represents a specific instance of a filesystem. The `vfsmount` structure is defined in `<linux/mount.h>`:
```c
struct vfsmount {
        struct list_head      mnt_hash;         /* hash table list */
        struct vfsmount       *mnt_parent;      /* parent filesystem */
        struct dentry         *mnt_mountpoint;  /* dentry of this mount point */
        struct dentry         *mnt_root;        /* dentry of root of this fs */
        struct super_block    *mnt_sb;          /* superblock of this filesystem */
        struct list_head      mnt_mounts;       /* list of children */
        struct list_head      mnt_child;        /* list of children */
        int                   mnt_flags;        /* mount flags */
        char                  *mnt_devname;     /* device file name */
        struct list_head      mnt_list;         /* list of descriptors */
        struct list_head      mnt_expire;       /* entry in expiry list */
        struct list_head      mnt_share;        /* entry in shared mounts list */
        struct list_head      mnt_slave_list;   /* list of slave mounts */
        struct list_head      mnt_slave;        /* entry in slave list */
        struct vfsmount       *mnt_master;      /* slave’s master */
        struct mnt_namespace  *mnt_namespace;   /* associated namespace */
        int                   mnt_id;           /* mount identifier */
        int                   mnt_group_id;     /* peer group identifier */
        atomic_t              mnt_count;        /* usage count */
        int                   mnt_expiry_mark;  /* is marked for expiration */
        int                   mnt_pinned;       /* pinned count */
        int                   mnt_ghosts;       /* ghosts count */
        atomic_t              __mnt_writers;    /* writers count */
};
```
The complicated part of maintaining the list of all mount points is the relation between the filesystem and all the other mount points. The various linked lists in `vfsmount` keep track of this information. The `vfsmount` structure also stores the flags, if any, specified on mount in the `mnt_flags` field.

Flag | Description
---- | -----------
MNT_NOSUID | Forbids setuid and setgid flags on binaries on this filesystem
MNT_NODEV | Forbids access to device files on this filesystem
MNT_NOEXEC | Forbids execution of binaries on this filesystem


## Data Structures Associated with a Process
Each process on the system has its own list of open files, root filesystem, current working directory, mount points, and so on. Three data structures tie together the VFS layer and the processes on the system: `files_struct`, `fs_struct`, and `namespace`.

The `files_struct` is defined in `<linux/fdtable.h>`. This table’s address is pointed to by the files entry in the processor descriptor. All per-process information about open files and file descriptors is contained therein:
```c
struct files_struct {
        atomic_t                count;              /* usage count */
        struct fdtable          *fdt;               /* pointer to other fd table */
        struct fdtable          fdtab;              /* base fd table */
        spinlock_t              file_lock;          /* per-file lock */
        int                     next_fd;            /* cache of next available fd */
        struct embedded_fd_set  close_on_exec_init; /* list of close-on-exec fds */
        struct embedded_fd_set  open_fds_init       /* list of open fds */
        struct file             *fd_array[NR_OPEN_DEFAULT]; /* base files array */
};
```
The array `fd_array` points to the list of open file objects. If a process opens more than 64 file objects, the kernel allocates a new array and points the `fdt` pointer at it.

The second process-ralated structure is `fs_struct`, which contains filesystem information related to a process and is pointed at by the `fs` field in the processs descriptor. The structure is defined in `<linux/fs_struct.h>`.
```c
struct fs_struct {
        int           users;    /* user count */
        rwlock_t      lock;     /* per-structure lock */
        int           umask;    /* umask */
        int           in_exec;  /* currently executing a file */
        struct path   root;     /* root directory */
        struct path   pwd;      /* current working directory */
};
```
This structure holds the current working directory and root directory of the current process.

The third and final structure is the `namespace` structure, which is defined in `<linux/mnt_namespace.h>` and pointed at by the `mnt_namespace` field in the process descriptor.
```c
struct mnt_namespace {
        atomic_t            list;   /* usage count */
        struct vfsmount     *root;  /* root directory */
        struct list_head    list;   /* list of mount points */
        wait_queue_head_t   poll;   /* polling waitqueue */
        int                 event;  /* event count */
};
```
The `list` member specifies a doubly linked list of the mounted filesystems that make up the namespace.
