// SPDX-License-Identifier: GPL-2.0
#include "../../include/debugfs.h"
#include "linux/slab.h"
#include "linux/mutex.h"

static ssize_t debug_foo_read(struct file *filp, char __user *buf, size_t size,
			      loff_t *f_pos);
static ssize_t debug_foo_write(struct file *filp, const char __user *buf, size_t size,
			       loff_t *f_pos);
static int     debug_foo_destruct(t_debug *dbg);

static char *buffer;
static size_t buffer_len;
static DEFINE_MUTEX(foo_lock);

int debug_foo_init(t_debug *dbg)
{
	dbg->fops.read = debug_foo_read;
	dbg->fops.write = debug_foo_write;
	dbg->destruct = debug_foo_destruct;
	buffer = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;
	return 0;
}

static int debug_foo_destruct(t_debug *dbg)
{
	kfree(buffer);
	buffer = NULL;
	buffer_len = 0;
	return 0;
}

static ssize_t debug_foo_read(struct file *filp, char __user *buf, size_t size, loff_t *f_pos)
{
	ssize_t ret;

	mutex_lock(&foo_lock);

	/* End of file */
	if (*f_pos >= buffer_len) {
		ret = 0;
		goto out_unlock;
	}

	/* Trim size to remaining data */
	if (size > buffer_len - *f_pos)
		size = buffer_len - *f_pos;

	if (copy_to_user(buf, buffer + *f_pos, size)) {
		ret = -EFAULT;
		goto out_unlock;
	}

	*f_pos += size;
	ret = size;

out_unlock:
	mutex_unlock(&foo_lock);

	return ret;
}

static ssize_t debug_foo_write(struct file *filp, const char __user *buf, size_t size,
			       loff_t *f_pos)
{
	ssize_t ret;

	if (size > PAGE_SIZE - 1)
		size = PAGE_SIZE - 1;

	mutex_lock(&foo_lock);

	if (copy_from_user(buffer, buf, size)) {
		ret = -EFAULT;
		goto out_unlock;
	}

	buffer[size] = '\0';
	buffer_len = size;
	ret = size;

out_unlock:
	mutex_unlock(&foo_lock);

	return ret;
}
