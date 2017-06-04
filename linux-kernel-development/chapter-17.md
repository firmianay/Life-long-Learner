# Chapter 17: Devices and Modules

- `Device types`: Classifications used in all Unix systems to unify behavior of common devices
- `Modules`: The mechanism by which the Linux kernel can load and unload object code on demand
- `Kernel objects`: Support for adding simple object-oriented behavior and a parent/child relationship to kernel data structures
- `Sysfs`: A filesystem representation of the system's device tree


## Device Types
In Linux, as with all Unix systems, devices are classified into one of three types:
- `Block devices`: Often abbreviated `blkdevs`, `block devices` are addressable in device-specified chunks called `blocks` and generally support `seeking`, the random access of data.
- `Character devices`: Often abbreviated `cdevs`, character devices are generally not addressable, providing access to data only as a stream, generally of characters (bytes).
- `Network devices`: Sometimes called `Ethernet devices` after the most common type of network devices, `network devices` provide access to a network via a physical adapter and a specific protocol.


## Modules
The Linux kernel is modular, supporting the dynamic insertion and removal of code from itself at runtime. Related subroutines, data, and entry and exit points are grouped together in a single binary image, a loadable kernel object, called a `module`.


## The Device Model
A significant new feature in the 2.6 Linux kernel is the addition of a unified `device model`. The device model provides a single mechanism for representing devices and describing their topology in the system.



## sysfs
The `sysfs filesystem` is an in-memory virtual filesystem that provides a view of the kobject hierarchy. It enables users to view the device topology of their system as a simple filesystem. Using attributes, kobjects can export files that enable kernel variables to be read from and optionally written to.

The magic behind `sysfs` is simply tying kobjects to directory entries via the dentry
member inside each kobject.
