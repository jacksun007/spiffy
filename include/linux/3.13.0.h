/*
 * 3.13.0.h
 *
 * all the required bits and pieces of the 3.13.0 kernel headers are here
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
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
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

// for __force and asmlinkage
#include <linux/linkage.h>

#define ___GFP_IO               0x40u
#define ___GFP_WAIT             0x10u
#define __GFP_IO        ((__force gfp_t)___GFP_IO)
#define __GFP_WAIT      ((__force gfp_t)___GFP_WAIT)
#define GFP_NOFS        (__GFP_WAIT | __GFP_IO)

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

} // extern C

