/*
 * kerncompat.h
 *
 * header file for kernel defines. Borrowed code from btrfs-progs/kerncompat.h
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2015, University of Toronto
 */

#ifndef KERNCOMPAT_H
#define KERNCOMPAT_H

#include <stdio.h>
#include <errno.h>
#include <assert.h>
 
typedef unsigned int u32;
typedef unsigned int __u32;
typedef unsigned long long u64;
typedef unsigned long long __u64;
typedef unsigned char u8;
typedef unsigned char __u8;
typedef unsigned short u16;
typedef unsigned short __u16;

#define __bitwise

typedef u16 __bitwise __le16;
typedef u16 __bitwise __be16;
typedef u32 __bitwise __le32;
typedef u32 __bitwise __be32;
typedef u64 __bitwise __le64;
typedef u64 __bitwise __be64;

#define WARN(fmt, va...) fprintf(stderr, fmt, ##va)
#define BUG_ON(cond) assert(!(cond))

#define printk(fmt, va...) printf(fmt, ##va)
#define kmalloc(size, flag) malloc(size)
#define kfree(ptr) free(ptr)

#define __user 
#define EXPORT_SYMBOL_GPL(...) 

// (jsun): we don't need mutex in user space (for now) 
#define create_mutex(...) NULL
#define mutex_lock(...) 
#define mutex_unlock(...)
#define destroy_mutex(...)

#define stringify(s) _stringify(s)
#define _stringify(s) #s
 
#endif /* KERNCOMPAT_H */
 
