// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tjukmong");
MODULE_DESCRIPTION("A hello, world USB driver for 42 subject");

int my_module_init(void);
void my_module_exit(void);

// ID 24ae:2013 Shenzhen Rapoo Technology Co., Ltd. Rapoo 2.4G Wireless Device
#define USB_CLASS 0x0003
#define USB_SUBCL 0x0000
#define USB_PROTO 0x0001

static struct usb_device_id skel_table[] = {
	{ USB_INTERFACE_INFO(USB_CLASS, USB_SUBCL, USB_PROTO) },
	{}
};

MODULE_DEVICE_TABLE(usb, skel_table);

static int usb_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	printk(KERN_INFO "rapoo - num=%d class=%d subclass=%d proto=%d\n",
	       intf->cur_altsetting->desc.bInterfaceNumber,
	       intf->cur_altsetting->desc.bInterfaceClass,
	       intf->cur_altsetting->desc.bInterfaceSubClass,
	       intf->cur_altsetting->desc.bInterfaceProtocol);
	return 0;
}

static void usb_disconnect(struct usb_interface *intf)
{
	printk(KERN_INFO "rapoo - remove interface: %d\n",
	       intf->cur_altsetting->desc.bInterfaceNumber);
}

struct input_dev *kbd_dev;
static struct usb_driver usb_skell = {
	.name = "rapoo_wireless",
	.id_table = skel_table,
	.probe = usb_probe,
	.disconnect = usb_disconnect,
};

int __init my_module_init(void)
{
	int usb_res;

	pr_info("rapoo - Init USB driver.\n");
	usb_res = usb_register(&usb_skell);
	if (usb_res)
		return -usb_res;
	return 0;
}

void __exit my_module_exit(void)
{
	pr_info("rapoo - Cleaning up module.\n");
	usb_deregister(&usb_skell);
}

module_init(my_module_init);
module_exit(my_module_exit);
