// SPDX-License-Identifier: GPL-2.0
#include "../include/main.h"

static const char EXPECTED_STRING[] = "tjukmong\n";
static const size_t EXPECTED_LEN =
	sizeof(EXPECTED_STRING) - 1; // 9 bytes ("tjukmong\n")

int device_open(struct inode *inode, struct file *file)
{
	int ret = 0;

	mutex_lock(&dev.mutex);

	if (dev.dev_open) {
		ret = -EBUSY;
		goto out_unlock;
	}

	dev.dev_open++;

out_unlock:
	mutex_unlock(&dev.mutex);

	pr_info("fortytwo: misc device '%s' opened (open count = %d)\n",
		dev.name, dev.dev_open);

	return ret;
}

int device_release(struct inode *inode, struct file *file)
{
	int ret = 0;

	mutex_lock(&dev.mutex);

	if (!dev.dev_open) {
		ret = -EINVAL;
		goto out_unlock;
	}

	dev.dev_open--;

out_unlock:
	mutex_unlock(&dev.mutex);

	pr_info("fortytwo: misc device '%s' closed (open count = %d)\n",
		dev.name, dev.dev_open);

	return ret;
}

ssize_t device_read(struct file *file, char __user *buf, size_t len,
		    loff_t *off)
{
	ssize_t ret;

	pr_info("fortytwo: read() called on '%s'. User requested %zu bytes.\n",
		dev.name, len);

	ret = simple_read_from_buffer(buf, len, off, EXPECTED_STRING,
				      EXPECTED_LEN);
	if (ret < 0)
		pr_err("fortytwo: Failed to copy data to user space.\n");
	else if (ret > 0)
		pr_info("fortytwo: Successfully copied %zd bytes to user.\n",
			ret);

	return ret;
}

ssize_t device_write(struct file *file, const char __user *buf, size_t len,
		     loff_t *off)
{
	char *kbuf;
	ssize_t ret = -EINVAL;

	pr_info("fortytwo: write() called. User provided %zu bytes.\n", len);

	if (len != EXPECTED_LEN && len != EXPECTED_LEN - 1) {
		pr_warn("fortytwo: Write failed. Expected length %zu or %zu, got %zu.\n",
			EXPECTED_LEN - 1, EXPECTED_LEN, len);
		return -EINVAL; // Return error for incorrect length
	}

	kbuf = kmalloc(len + 1, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	if (copy_from_user(kbuf, buf, len)) {
		pr_err("fortytwo: Failed to copy data from user space.\n");
		ret = -EFAULT;
		goto out;
	}
	kbuf[len] = '\0';

	if (strncmp(kbuf, EXPECTED_STRING, len) == 0) {
		pr_info("fortytwo: SUCCESS! Received expected value.\n");
		ret = len; // Return the number of bytes written on success
	} else {
		pr_warn("fortytwo: Invalid value received: '%s'. Returning -EINVAL.\n",
			kbuf);
		ret = -EINVAL;
	}

out:
	kfree(kbuf);

	return ret;
}
