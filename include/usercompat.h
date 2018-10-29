/*
 * usercompat.h
 *
 * header file for C++ code to access certain kernel functions without having
 * to include some kernel headers (which can be disastrous). Please note that 
 * this is currently not portable at all
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 6, University of Toronto
 */

#ifndef USERCOMPAT_H
#define USERCOMPAT_H

#ifndef __KERNEL__
#error "This header is meant to be included by C++ kernel source code"
#endif

#ifndef CONFIG_X86_64
#error "This header will NOT work on any other architecture"
#endif

#ifndef __GNUC__
#error "This header has not been tested on other (non-gnu-c) compilers"
#endif

#ifndef __cplusplus
#error "This head is supposed to be included by C++ source code"
#endif

#include "linux/4.4.0.h"

extern "C" {
// (jsun): we implement these ourselves
struct mutex * create_mutex(void);
void destroy_mutex(struct mutex * lock);

// for rbtree 
struct rb_node;

struct rb_root {
	struct rb_node *rb_node;
};

#define RB_ROOT	(struct rb_root) { nullptr, }

// for list/hlist
struct list_head {
	struct list_head *next, *prev;
};

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

// C++ special "under the cover" functions
__extension__ typedef int __guard __attribute__((mode(__DI__)));

void __cxa_pure_virtual();
int __cxa_guard_acquire (__guard *);
void __cxa_guard_release (__guard *);

} /* extern C */

extern "C++" {
void *operator new(size_t size);
void *operator new(size_t size, void * buf);    // placement new
void *operator new[](size_t size);
void operator delete(void *p);
void operator delete[](void *p);

#include "std/list.h"
}

#define WARN(fmt, va...) printk(fmt, ##va)
#define assert(x) (void)(x)

#endif /* USERCOMPAT_H */
 
