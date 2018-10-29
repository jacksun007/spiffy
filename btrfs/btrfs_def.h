/*
 * btrfs_def.h
 *
 * defines from linux/btrfs.h
 *
 */

#ifndef BTRFS_DEF_H
#define BTRFS_DEF_H

#include <jd.h>

#ifdef __KERNEL__
#include <usercompat.h>
#else
#include <kerncompat.h>
#endif

#define BTRFS_FSID_SIZE 16
#define BTRFS_UUID_SIZE 16

#endif
