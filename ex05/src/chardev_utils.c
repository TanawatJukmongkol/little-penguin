// SPDX-License-Identifier: GPL-2.0
#include "../include/main.h"

int register_char_device(const char *dev_name, struct s_chrdev *dev)
{
	mutex_init(&dev->mutex);
	mutex_lock(&dev->mutex);

	dev->name = dev_name;

	dev->misc.name = dev->name;
	dev->misc.fops = dev->fops;
	dev->misc.minor = MISC_DYNAMIC_MINOR;

	int misc_dev_ret = misc_register(&dev->misc);

	if (misc_dev_ret < 0) {
		pr_alert("fortytwo: misc_register() failed: %d\n",
			 misc_dev_ret);
		dev->errno = misc_dev_ret;
		mutex_unlock(&dev->mutex);
		return dev->errno;
	}

	pr_info("fortytwo: misc device '%s' registered (minor %d)\n",
		dev->name, dev->misc.minor);

	mutex_unlock(&dev->mutex);
	return 0;
}

int unregister_char_device(struct s_chrdev *dev)
{
	mutex_lock(&dev->mutex);

	misc_deregister(&dev->misc);

	pr_info("fortytwo: misc device '%s' unregistered\n",
		dev->name ? dev->name : "unknown");

	mutex_unlock(&dev->mutex);
	mutex_destroy(&dev->mutex);

	*dev = CHARDEV(NULL);

	return 0;
}
