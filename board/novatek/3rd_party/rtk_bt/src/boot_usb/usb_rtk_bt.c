/*
 * Most of this source has been derived from the Linux USB
 * project:
 *   (c) 1999-2002 Matthew Dharm (mdharm-usb@one-eyed-alien.net)
 *   (c) 2000 David L. Brown, Jr. (usb-storage@davidb.org)
 *   (c) 1999 Michael Gee (michael@linuxspecific.com)
 *   (c) 2000 Yggdrasil Computing, Inc.
 *
 *
 * Adapted for U-Boot:
 *   (C) Copyright 2001 Denis Peter, MPL AG Switzerland
 * Driver model conversion:
 *   (C) Copyright 2015 Google, Inc
 *
 * For BBB support (C) Copyright 2003
 * Gary Jennejohn, DENX Software Engineering <garyj@denx.de>
 *
 * BBB support based on /sys/dev/usb/umass.c from
 * FreeBSD.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* Note:
 * Currently only the CBI transport protocoll has been implemented, and it
 * is only tested with a TEAC USB Floppy. Other Massstorages with CBI or CB
 * transport protocoll may work as well.
 */
/*
 * New Note:
 * Support for USB Mass Storage Devices (BBB) has been added. It has
 * only been tested with USB memory sticks.
 */


#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <inttypes.h>
#include <mapmem.h>
#include <memalign.h>
#include <asm/byteorder.h>
#include <asm/processor.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

#include <part.h>
#include <usb.h>
#include <osi_usb.h>

#undef BBB_COMDAT_TRACE
#undef BBB_XPORT_TRACE

#include <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */

#ifdef CONFIG_DM_USB
struct osi_usb_device *osi_udev_rtk_bt;

rtk_bt_model_id bt_model_id = RTK_BT_UNKNOWN;

static int usb_rtk_bt_probe(struct udevice *dev)
{
	
	struct usb_device *udev = dev_get_parent_priv(dev);

	printf("!!!!usb_rtk_bt_probe\n");
	osi_udev_rtk_bt->devnum = udev->devnum;
	osi_udev_rtk_bt->speed = udev->speed;
	strcpy(osi_udev_rtk_bt->mf, udev->mf);
	strcpy(osi_udev_rtk_bt->prod, udev->prod);
	strcpy(osi_udev_rtk_bt->serial, udev->serial);
	osi_udev_rtk_bt->maxpacketsize = udev->maxpacketsize;
	memcpy(&osi_udev_rtk_bt->descriptor, &udev->descriptor, USB_DT_DEVICE_SIZE);
	osi_udev_rtk_bt->context = udev;
	//save mode
	if (udev->descriptor.idVendor == 0x0bda) {
		switch(udev->descriptor.idProduct) {
			case 0x818c:
				bt_model_id = RTK_BT_8761;
				printf("==detect BT model 8761===\n");
				break;
			case 0xb82c:
				bt_model_id = RTK_BT_8822;
				printf("==detect BT model 8822===\n");
				break;
			case 0xd723:
				bt_model_id = RTK_BT_8723;
				printf("==detect BT model 8723===\n");
				break;
			default:
				bt_model_id = RTK_BT_UNKNOWN;
				printf("==Unknown BT model 0x%x===\n", udev->descriptor.idProduct);
				break;
		}
	}

	return 0;
}

static const struct udevice_id usb_rtk_bt_ids[] = {
	{ .compatible = "usb-rtk-bt" },
	{ }
};

U_BOOT_DRIVER(usb_rtk_bt) = {
	.name	= "usb_rtk_bt",
	.id	= UCLASS_MISC,
	.of_match = usb_rtk_bt_ids,
	.probe = usb_rtk_bt_probe,
};

UCLASS_DRIVER(usb_rtk_bt) = {
	.id		= UCLASS_MISC,
	.name		= "usb_rtk_bt",
};

static const struct usb_device_id rtk_bt_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_WIRELESS_CONTROLLER
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_rtk_bt, rtk_bt_id_table);
#endif
