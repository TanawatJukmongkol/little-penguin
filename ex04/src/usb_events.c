// SPDX-License-Identifier: GPL-2.0
#include "../include/main.h"

static int devices = 0;
static char *envp[] = { "DRIVER_UNLOAD=1", NULL };

// printk(KERN_INFO
//        "keyboard_driver: num=%d class=%d subclass=%d proto=%d\n",
//        intf->cur_altsetting->desc.bInterfaceNumber,
//        intf->cur_altsetting->desc.bInterfaceClass,
//        intf->cur_altsetting->desc.bInterfaceSubClass,
//        intf->cur_altsetting->desc.bInterfaceProtocol);

int usb_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	struct kbd_dev_info *info;
	size_t dev_name_len = 0;

	info = kmalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;
	info->udev = udev;
	strscpy(info->product, "unknown USB device", sizeof(info->product));
	dev_name_len = usb_string(udev, udev->descriptor.iProduct,
				  info->product, sizeof(info->product));
	info->product[dev_name_len] = '\0';
	usb_set_intfdata(intf, info);
	pr_info("keyboard_driver: %s [%04x:%04x] connected.\n", info->product,
		udev->descriptor.idVendor, udev->descriptor.idProduct);
	devices++;
	return 0;
}
// RN_INFO "keyboard_driver: remove interface: %d\n",
//        intf->cur_altsetting->desc.bInterfaceNumber);

void usb_disconnect(struct usb_interface *intf)
{
	struct kbd_dev_info *info = usb_get_intfdata(intf);

	pr_info("keyboard_driver: %s disconnected. %d devices left\n", info->product, devices - 1);
	usb_set_intfdata(intf, NULL);
	kfree(info);
	if (--devices == 0)
		kobject_uevent_env(&intf->dev.kobj, KOBJ_CHANGE, envp);
}
