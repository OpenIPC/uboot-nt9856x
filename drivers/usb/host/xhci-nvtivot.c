/*
 * Copyright (c) 2015, Novatek Microelectronics Corp.
 * All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <usb.h>

#include "xhci.h"
#ifdef CONFIG_DM_USB
#include <dm.h>
#include <fdtdec.h>
#endif
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>

#ifdef CONFIG_DM_USB
/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

struct uboot_xhci_priv {
        struct xhci_ctrl ctrl; /* Needed by EHCI */
        struct xhci_hccr *hccr;
        struct xhci_hcor *hcor;
};
#endif

#ifndef CONFIG_DM_USB
/*
 * Create the appropriate control structures to manage a new XHCI host
 * controller.
 */
int xhci_hcd_init(int index, struct xhci_hccr **ret_hccr,
		  struct xhci_hcor **ret_hcor)
{
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;
	int len;
	unsigned long tmpval;

	/* init OTG controller */
	/* Enable phy clk*/
	tmpval = readl((volatile unsigned long *)IOADDR_CG_REG_BASE);
	tmpval |= (0x1<<20);
	writel(tmpval, (volatile unsigned long *)IOADDR_CG_REG_BASE);

	/* Enable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	tmpval &= ~(0x1<<18);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

	udelay(10);

	/* Disable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	tmpval |= 0x1<<18;
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

	/* Release sram shutdown*/
	tmpval = readl((volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));
	tmpval &= ~(0x1<<7);
	writel(tmpval, (volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));

	/* Enable clock*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));
	tmpval |= (0x1<<26);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));

	mdelay(10);

	tmpval = readl((volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x28));
	tmpval = 0x1;
	writel(tmpval, (volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x28));

	tmpval = readl((volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x10));
	tmpval &= ~0x3;
	if (1)
		tmpval |= (0x1 << 3);
	else
		tmpval &= ~(0x1 << 11);
	tmpval |= ((0x1 << 5) | (0x1 << 0));
	writel(tmpval, (volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x10));

	hccr = (struct xhci_hccr *)IOADDR_USB3_REG_BASE;
	len = HC_LENGTH(xhci_readl(&hccr->cr_capbase));
	hcor = (struct xhci_hcor *)((unsigned long)hccr + len);

	printk("XHCI init hccr 0x%lx and hcor 0x%lx hc_length 0x%x\n",
		(unsigned long)hccr, (unsigned long)hcor, len);

	*ret_hccr = hccr;
	*ret_hcor = hcor;

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding * to the XHCI host
 * controller
 */
void xhci_hcd_stop(int index)
{
}


#else
/*
 * Create the appropriate control structures to manage a new XHCI host
 * controller.
 */
int dm_xhci_hcd_init(int index, struct xhci_hccr **ret_hccr,
		struct xhci_hcor **ret_hcor)
{
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;
	int len;
	unsigned long tmpval = 0;

	/* init OTG controller */
	/* Enable phy clk*/
	tmpval = readl((volatile unsigned long *)IOADDR_CG_REG_BASE);
	tmpval |= (0x1<<20);
	writel(tmpval, (volatile unsigned long *)IOADDR_CG_REG_BASE);

	/* Enable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	tmpval &= ~(0x1<<18);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

	udelay(10);

	/* Disable Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));
	tmpval |= 0x1<<18;
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x98));

	/* Release sram shutdown*/
	tmpval = readl((volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));
	tmpval &= ~(0x1<<25);
	writel(tmpval, (volatile unsigned long *)(IOADDR_TOP_REG_BASE+0x1000));

	/* Enable clock*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));
	tmpval |= (0x1<<26);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+0x74));

	mdelay(10);

	tmpval = readl((volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x10));
	tmpval &= ~0x3;
	tmpval &= ~(0x1 << 11);
	tmpval |= ((0x1 << 5) | (0x1 << 0));
	writel(tmpval, (volatile unsigned long *)(IOADDR_USB3CTRL_REG_BASE+0x10));

	hccr = (struct xhci_hccr *)IOADDR_USB3_REG_BASE;
	len = HC_LENGTH(xhci_readl(&hccr->cr_capbase));
	hcor = (struct xhci_hcor *)((unsigned long)hccr + len);

	printk("XHCI init hccr 0x%lx and hcor 0x%lx hc_length 0x%x\n",
		(unsigned long)hccr, (unsigned long)hcor, len);

	*ret_hccr = hccr;
	*ret_hcor = hcor;

	return 0;
}

static int xhci_usb_probe(struct udevice *dev)
{
	struct uboot_xhci_priv *ctx = dev_get_priv(dev);
	int ret = 0;

	ret = dm_xhci_hcd_init(0 , &ctx->hccr, &ctx->hcor);
	if (ret) {
		puts("XHCI: failed to initialize controller\n");
		return -EINVAL;
	}

	debug("%s hccr 0x%lx and hcor 0x%lx hc_length %ld\n", __func__,
			(unsigned long)ctx->hccr, (unsigned long)ctx->hcor,
			(unsigned long)HC_LENGTH(xhci_readl(&ctx->hccr->cr_capbase)));

	return xhci_register(dev, ctx->hccr, ctx->hcor);
}

static int xhci_usb_remove(struct udevice *dev)
{
	int ret;

	ret = xhci_deregister(dev);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id xhci_usb_ids[] = {
	{ .compatible = "nvt,nvt_usb3xhci" },
	{ }
};

U_BOOT_DRIVER(usb_xhci) = {
	.name   = "xhci_nvtivot",
	.id     = UCLASS_USB,
	.of_match = xhci_usb_ids,
	.probe = xhci_usb_probe,
	.remove = xhci_usb_remove,
	.ops    = &xhci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct uboot_xhci_priv),
	.flags  = DM_FLAG_ALLOC_PRIV_DMA,
};
#endif