/* SPDX-License-Identifier: GPL-2.0 */
#ifndef CHARDEV_H
#define CHARDEV_H

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mutex.h>

struct s_chrdev {
	const struct file_operations *fops;
	struct miscdevice misc;

	const char *name;

	int dev_open;
	int errno;

	struct mutex mutex;
};

#define CHARDEV(_fops) ((struct s_chrdev) { \
	.fops     = (_fops), \
	.misc = { \
		.minor = 1, \
		.name  = NULL, \
		.fops  = NULL, \
	}, \
	.name     = NULL, \
	.dev_open = 0, \
	.errno    = 0, \
})

extern struct s_chrdev dev;

// Char device utils
int register_char_device(const char *dev_name, struct s_chrdev *dev);
int unregister_char_device(struct s_chrdev *dev);

#endif
