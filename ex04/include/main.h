/* SPDX-License-Identifier: GPL-2.0 */

#ifndef MAIN_H
#define MAIN_H

#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/usb.h>

#define USB_CLASS 0x0003
#define USB_SUBCL 0x0001
#define USB_PROTO 0x0001

struct kbd_dev_info {
	struct usb_device *udev;
	char product[128];
};

int my_module_init(void);
void my_module_exit(void);

int usb_probe(struct usb_interface *intf, const struct usb_device_id *id);
void usb_disconnect(struct usb_interface *intf);

#endif
