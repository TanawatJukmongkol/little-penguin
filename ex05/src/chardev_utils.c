#include "../include/main.h"

int register_char_device(const char *dev_name, t_chrdev *dev)
{
	mutex_init(&dev->mutex);
	mutex_lock(&dev->mutex);

	dev->name = dev_name;

	dev->misc.name = dev->name;
	dev->misc.fops = &dev->fops;
	dev->misc.minor = MISC_DYNAMIC_MINOR;

	int misc_dev_ret = misc_register(&dev->misc);
	if (misc_dev_ret < 0) {
		printk(KERN_ALERT "fourtytwo: misc_register() failed: %d\n",
		       misc_dev_ret);
		dev->errno = misc_dev_ret;
		mutex_unlock(&dev->mutex);
		return dev->errno;
	}

	printk(KERN_INFO "fourtytwo: misc device '%s' registered (minor %d)\n",
	       dev->name, dev->misc.minor);

	mutex_unlock(&dev->mutex);
	return 0;
}

int unregister_char_device(t_chrdev *dev)
{
	mutex_lock(&dev->mutex);

	misc_deregister(&dev->misc);

	printk(KERN_INFO "fourtytwo: misc device '%s' unregistered\n",
	       dev->name ? dev->name : "unknown");

	*dev = CHARDEV(NULL, NULL, NULL, NULL);

	mutex_unlock(&dev->mutex);
	mutex_destroy(&dev->mutex);

	return 0;
}
