// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tjukmong");

int my_module_init(void);
void my_module_exit(void);

int my_module_init(void)
{
	pr_info("Hello world!\n");
	return 0;
}

void my_module_exit(void)
{
	pr_info("Cleaning up module.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
