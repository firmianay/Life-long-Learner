# Chapter 16: The Page Cache and Page Writeback
The Linux kernel implements a disk cache called the `page cache`. The goal of this cache is to minimize disk I/O by storing data in physical memory that would otherwise require disk access.

Two factors comingle to make disk caches a critical component of any modern operating system:
- disk access is several orders of magnitude slower than memory access.
- data accessed once will, with a high likelihood, find itself accessed again in the near future. This principle is called `temporal locality`.


## Approaches to Caching
