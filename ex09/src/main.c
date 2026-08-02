// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include "../include/mounts.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tjukmong");
MODULE_DESCRIPTION("A procfs entry mirroring /proc/mounts");

/* File operations bound to the /proc/mymounts entry */
static const struct proc_ops mymounts_fops = {
	.proc_open    = mymounts_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

static struct proc_dir_entry *mymounts_entry;

/* Driver Initialization */
static int __init mymounts_init(void)
{
	mymounts_entry = proc_create("mymounts", 0444, NULL, &mymounts_fops);
	if (!mymounts_entry) {
		pr_err("Failed to register /proc/mymounts\n");
		return -ENOMEM;
	}
	pr_info("/proc/mymounts driver initialized successfully\n");
	return 0;
}

/* Driver Cleanup */
static void __exit mymounts_exit(void)
{
	proc_remove(mymounts_entry);
	pr_info("/proc/mymounts driver unloaded\n");
}

module_init(mymounts_init);
module_exit(mymounts_exit);
