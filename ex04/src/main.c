// SPDX-License-Identifier: GPL-2.0
#include "../include/main.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tjukmong");
MODULE_DESCRIPTION("A hello, world USB driver for 42 subject");

static struct usb_device_id skel_table[] = {
	{ USB_INTERFACE_INFO(USB_CLASS, USB_SUBCL, USB_PROTO) },
	{}
};

static struct usb_driver usb_skell = {
	.name = "keyboard_driver",
	.id_table = skel_table,
	.probe = usb_probe,
	.disconnect = usb_disconnect,
};

MODULE_DEVICE_TABLE(usb, skel_table);

int my_module_init(void)
{
	int usb_res;

	pr_info("keyboard_driver: Loading USB keyboard driver...\n");
	usb_res = usb_register(&usb_skell);
	if (usb_res)
		return -usb_res;
	pr_info("keyboard_driver: Loading USB keyboard driver successfully!\n");
	return 0;
}

void my_module_exit(void)
{
	pr_info("keyboard_driver: Unloading USB keyboard module...\n");
	usb_deregister(&usb_skell);
	pr_info("keyboard_driver: Goodbye!\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
