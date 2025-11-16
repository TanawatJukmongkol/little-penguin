#ifndef CHARDEV_H
#define CHARDEV_H

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mutex.h>

typedef struct s_chrdev {
	struct file_operations fops;
	struct miscdevice misc;

	const char *name;

	int dev_open;
	int errno;

	struct mutex mutex;
} t_chrdev;

#define CHARDEV(_open, _release, _read, _write) \
	(t_chrdev)                              \
	{                                       \
		.fops = { \
			.owner   = THIS_MODULE, \
			.open    = (_open), \
			.release = (_release), \
			.read    = (_read), \
			.write   = (_write), \
		}, \
		.misc = { \
			.minor = 1, \
			.name  = NULL, \
			.fops  = NULL, \
		}, \
		.name     = NULL, \
		.dev_open = 0, \
		.errno    = 0,                     \
	}

extern t_chrdev dev;

// Char device utils
int register_char_device(const char *dev_name, t_chrdev *dev);
int unregister_char_device(t_chrdev *dev);

#endif