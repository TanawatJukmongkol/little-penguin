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
	const char *data_to_send = "tjukmong\n";
	size_t data_len = strlen(data_to_send);
	size_t bytes_to_copy;
	unsigned long uncopied;

	if (*off == data_len)
		return EOF;

	bytes_to_copy = min(len, data_len);

	pr_info("fortytwo: read() called on '%s'. User requested %zu bytes.\n",
		dev.name, len);

	uncopied = copy_to_user(buf, data_to_send, bytes_to_copy);

	if (uncopied) {
		pr_err("fortytwo: Failed to copy %lu bytes to user space.\n",
		       uncopied);
		return -EFAULT;
	}

	pr_info("fortytwo: Successfully copied %zu bytes to user.\n",
		bytes_to_copy);

	*off += bytes_to_copy;

	return bytes_to_copy;
}

ssize_t device_write(struct file *file, const char __user *buf, size_t len,
		     loff_t *off)
{
	char *kbuf;
	ssize_t ret = -EINVAL;

	pr_info("fortytwo: write() called. User provided %zu bytes.\n", len);

	if (len != EXPECTED_LEN) {
		pr_warn("fortytwo: Write failed. Expected length %zu, got %zu.\n",
			EXPECTED_LEN, len);
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
