/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/include/linuxab/types.h
 * Fundamental kernel types for linuxab
 */

#ifndef _LINUXAB_TYPES_H
#define _LINUXAB_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Fixed-width unsigned types */
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

/* Fixed-width signed types */
typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

/* Fast minimum-width types */
typedef uint8_t     __u8;
typedef uint16_t    __u16;
typedef uint32_t    __u32;
typedef uint64_t    __u64;

typedef int8_t      __s8;
typedef int16_t     __s16;
typedef int32_t     __s32;
typedef int64_t     __s64;

/* LE/BE types for I/O */
typedef uint16_t    __le16;
typedef uint16_t    __be16;
typedef uint32_t    __le32;
typedef uint32_t    __be32;
typedef uint64_t    __le64;
typedef uint64_t    __be64;

/* Kernel size types */
typedef unsigned long       ulong;
typedef unsigned int        uint;
typedef unsigned short      ushort;
typedef unsigned char       uchar;

/* POSIX/Linux types */
typedef int32_t     pid_t;
typedef uint32_t    uid_t;
typedef uint32_t    gid_t;
typedef uint32_t    mode_t;
typedef uint64_t    dev_t;
typedef uint64_t    ino_t;
typedef uint32_t    nlink_t;
typedef int64_t     off_t;
typedef int64_t     loff_t;
typedef uint32_t    blksize_t;
typedef uint64_t    blkcnt_t;
typedef int64_t     time_t;
typedef int64_t     suseconds_t;
typedef uint32_t    useconds_t;
typedef int32_t     clockid_t;

/* File system types */
typedef uint64_t    sector_t;
typedef uint64_t    blkcnt64_t;
typedef int64_t     off64_t;

/* Network types */
typedef uint16_t    __sum16;
typedef uint32_t    __wsum;

/* Address types */
typedef uintptr_t   phys_addr_t;
typedef uintptr_t   dma_addr_t;
typedef void        *vm_offset_t;

/* Page-related */
typedef ulong       pgoff_t;

/* IDR/Radix tree */
typedef uint32_t    idr_t;

/* Reference count */
typedef int32_t     refcount_t;
typedef int32_t     atomic_t;
typedef int64_t     atomic64_t;

/* Resource size */
typedef uint64_t    resource_size_t;

/* IRQ */
typedef uint16_t    irq_hw_number_t;
typedef int         irqreturn_t;

/* Capability */
typedef struct {
    uint32_t cap[2];
} kernel_cap_t;

/* User/Kernel pointer annotations */
#ifndef __user
#define __user
#endif
#ifndef __kernel
#define __kernel
#endif
#ifndef __iomem
#define __iomem
#endif
#ifndef __force
#define __force
#endif
#ifndef __bitwise
#define __bitwise
#endif

/* Bitwise type safety for LE/BE */
typedef uint16_t    __bitwise __le16_bw;
typedef uint16_t    __bitwise __be16_bw;
typedef uint32_t    __bitwise __le32_bw;
typedef uint32_t    __bitwise __be32_bw;
typedef uint64_t    __bitwise __le64_bw;
typedef uint64_t    __bitwise __be64_bw;

/* Type limits */
#define U8_MAX      ((u8)~0U)
#define S8_MAX      ((s8)(U8_MAX >> 1))
#define S8_MIN      ((s8)(-S8_MAX - 1))

#define U16_MAX     ((u16)~0U)
#define S16_MAX     ((s16)(U16_MAX >> 1))
#define S16_MIN     ((s16)(-S16_MAX - 1))

#define U32_MAX     ((u32)~0U)
#define S32_MAX     ((s32)(U32_MAX >> 1))
#define S32_MIN     ((s32)(-S32_MAX - 1))

#define U64_MAX     ((u64)~0ULL)
#define S64_MAX     ((s64)(U64_MAX >> 1))
#define S64_MIN     ((s64)(-S64_MAX - 1))

/* Size limits */
#define SIZE_MAX    (~(size_t)0)
#define SSIZE_MAX   ((ssize_t)(SIZE_MAX >> 1))

/* Type conversion helpers */
#define __TYPECHECK(x, y) \
    (!!(sizeof((typeof(x) *)1 == (typeof(y) *)1)))

/* Alignment */
#define ALIGN(x, a)         __ALIGN_KERNEL((x), (a))
#define ALIGN_DOWN(x, a)    __ALIGN_KERNEL_MASK((x), ~(typeof(x))((a) - 1))
#define __ALIGN_KERNEL(x, a) \
    __ALIGN_KERNEL_MASK(x, (typeof(x))((a) - 1))
#define __ALIGN_KERNEL_MASK(x, mask) \
    (((x) + (mask)) & ~(mask))
#define PTR_ALIGN(p, a)     ((typeof(p))ALIGN((unsigned long)(p), (a)))
#define IS_ALIGNED(x, a)    (((x) & ((typeof(x))(a) - 1)) == 0)

/* Round up/down */
#define DIV_ROUND_UP(n, d)  (((n) + (d) - 1) / (d))
#define DIV_ROUND_DOWN_ULL(ll, d) \
    ({ unsigned long long _tmp = (ll); do_div(_tmp, d); _tmp; })

/* min/max/clamp */
#define min(x, y) ({ \
    typeof(x) _min1 = (x); \
    typeof(y) _min2 = (y); \
    (void)(&_min1 == &_min2); \
    _min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({ \
    typeof(x) _max1 = (x); \
    typeof(y) _max2 = (y); \
    (void)(&_max1 == &_max2); \
    _max1 > _max2 ? _max1 : _max2; })

#define min3(x, y, z) min((typeof(x))min(x, y), z)
#define max3(x, y, z) max((typeof(x))max(x, y), z)

#define clamp(val, lo, hi) min((typeof(val))max(val, lo), hi)
#define clamp_t(type, val, lo, hi) \
    min_t(type, max_t(type, val, lo), hi)

#define min_t(type, x, y) ({ \
    type __min1 = (x); \
    type __min2 = (y); \
    __min1 < __min2 ? __min1 : __min2; })

#define max_t(type, x, y) ({ \
    type __max1 = (x); \
    type __max2 = (y); \
    __max1 > __max2 ? __max1 : __max2; })

/* Swap */
#define swap(a, b) \
    do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

/* Array size */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

/* Container of */
#define container_of(ptr, type, member) ({ \
    const typeof(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member)); })

/* Checksum types */
typedef uint16_t    __sum16;
typedef uint32_t    __wsum;
typedef uint32_t    __sum32;

/* UUID/GUID */
typedef struct {
    uint8_t b[16];
} guid_t;

typedef guid_t uuid_t;

/* Time types */
typedef int64_t     ktime_t;
typedef uint64_t    cycles_t;

/* Sequential lock */
typedef unsigned    seqcount_t;

/* PID namespace */
typedef struct {
    uint32_t nr;
} pid_ns_t;

#endif /* _LINUXAB_TYPES_H */