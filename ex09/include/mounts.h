/* SPDX-License-Identifier: GPL-2.0 */
#ifndef MOUNTS_H
#define MOUNTS_H

#include <linux/fs.h>

/*
 * 0: the subject's format, "<name> <mount point>" (name is "root" for "/").
 * 1: the full /proc/mounts format. Set with `make MYMOUNTS_FULL=1`.
 */
#ifndef MYMOUNTS_FULL
#define MYMOUNTS_FULL 0
#endif

int mymounts_open(struct inode *inode, struct file *file);
int mymounts_release(struct inode *inode, struct file *file);

#endif
