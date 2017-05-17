# Chapter 10: Kernel Synchronization Methods

## Atomic Operations
atomic operations provide instructions that execute atomically, without interruption.

The kernel provides two sets of interfaces for atomic operations: one that operates on integers and another that operates on individual bits.

### Atomic Integer Operations
The atomic integer methods operate on a special data type, `atomic_t`. This special type is used, as opposed to having the functions work directly on the C `int` type, for several reasons:
1. Having the atomic functions accept only the `atomic_t` type ensures that the atomic operations are used only with these special types. Likewise, it also ensures that the data types are not passed to any non-atomic functions.
2. The use of `atomic_t` ensures the compiler does not optimize access to the value, it is important the atomic operations receive the correct memory address and not an alias.
3. Use of `atomic_t` can hide any architecture-specific differences in its implementation.

The `atomic_t` type is defined in `<linux/types.h>`:
```
typedef struct {
    volatile int counter;
} atomic_t;
```

The declarations needed to use the atomic integer operations are in `<asm/atomic.h>`.

Atomic Integer Operation | Description
------------------------ | -----------
ATOMIC_INIT(int i) | At declaration, initialize to `i`.
int atomic_read(atomic_t *v) | Atomically read the integer value of `v`.
void atomic_set(atomic_t *v, int i) | Atomically set `v` equal to `i`.
void atomic_add(int i, atomic_t *v) | Atomically add `i` to `v`.
void atomic_sub(int i, atomic_t *v) | Atomically subtract `i` from `v`.
void atomic_inc(atomic_t *v) | Atomically add one to `v`.
void atomic_dec(atomic_t *v) | Atomically subtract one from `v`.
int atomic_sub_and_test(int i, atomic_t *v) | Atomically subtract `i` from `v` and return true if the result is zero; otherwise false.
int atomic_add_negative(int i, atomic_t *v) | Atomically add `i` to `v` and return true if the result is negative; otherwise false.
int atomic_add_return(int i, atomic_t *v) | Atomically add `i` to `v` and return the result.
int atomic_sub_return(int i, atomic_t *v) | Atomically subtract `i` from `v` and return the result.
int atomic_inc_return(int i, atomic_t *v) | Atomically increment `v` by one and return the result.
int atomic_dec_return(int i, atomic_t *v) | Atomically decrement `v` by one and return the result.
int atomic_dec_and_test(atomic_t *v) | Atomically decrement `v` by one and return true if zero; false otherwise.
int atomic_inc_and_test(atomic_t *v) | Atomically increment `v` by one and return true if the result is zero; false otherwise.

The atomic operations are typically implemented as inline functions with inline assembly. In the case where a specific function is inherently atomic, the given function is usually just a macro.

### 64-Bit Atomic operations
`atomic_t` is 32-bit even on 64-bit architectures. Instead, the `atomic64_t` type provides a 64-bit atomic integer.

As with `atomic_t`, the `atomic64_t` type is just a simple wrapper around an integer type a long:
```
typedef struct {
    volatile long counter;
} atomic64_t;
```

Atomic Integer Operation | Description
------------------------ | -----------
ATOMIC64_INIT(long i) | At declaration, initialize to `i`.
long atomic64_read(atomic64_t *v) | Atomically read the integer value of `v`.
void atomic64_set(atomic64_t *v, int i) | Atomically set `v` equal to `i`.
void atomic64_add(int i, atomic64_t *v) | Atomically add `i` to `v`.
void atomic64_sub(int i, atomic64_t *v) | Atomically subtract `i` from v.
void atomic64_inc(atomic64_t *v) | Atomically add one to `v`.
void atomic64_dec(atomic64_t *v) | Atomically subtract one from `v`.
int atomic64_sub_and_test(int i, atomic64_t *v) | Atomically subtract `i` from `v` and return true if the result is zero; otherwise false.
int atomic64_add_negative(int i, atomic64_t *v) | Atomically add `i` to `v` and return true if the result is negative; otherwise false.
long atomic64_add_return(int i, atomic64_t *v) | Atomically add `i` to `v` and return the result.
long atomic64_sub_return(int i, atomic64_t *v) | Atomically subtract `i` from `v` and return the result.
long atomic64_inc_return(int i, atomic64_t *v) | Atomically increment `v` by one and return the result.
long atomic64_dec_return(int i, atomic64_t *v) | Atomically decrement `v` by one and return the result.
int atomic64_dec_and_test(atomic64_t *v) | Atomically decrement `v` by one and return true if zero; false otherwise.
int atomic64_inc_and_test(atomic64_t *v) | Atomically increment `v` by one and return true if the result is zero; false otherwise.

### Atomic Bitwise Operations
The functions that operate at the bit level are architecture-specific and defined in `<asm/bitops.h>`.

The bitwise functions operate on generic memory addresses. The arguments are a pointer and a bit number. Bit zero is the least significant bit of the given address.

A listing of the standard atomic bit operations:

Atomic Bitwise Operation | Description
------------------------ | -----------
void set_bit(int nr, void *addr) | Atomically set the `nr`-th bit starting from `addr`.
void clear_bit(int nr, void *addr) | Atomically clear the `nr`-th bit starting from `addr`.
void change_bit(int nr, void *addr) | Atomically flip the value of the `nr`-th bit starting from `addr`.
int test_and_set_bit(int nr, void *addr) | Atomically set the `nr`-th bit starting from `addr` and return the previous value.
int test_and_clear_bit(int nr, void *addr) | Atomically clear the `nr`-th bit starting from `addr` and return the previous value.
int test_and_change_bit(int nr, void *addr) | Atomically flip the `nr`-th bit starting from `addr` and return the previous value.
int test_bit(int nr, void *addr) | Atomically return the value of the `nr`-th bit starting from `addr`.

Conveniently, nonatomic versions of all the bitwise functions are also provided. They behave identically to their atomic siblings, except they do not guarantee atomicity, and their names are prefixed with double underscores.

The kernel also provides routines to find the first set (or unset) bit starting at a given address:
```
int find_first_bit(unsigned long *addr, unsigned int size)
int find_first_zero_bit(unsigned long *addr, unsigned int size)
```
Both functions take a pointer as their first argument and the number of bits in total to search as their second. They return the bit number of the first set or first unset bit.
