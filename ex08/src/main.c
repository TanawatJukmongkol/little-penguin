// SPDX-License-Identifier: MIT
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/mutex.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Louis Solofrizzo <louis@ne02ptzero.me>");
MODULE_DESCRIPTION("Useless module");

static ssize_t myfd_read(struct file *fp, char __user *user, size_t size,
			 loff_t *offs);
static ssize_t myfd_write(struct file *fp, const char __user *user, size_t size,
			  loff_t *offs);

static const struct file_operations myfd_fops = {
	.owner = THIS_MODULE,
	.read = &myfd_read,
	.write = &myfd_write
};

static struct miscdevice myfd_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "reverse",
	.fops = &myfd_fops
};

static char str[PAGE_SIZE];
static DEFINE_MUTEX(str_lock);

static int __init myfd_init(void)
{
	int retval;

	retval = misc_register(&myfd_device);
	return retval;
}

static void __exit myfd_cleanup(void)
{
	misc_deregister(&myfd_device);
}

static ssize_t myfd_read(struct file *fp, char __user *user, size_t size, loff_t *offs)
{
	size_t len, i, j;
	char c, *tmp;
	ssize_t res;

	tmp = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	mutex_lock(&str_lock);
	len = strlen(str);
	memcpy(tmp, str, len);
	mutex_unlock(&str_lock);
	tmp[len] = 0x0;

	for (i = 0, j = len ? len - 1 : 0; i < j; i++, j--) {
		c = tmp[i];
		tmp[i] = tmp[j];
		tmp[j] = c;
	}

	res = simple_read_from_buffer(user, size, offs, tmp, len);
	kfree(tmp);
	return res;
}

static ssize_t myfd_write(struct file *fp, const char __user *user, size_t size,
			  loff_t *offs)
{
	ssize_t res;

	if (*offs >= sizeof(str) - 1 && size)
		return -ENOSPC;

	mutex_lock(&str_lock);
	res = simple_write_to_buffer(str, sizeof(str) - 1, offs, user, size);
	if (res >= 0)
		str[*offs] = 0x0;
	mutex_unlock(&str_lock);
	return res;
}

module_init(myfd_init);
module_exit(myfd_cleanup);
