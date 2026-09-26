// SPDX-License-Identifier: GPL-2.0
#include "../../include/debugfs.h"
#include "linux/slab.h"
#include "linux/mutex.h"

static const char EXPECTED_STRING[] = "tjukmong\n";
static const size_t EXPECTED_LEN = sizeof(EXPECTED_STRING) - 1;

static DEFINE_MUTEX(id_mutex);
static int id_open;

struct s_debug *debug_fs_id;

static int debug_id_open(struct inode *inode, struct file *filp);
static int debug_id_release(struct inode *inode, struct file *filp);
static ssize_t debug_id_read(struct file *filp, char __user *buf, size_t size, loff_t *f_pos);
static ssize_t debug_id_write(struct file *filp, const char __user *buf, size_t size,
			      loff_t *f_pos);
static int    debug_id_destruct(struct s_debug *dbg);

static const struct file_operations debug_id_fops = {
	.owner = THIS_MODULE,
	.open = debug_id_open,
	.release = debug_id_release,
	.read = debug_id_read,
	.write = debug_id_write
};

int debug_id_init(struct s_debug *dbg)
{
	dbg->fops = &debug_id_fops;
	dbg->destruct = debug_id_destruct;
	debug_fs_id = dbg;
	return 0;
}

static int debug_id_destruct(struct s_debug *dbg)
{
	debug_fs_id = NULL;
	return 0;
}

static int debug_id_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	mutex_lock(&id_mutex);

	if (id_open) {
		ret = -EBUSY;
		goto out_unlock;
	}

	id_open++;

out_unlock:
	mutex_unlock(&id_mutex);

	pr_info("debugfs: file '%s' opened (open count = %d)\n",
		debug_fs_id->name, id_open);

	return ret;
}

static int debug_id_release(struct inode *inode, struct file *filp)
{
	int ret = 0;

	mutex_lock(&id_mutex);

	if (!id_open) {
		ret = -EINVAL;
		goto out_unlock;
	}

	id_open--;

out_unlock:
	mutex_unlock(&id_mutex);

	pr_info("debugfs: file '%s' closed (open count = %d)\n",
		debug_fs_id->name, id_open);

	return ret;
}

static ssize_t debug_id_read(struct file *filp, char __user *buf, size_t size, loff_t *f_pos)
{
	ssize_t ret;

	pr_info("debugfs: read() called on '%s'. User requested %zu bytes.\n",
		debug_fs_id->name, size);

	ret = simple_read_from_buffer(buf, size, f_pos, EXPECTED_STRING,
				      EXPECTED_LEN);
	if (ret < 0)
		pr_err("debugfs: Failed to copy data to user space.\n");
	else if (ret > 0)
		pr_info("debugfs: Successfully copied %zd bytes to user.\n",
			ret);

	return ret;
}

static ssize_t debug_id_write(struct file *filp, const char __user *buf, size_t size, loff_t *f_pos)
{
	char *kbuf;
	ssize_t ret = -EINVAL;

	pr_info("debugfs: write() called. User provided %zu bytes.\n", size);

	if (size != EXPECTED_LEN && size != EXPECTED_LEN - 1) {
		pr_warn("debugfs: Write failed. Expected length %zu or %zu, got %zu.\n",
			EXPECTED_LEN - 1, EXPECTED_LEN, size);
		return -EINVAL;
	}

	kbuf = kmalloc(size + 1, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	if (copy_from_user(kbuf, buf, size)) {
		pr_err("debugfs: Failed to copy data from user space.\n");
		ret = -EFAULT;
		goto out;
	}
	kbuf[size] = '\0';

	if (strncmp(kbuf, EXPECTED_STRING, size) == 0) {
		pr_info("debugfs: SUCCESS! Received expected value.\n");
		ret = size;
	} else {
		pr_warn("debugfs: Invalid value received: '%s'. Returning -EINVAL.\n",
			kbuf);
		ret = -EINVAL;
	}

out:
	kfree(kbuf);

	return ret;
}
