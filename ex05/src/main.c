// SPDX-License-Identifier: GPL-2.0

#include "../include/main.h"

// Global extern
t_chrdev dev = CHARDEV(device_open, device_release, device_read, device_write);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tjukmong");
MODULE_DESCRIPTION("A simple charactor device driver.");

int __init my_module_init(void)
{
	int ft_dev = register_char_device("fourtytwo", &dev);

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
