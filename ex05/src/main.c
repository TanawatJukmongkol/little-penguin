// SPDX-License-Identifier: GPL-2.0

#include "../include/main.h"

static const struct file_operations chardev_fops = {
	.owner   = THIS_MODULE,
	.open    = device_open,
	.release = device_release,
	.read    = device_read,
	.write   = device_write,
};

// Global extern
struct s_chrdev dev = CHARDEV(&chardev_fops);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tjukmong");
MODULE_DESCRIPTION("A simple character device driver.");

int __init my_module_init(void)
{
	int ft_dev = register_char_device("fortytwo", &dev);

	if (ft_dev != 0)
		return ft_dev;

	return 0;
}

void __exit my_module_exit(void)
{
	unregister_char_device(&dev);
}

module_init(my_module_init);
module_exit(my_module_exit);
