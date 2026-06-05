// SPDX-License-Identifier: GPL-2.0
/*
 * linuxab/lib/types.c
 * Type helper functions and 64-bit division
 */

#include "types.h"
#include "printk.h"

/*
 * 64-bit division and modulo
 * x86_64 has hardware div, but this handles edge cases
 */

/* Unsigned 64-by-32 division */
uint32_t __div64_32(uint64_t *n, uint32_t base)
{
    uint64_t rem = *n;
    uint64_t b = base;
    uint64_t res, d = 1;
    uint32_t high = rem >> 32;

    /* Reduce the thing a bit first */
    res = 0;
    if (high >= base) {
        high /= base;
        res = (uint64_t)high << 32;
        rem -= (uint64_t)(high * base) << 32;
    }

    while ((int64_t)b > 0 && b < rem) {
        b = b + b;
        d = d + d;
    }

    do {
        if (rem >= b) {
            rem -= b;
            res += d;
        }
        b >>= 1;
        d >>= 1;
    } while (d);

    *n = res;
    return (uint32_t)rem;
}

/* Signed 64-by-64 division */
int64_t div64_s64(int64_t dividend, int64_t divisor)
{
    int64_t res;

    if (divisor == 0) {
        printk(KERN_WARNING "div64_s64: division by zero\n");
        return 0;
    }

    res = dividend / divisor;
    return res;
}

/* Unsigned 64-by-64 division */
uint64_t div64_u64(uint64_t dividend, uint64_t divisor)
{
    if (divisor == 0) {
        printk(KERN_WARNING "div64_u64: division by zero\n");
        return 0;
    }
    return dividend / divisor;
}

/*
 * String to number conversions
 */

/* Simple atoi */
int atoi(const char *s)
{
    int res = 0;
    int sign = 1;

    while (*s == ' ' || *s == '\t')
        s++;

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }

    return res * sign;
}

/* Simple atol */
long atol(const char *s)
{
    long res = 0;
    int sign = 1;

    while (*s == ' ' || *s == '\t')
        s++;

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }

    return res * sign;
}

/* atoll for 64-bit */
long long atoll(const char *s)
{
    long long res = 0;
    int sign = 1;

    while (*s == ' ' || *s == '\t')
        s++;

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }

    return res * sign;
}

/* strtoul */
unsigned long strtoul(const char *s, char **endptr, int base)
{
    unsigned long res = 0;
    int digit;

    if (base == 0) {
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            base = 16;
            s += 2;
        } else if (s[0] == '0') {
            base = 8;
            s++;
        } else {
            base = 10;
        }
    }

    while (1) {
        if (*s >= '0' && *s <= '9')
            digit = *s - '0';
        else if (*s >= 'a' && *s <= 'z')
            digit = *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'Z')
            digit = *s - 'A' + 10;
        else
            break;

        if (digit >= base)
            break;

        res = res * base + digit;
        s++;
    }

    if (endptr)
        *endptr = (char *)s;

    return res;
}

/* strtoull */
unsigned long long strtoull(const char *s, char **endptr, int base)
{
    unsigned long long res = 0;
    int digit;

    if (base == 0) {
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            base = 16;
            s += 2;
        } else if (s[0] == '0') {
            base = 8;
            s++;
        } else {
            base = 10;
        }
    }

    while (1) {
        if (*s >= '0' && *s <= '9')
            digit = *s - '0';
        else if (*s >= 'a' && *s <= 'z')
            digit = *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'Z')
            digit = *s - 'A' + 10;
        else
            break;

        if (digit >= base)
            break;

        res = res * base + digit;
        s++;
    }

    if (endptr)
        *endptr = (char *)s;

    return res;
}

/* itoa - integer to ascii */
char *itoa(int val, char *buf, int base)
{
    char *p = buf;
    char *p1, *p2;
    unsigned int uv;
    int sign = 0;

    if (base < 2 || base > 16) {
        *p = '\0';
        return buf;
    }

    if (val < 0 && base == 10) {
        sign = 1;
        uv = -val;
    } else {
        uv = val;
    }

    do {
        int rem = uv % base;
        *p++ = (rem < 10) ? ('0' + rem) : ('a' + rem - 10);
        uv /= base;
    } while (uv > 0);

    if (sign)
        *p++ = '-';

    *p = '\0';

    /* Reverse */
    p1 = buf;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }

    return buf;
}

/* ltoa for long */
char *ltoa(long val, char *buf, int base)
{
    char *p = buf;
    char *p1, *p2;
    unsigned long uv;
    int sign = 0;

    if (base < 2 || base > 16) {
        *p = '\0';
        return buf;
    }

    if (val < 0 && base == 10) {
        sign = 1;
        uv = -val;
    } else {
        uv = val;
    }

    do {
        int rem = uv % base;
        *p++ = (rem < 10) ? ('0' + rem) : ('a' + rem - 10);
        uv /= base;
    } while (uv > 0);

    if (sign)
        *p++ = '-';

    *p = '\0';

    p1 = buf;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }

    return buf;
}

/* lltoa for long long */
char *lltoa(long long val, char *buf, int base)
{
    char *p = buf;
    char *p1, *p2;
    unsigned long long uv;
    int sign = 0;

    if (base < 2 || base > 16) {
        *p = '\0';
        return buf;
    }

    if (val < 0 && base == 10) {
        sign = 1;
        uv = -val;
    } else {
        uv = val;
    }

    do {
        int rem = uv % base;
        *p++ = (rem < 10) ? ('0' + rem) : ('a' + rem - 10);
        uv /= base;
    } while (uv > 0);

    if (sign)
        *p++ = '-';

    *p = '\0';

    p1 = buf;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }

    return buf;
}

/* utoa for unsigned */
char *utoa(unsigned int val, char *buf, int base)
{
    char *p = buf;
    char *p1, *p2;

    if (base < 2 || base > 16) {
        *p = '\0';
        return buf;
    }

    do {
        int rem = val % base;
        *p++ = (rem < 10) ? ('0' + rem) : ('a' + rem - 10);
        val /= base;
    } while (val > 0);

    *p = '\0';

    p1 = buf;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }

    return buf;
}

/* Type size pretty-print */
void print_size(uint64_t size, const char *suffix)
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int i = 0;
    uint64_t rem = 0;

    while (size >= 1024 && i < 5) {
        rem = size % 1024;
        size /= 1024;
        i++;
    }

    if (i == 0 || rem == 0)
        printk("%llu %s%s\n", (unsigned long long)size, units[i], suffix);
    else
        printk("%llu.%llu %s%s\n", (unsigned long long)size,
               (unsigned long long)(rem * 100 / 1024), units[i], suffix);
}

/* Hex dump helper */
void hex_dump(const void *data, size_t len)
{
    const unsigned char *p = data;
    const char *hex = "0123456789ABCDEF";

    for (size_t i = 0; i < len; i++) {
        if (i > 0 && (i % 16) == 0)
            printk("\n");
        else if (i > 0 && (i % 8) == 0)
            printk(" ");

        printk("%c%c ", hex[p[i] >> 4], hex[p[i] & 0xF]);
    }
    printk("\n");
}