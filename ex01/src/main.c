// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tjukmong");

int init_module(void)
{
	pr_info("Hello world!\n");
	return 0;
}

void cleanup_module(void)
{
	pr_info("Cleaning up module.\n");
}
