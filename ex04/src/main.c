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

#define USB_CLASS 0x0003
#define USB_SUBCL 0x0001
#define USB_PROTO 0x0001

static struct usb_device_id skel_table[] = {
	{ USB_INTERFACE_INFO(USB_CLASS, USB_SUBCL, USB_PROTO) },
	{}
};

MODULE_DEVICE_TABLE(usb, skel_table);

static int usb_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	pr_info("keyboard_driver: keyboard %d:%d connected.\n", id->idVendor,
		id->idProduct);
	printk(KERN_INFO
	       "keyboard_driver: num=%d class=%d subclass=%d proto=%d\n",
	       intf->cur_altsetting->desc.bInterfaceNumber,
	       intf->cur_altsetting->desc.bInterfaceClass,
	       intf->cur_altsetting->desc.bInterfaceSubClass,
	       intf->cur_altsetting->desc.bInterfaceProtocol);
	return 0;
}

static void usb_disconnect(struct usb_interface *intf)
{
	pr_info("keyboard_driver: keyboard disconnected.\n");
	printk(KERN_INFO "keyboard_driver: remove interface: %d\n",
	       intf->cur_altsetting->desc.bInterfaceNumber);
}

struct input_dev *kbd_dev;
static struct usb_driver usb_skell = {
	.name = "keyboard_driver",
	.id_table = skel_table,
	.probe = usb_probe,
	.disconnect = usb_disconnect,
};

int my_module_init(void)
{
	int usb_res;

	pr_info("keyboard_driver: Load USB keyboard driver.\n");
	usb_res = usb_register(&usb_skell);
	if (usb_res)
		return -usb_res;
	return 0;
}

void my_module_exit(void)
{
	pr_info("keyboard_driver: Unload USB keyboard module.\n");
	usb_deregister(&usb_skell);
}

module_init(my_module_init);
module_exit(my_module_exit);
