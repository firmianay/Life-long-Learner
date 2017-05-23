# Chapter 13: The Virtual Filesystem
The `Virtual Filesystem` (VFS) is the subsystem of the kernel that implements the file and filesystem-related interfaces to user-space. All filesystems rely on the VFS not only to coexist but also to interoperate. This enables programs to use standard Unix system calls to read and write to different filesystems, even on different media.

## Common Filesystem interfaces
The VFS enables system calls work between different filesystems and media. New filesystems and new Virtual of storage media can find their way into Linux, and programs need not be rewritten or even recompiled.


## Filesystem Abstraction
Ther kernel implements an abstraction layer around its low-level filesystem interface. This abstraction layer works by defining the basic conceptual interfaces and data structures that all filesystems support.


## Unix Filesystem
Unix has provided four basic filesystem-related abstractions:
- `file`:
- `directory entries`:
- `inodes`:
- `mount points`:
