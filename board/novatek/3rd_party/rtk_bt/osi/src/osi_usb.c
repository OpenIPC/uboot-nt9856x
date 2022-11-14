#include <memalign.h>
#include <usb.h>
#include "osi_usb.h"

int osi_usb_control_msg(struct osi_usb_device *dev, unsigned int pipe,
		unsigned char request, unsigned char requesttype,
		unsigned short value, unsigned short index,
		void *data, unsigned short size, int timeout)
{
    struct usb_device *udev = dev->context;
		
	ALLOC_CACHE_ALIGN_BUFFER(struct devrequest, setup_packet, 1);
		int err;
	
	if ((timeout == 0)) {
		/* request for a asynch control pipe is not allowed */
		return -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->value = cpu_to_le16(value);
	setup_packet->index = cpu_to_le16(index);
	setup_packet->length = cpu_to_le16(size);
	debug("osi_usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
		  "value 0x%X index 0x%X length 0x%X\n",
		  request, requesttype, value, index, size);
	udev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_msg(udev, pipe, data, size, setup_packet);
	if (err < 0)
		return err;
	if (timeout == 0)
		return (int)size;

	return size;
	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	while (timeout--) {
		if (!((volatile unsigned long)udev->status & USB_ST_NOT_PROC))
			break;
		mdelay(1);
	}
	if (udev->status)
		return -1;

	return udev->act_len;

}

/*
 * submits an Interrupt Message
 */
int osi_usb_submit_int_msg(struct osi_usb_device *dev, unsigned long pipe,
		void *buffer, int transfer_len, int interval)
{
	struct usb_device *udev = dev->context;
	
    return submit_int_msg(udev, pipe, buffer, transfer_len, interval);
}
