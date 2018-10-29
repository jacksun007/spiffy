/*
 * 4.4.0.h
 *
 * all the required bits and pieces of the 4.4.0 kernel headers are here
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2016, University of Toronto
 */

extern "C" {

// for error numbers -- this file is safe (I checked!)
#include <asm/errno.h>

typedef long __kernel_off_t ;
typedef unsigned long __kernel_size_t ;
typedef long __kernel_ssize_t ;

typedef __kernel_off_t off_t ;
typedef __kernel_size_t size_t ;
typedef __kernel_ssize_t ssize_t ;

typedef unsigned int u32;
typedef unsigned int __u32;
typedef unsigned long long u64;
typedef unsigned long long __u64;
typedef unsigned char u8;
typedef unsigned char __u8;
typedef unsigned short u16;
typedef unsigned short __u16;

// for compiler options that does extra checks (such as __bitwise__)
#ifdef __CHECKER__
#define __bitwise__    __attribute__((bitwise))
#define __force        __attribute__((force))
#define __user         __attribute__((noderef, address_space(1)))
#else /* NO CHECK */
#define __bitwise__
#define __force
#define __user
#endif /* __CHECKER__ */

// from compiler-gcc.h
#define GCC_VERSION (__GNUC__ * 10000           \
                     + __GNUC_MINOR__ * 100     \
                     + __GNUC_PATCHLEVEL__)

#if GCC_VERSION < 30300
#define __used                 __attribute__((__unused__))
#else
#define __used                 __attribute__((__used__))
#endif

#if GCC_VERSION >= 40600
#define __visible       __attribute__((externally_visible))
#else 
#define __visible
#endif

#define __printf(a, b)          __attribute__((format(printf, a, b)))
#define __cold                  __attribute__((__cold__))

#ifdef __CHECK_ENDIAN__
#define __bitwise __bitwise__
#else
#define __bitwise
#endif

typedef u16 __bitwise __le16;
typedef u16 __bitwise __be16;
typedef u32 __bitwise __le32;
typedef u32 __bitwise __be32;
typedef u64 __bitwise __le64;
typedef u64 __bitwise __be64;

// for <linux/slab.h>
typedef unsigned __bitwise__ gfp_t;

// from linux/linkage.h
#ifdef __cplusplus
#define CPP_ASMLINKAGE extern "C"
#else
#define CPP_ASMLINKAGE
#endif
#ifndef asmlinkage
#define asmlinkage CPP_ASMLINKAGE
#endif

// (jsun): these are completely different from 3.13.0!
#define ___GFP_IO               0x40u
#define ___GFP_DIRECT_RECLAIM   0x400000u
#define ___GFP_KSWAPD_RECLAIM   0x2000000u

#define __GFP_RECLAIM ((__force gfp_t)(___GFP_DIRECT_RECLAIM|___GFP_KSWAPD_RECLAIM))
#define __GFP_IO        ((__force gfp_t)___GFP_IO)

#define GFP_NOFS        (__GFP_RECLAIM | __GFP_IO)

extern void free(const void *);
extern void * malloc(size_t size, gfp_t flags=GFP_NOFS);
extern void * realloc(const void *, size_t size, gfp_t flags=GFP_NOFS);

// string functions
extern char * strcpy(char *,const char *);
extern char * strncpy(char *,const char *, __kernel_size_t);
extern int strcmp(const char *, const char *);
extern void * memcpy(void *, const void *, __kernel_size_t);
extern void * memset(void *,int,__kernel_size_t);
extern int memcmp(const void *,const void *,__kernel_size_t);

// mutex functions
struct mutex;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
extern void mutex_lock_nested(struct mutex *lock, unsigned int subclass);
#define mutex_lock(lock) mutex_lock_nested(lock, 0)
#else
extern void mutex_lock(struct mutex *lock);
#endif
extern void mutex_unlock(struct mutex *lock);

asmlinkage __printf(1, 2) __cold int printk(const char *fmt, ...);
extern __printf(3, 4) int snprintf(char *buf, size_t size, const char *fmt, ...);

// used for creating modules
#include <linux/export.h>

} // extern C

