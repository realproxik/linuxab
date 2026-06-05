/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/arch/x86/include/asm/atomic.h
 * x86_64 atomic operations using lock prefix
 */

#ifndef _LINUXAB_X86_ATOMIC_H
#define _LINUXAB_X86_ATOMIC_H

#include <stdint.h>

typedef struct {
    volatile int32_t counter;
} atomic_t;

typedef struct {
    volatile int64_t counter;
} atomic64_t;

#define ATOMIC_INIT(i)      { (i) }
#define ATOMIC64_INIT(i)    { (i) }

static inline int32_t atomic_read(const atomic_t *v)
{
    return v->counter;
}

static inline void atomic_set(atomic_t *v, int32_t i)
{
    v->counter = i;
}

static inline void atomic_add(int32_t i, atomic_t *v)
{
    __asm__ volatile ("lock; addl %1,%0"
                      : "+m" (v->counter)
                      : "ir" (i));
}

static inline void atomic_sub(int32_t i, atomic_t *v)
{
    __asm__ volatile ("lock; subl %1,%0"
                      : "+m" (v->counter)
                      : "ir" (i));
}

static inline bool atomic_sub_and_test(int32_t i, atomic_t *v)
{
    uint8_t c;
    __asm__ volatile ("lock; subl %2,%0; sete %1"
                      : "+m" (v->counter), "=qm" (c)
                      : "ir" (i) : "memory");
    return c;
}

static inline void atomic_inc(atomic_t *v)
{
    __asm__ volatile ("lock; incl %0"
                      : "+m" (v->counter));
}

static inline void atomic_dec(atomic_t *v)
{
    __asm__ volatile ("lock; decl %0"
                      : "+m" (v->counter));
}

static inline bool atomic_dec_and_test(atomic_t *v)
{
    uint8_t c;
    __asm__ volatile ("lock; decl %0; sete %1"
                      : "+m" (v->counter), "=qm" (c)
                      :: "memory");
    return c;
}

static inline bool atomic_inc_and_test(atomic_t *v)
{
    uint8_t c;
    __asm__ volatile ("lock; incl %0; sete %1"
                      : "+m" (v->counter), "=qm" (c)
                      :: "memory");
    return c;
}

static inline int32_t atomic_add_return(int32_t i, atomic_t *v)
{
    int32_t __i = i;
    __asm__ volatile ("lock; xaddl %0, %1"
                      : "+r" (i), "+m" (v->counter)
                      :: "memory");
    return i + __i;
}

static inline int32_t atomic_sub_return(int32_t i, atomic_t *v)
{
    return atomic_add_return(-i, v);
}

static inline int32_t atomic_fetch_add(int32_t i, atomic_t *v)
{
    __asm__ volatile ("lock; xaddl %0, %1"
                      : "+r" (i), "+m" (v->counter)
                      :: "memory");
    return i;
}

/* 64-bit variants */
static inline int64_t atomic64_read(const atomic64_t *v)
{
    return v->counter;
}

static inline void atomic64_set(atomic64_t *v, int64_t i)
{
    v->counter = i;
}

static inline void atomic64_add(int64_t i, atomic64_t *v)
{
    __asm__ volatile ("lock; addq %1,%0"
                      : "+m" (v->counter)
                      : "er" (i));
}

static inline int64_t atomic64_add_return(int64_t i, atomic64_t *v)
{
    int64_t __i = i;
    __asm__ volatile ("lock; xaddq %0, %1"
                      : "+r" (i), "+m" (v->counter)
                      :: "memory");
    return i + __i;
}

static inline int64_t atomic64_fetch_add(int64_t i, atomic64_t *v)
{
    __asm__ volatile ("lock; xaddq %0, %1"
                      : "+r" (i), "+m" (v->counter)
                      :: "memory");
    return i;
}

static inline void atomic64_inc(atomic64_t *v)
{
    __asm__ volatile ("lock; incq %0" : "+m" (v->counter));
}

static inline void atomic64_dec(atomic64_t *v)
{
    __asm__ volatile ("lock; decq %0" : "+m" (v->counter));
}

static inline bool atomic64_dec_and_test(atomic64_t *v)
{
    uint8_t c;
    __asm__ volatile ("lock; decq %0; sete %1"
                      : "+m" (v->counter), "=qm" (c)
                      :: "memory");
    return c;
}

/* Exchange */
static inline int32_t atomic_xchg(atomic_t *v, int32_t new)
{
    __asm__ volatile ("xchgl %0, %1"
                      : "=r" (new), "+m" (v->counter)
                      : "0" (new)
                      : "memory");
    return new;
}

static inline int64_t atomic64_xchg(atomic64_t *v, int64_t new)
{
    __asm__ volatile ("xchgq %0, %1"
                      : "=r" (new), "+m" (v->counter)
                      : "0" (new)
                      : "memory");
    return new;
}

/* Compare and exchange */
static inline bool atomic_cmpxchg(atomic_t *v, int32_t old, int32_t new)
{
    int32_t prev;
    __asm__ volatile ("lock; cmpxchgl %2, %1"
                      : "=a" (prev), "+m" (v->counter)
                      : "r" (new), "0" (old)
                      : "memory");
    return prev == old;
}

static inline bool atomic64_cmpxchg(atomic64_t *v, int64_t old, int64_t new)
{
    int64_t prev;
    __asm__ volatile ("lock; cmpxchgq %2, %1"
                      : "=a" (prev), "+m" (v->counter)
                      : "r" (new), "0" (old)
                      : "memory");
    return prev == old;
}

#endif /* _LINUXAB_X86_ATOMIC_H */
