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
```
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
```
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
```
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
```
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
```
i->i_op->truncate(i)
```
`i` is a reference to a particular inode, the `truncate()` operation defined by the filesystem on which `i` exists is called on the given inode.

The `inode_operations` structure is defined in `<linux/fs.h>`:
```
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
