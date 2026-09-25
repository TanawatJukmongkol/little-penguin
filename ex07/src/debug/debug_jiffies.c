// SPDX-License-Identifier: GPL-2.0
#include "../../include/debugfs.h"
#include "linux/jiffies.h"

static ssize_t debug_jiffies_read(struct file *filp, char __user *buf, size_t size, loff_t *f_pos);
static int    debug_jiffies_destruct(struct s_debug *dbg);

static const struct file_operations debug_jiffies_fops = {
	.read = debug_jiffies_read,
};

int debug_jiffies_init(struct s_debug *dbg)
{
	dbg->fops = &debug_jiffies_fops;
	dbg->destruct = debug_jiffies_destruct;
	return 0;
}

static int debug_jiffies_destruct(struct s_debug *dbg)
{
	return 0;
}

static ssize_t debug_jiffies_read(struct file *filp, char __user *buf, size_t size, loff_t *f_pos)
{
	char tmp[32];
	int len;

	len = snprintf(tmp, sizeof(tmp), "%lu\n", jiffies);

	return simple_read_from_buffer(buf, size, f_pos, tmp, len);
}
