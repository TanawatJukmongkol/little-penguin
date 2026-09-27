/* SPDX-License-Identifier: GPL-2.0 */
#ifndef MAIN_H
#define MAIN_H

#define SUCCESS 0
#define EOF 0

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include "chardev.h"

// Trace file open/close/read/write. Off by default; enable with `make TRACE_DBG=1`.
#ifndef TRACE_DBG
# define TRACE_DBG 0
#endif

#define pr_trace(fmt, ...)					\
	do {							\
		if (TRACE_DBG)					\
			pr_info(fmt, ##__VA_ARGS__);		\
	} while (0)

int __init my_module_init(void);
void __exit my_module_exit(void);

// Device fops
int device_open(struct inode *inode, struct file *file);
int device_release(struct inode *inode, struct file *file);
ssize_t device_read(struct file *file, char __user *buf, size_t len,
		    loff_t *off);
ssize_t device_write(struct file *file, const char __user *buf, size_t len,
		     loff_t *off);

#endif
