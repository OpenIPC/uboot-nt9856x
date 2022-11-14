/*
 * Driver for the Novatek pinmux
 *
 * Copyright (c) 2019, NOVATEK MICROELECTRONIC CORPORATION.  All rights reserved.
 *
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>
#include <linux/libfdt.h>
#include <asm/arch/na51055_pinmux.h>

#include <asm/io.h>
#include "common.h"
#include <errno.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <command.h>
#include <asm/gpio.h>
#include <malloc.h>
#include <asm/arch/hardware.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/top.h>

#ifdef CONFIG_TARGET_NA51055
#include <asm/arch/na51055_regs.h>
#else
#ifdef CONFIG_TARGET_NA51089
#include <asm/arch/na51089_regs.h>
#else
#if (defined(CONFIG_TARGET_NA51090) || defined(CONFIG_TARGET_NA51090_A64))
#include <asm/arch/na51090_regs.h>
#else
#include <asm/arch/na51000_regs.h>
#endif
#endif
#endif



u32 top_reg_addr = 0xf0010000;
extern u32 top_reg_addr;

#define TOP_DTS_MAX 21

const char *pinmux_dts_name[TOP_DTS_MAX] =
{
	"sdio",
	"sdio2",
	"sdio3",
	"nand",
	"sensor",
	"sensor2",
	"mipi_lvds",
	"i2c",
	"sif",
	"uart",
	"spi",
	"sdp",
	"remote",
	"pwm",
	"pwm2",
	"ccnt",
	"audio",
	"lcd",
	"tv",
	"eth",
	"misc"
};

static PINMUX_FUNC_ID id_restore = 0x0;
static u32 pinmux_restore = 0x0;

static void nvt_pinmux_show_conflict(struct nvt_pinctrl_info *info)
{
	int i;

	pinmux_parsing(info);
	for (i = 0; i < PIN_FUNC_MAX; i++) {
        printf("pinmux %-2d config 0x%x\n", i, info->top_pinmux[i].config);
	}

	panic("###### Conflicted Pinmux Setting ######\n");
}


int nvt_pinmux_capture(PIN_GROUP_CONFIG *pinmux_config, int count)
{
	struct nvt_pinctrl_info *info;
	int i, j, ret = E_OK;
/*
	if (in_interrupt() || in_atomic() || irqs_disabled())
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_ATOMIC);
	else
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	if (!info) {
		printf("nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}
	*/

	info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	if (!info) {
		printf("nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}

	if (top_reg_addr) {
		info->top_base = (void*) top_reg_addr;
		pinmux_parsing(info);

		for (j = 0; j < count; j++) {
			for (i = 0; i < PIN_FUNC_MAX; i++) {
				if (i == pinmux_config[j].pin_function)
					pinmux_config[j].config = info->top_pinmux[i].config;
			}
		}
	} else {
		printf("invalid pinmux address\n");
		ret = -ENOMEM;
	}

	kfree(info);

	return ret;
}
EXPORT_SYMBOL(nvt_pinmux_capture);

int nvt_pinmux_update(PIN_GROUP_CONFIG *pinmux_config, int count)
{
	struct nvt_pinctrl_info *info;
	int i, j, ret = E_OK;
/*
	if (in_interrupt() || in_atomic() || irqs_disabled())
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_ATOMIC);
	else
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	if (!info) {
		printf("nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}
*/
	info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	if (!info) {
		printf("nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}

	if (top_reg_addr) {
		info->top_base = (void*) top_reg_addr;
		pinmux_parsing(info);

		for (j = 0; j < count; j++) {
			for (i = 0; i < PIN_FUNC_MAX; i++) {
				if (i == pinmux_config[j].pin_function)
					info->top_pinmux[i].config = pinmux_config[j].config;
			}
		}

		ret = pinmux_init(info);
		if (ret == E_OK)
			ret = pinmux_set_config(id_restore, pinmux_restore);
	} else {
		printf("invalid pinmux address\n");
		ret = -ENOMEM;
	}

	kfree(info);

	return ret;
}
EXPORT_SYMBOL(nvt_pinmux_update);


int pinmux_set_config(PINMUX_FUNC_ID id, u32 pinmux)
{
	struct nvt_pinctrl_info *info;
	int ret;
/*
	if (in_interrupt() || in_atomic() || irqs_disabled())
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_ATOMIC);
	else
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	if (!info) {
		printf("nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}
*/
    info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);

	if (top_reg_addr) {
		info->top_base = (void*) top_reg_addr;
		ret = pinmux_set_host(info, id, pinmux);

		if (id <= PINMUX_FUNC_ID_LCD2) {
			id_restore = id;
			pinmux_restore = pinmux;
		}
	} else {
		printf("invalid pinmux address\n");
		ret = -ENOMEM;
	}

	kfree(info);

	return ret;
}
EXPORT_SYMBOL(pinmux_set_config);

int nvt_pinmux_probe(void)
{
    struct nvt_pinctrl_info *info ;

	//struct nvt_gpio_info *gpio;
	//struct resource *nvt_mem_base;
	//struct device_node *top_np;
	//u32 value;
	//u32 pad_config[4] = {0, 0, 0, 0};
	//u32 gpio_config[2] = {0, 0};
	//int nr_pinmux = 0, nr_pad = 0, nr_gpio = 0 ;
    int i = 0, nr_pinmux = 0;

	/* Enable it after dts parsing ready*/
    ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
    int nodeoffset;
    u32 *cell = NULL;
    char path[20] = {0};

    info = malloc(sizeof(struct nvt_pinctrl_info));
	//info = devm_kzalloc(&pdev->dev, sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	if (!info) {
		dev_err(pdev, "nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}
#if 0
	for_each_child_of_node(pdev->dev.of_node,  top_np) {
		if (!of_get_property(top_np, "gpio_config", NULL))
			continue;

		nr_gpio++;
	}

	gpio = devm_kzalloc(&pdev->dev, nr_gpio * sizeof(struct nvt_gpio_info), GFP_KERNEL);
	if (!gpio) {
		dev_err(&pdev->dev, "nvt gpio mem alloc fail\n");
		return -ENOMEM;
	}

	nr_gpio = 0;
#endif

/*
	nvt_mem_base = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	info->top_base = devm_ioremap_resource(&pdev->dev, nvt_mem_base);
	if (IS_ERR(info->top_base)) {
		dev_err(&pdev->dev, "fail to get pinmux mem base\n");
		return -ENOMEM;
	}
	*/
    info->top_base = (void*) IOADDR_TOP_REG_BASE;
	top_reg_addr = (u32) info->top_base;
    printf("%s(pinmux= top_base=%p, length=%x):\n", __func__ , info->top_base,
	      IOADDR_TOP_REG_BASE);
#if 0
	nvt_mem_base = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	info->pad_base = devm_ioremap_resource(&pdev->dev, nvt_mem_base);
	if (IS_ERR(info->pad_base)) {
		dev_err(&pdev->dev, "fail to get pad mem base\n");
		return -ENOMEM;
	}

	nvt_mem_base = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	info->gpio_base = devm_ioremap_resource(&pdev->dev, nvt_mem_base);
	if (IS_ERR(info->gpio_base)) {
		dev_err(&pdev->dev, "fail to get gpio mem base\n");
		return -ENOMEM;
	}
#endif


    for(i=0;i < PIN_FUNC_MAX; i++)
    {
        sprintf(path,"/top@%x/%s",IOADDR_TOP_REG_BASE, pinmux_dts_name[i]);
    	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
    	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "pinmux", NULL);
        info->top_pinmux[nr_pinmux].config = __be32_to_cpu(cell[0]);
        info->top_pinmux[nr_pinmux].pin_function = nr_pinmux;

//printf("%s(top = top_base=%s, length=%x):\n", __func__ , pinmux_dts_name[i],
	      //info->top_pinmux[nr_pinmux].config);

        nr_pinmux++;



#if 0
	for_each_child_of_node(pdev->dev.of_node, top_np) {
		if (!of_property_read_u32(top_np, "pinmux", &value)) {
			info->top_pinmux[nr_pinmux].pin_function = nr_pinmux;
			info->top_pinmux[nr_pinmux].config = value;
			nr_pinmux++;
		}


		if (!of_property_read_u32_array(top_np, "pad_config", pad_config, 4)) {
			info->pad[nr_pad].pad_ds_pin = pad_config[0];
			info->pad[nr_pad].driving = pad_config[1];
			info->pad[nr_pad].pad_gpio_pin = pad_config[2];
			info->pad[nr_pad].direction = pad_config[3];
			nr_pad++;
		}

		if (!of_property_read_u32_array(top_np, "gpio_config", gpio_config, 2)) {
			gpio[nr_gpio].gpio_pin = gpio_config[0];
			gpio[nr_gpio].direction = gpio_config[1];
			nr_gpio++;
		}
#endif
	}

	if (nr_pinmux == 0)
		return -ENOMEM;

	pinmux_preset(info);

	//pad_preset(info);

	if (pinmux_init(info))
		nvt_pinmux_show_conflict(info);
#if 0
	if (nr_pad)
		pad_init(info, nr_pad);

	if (nr_gpio)
		gpio_init(gpio, nr_gpio, info);
#endif

	//if(nvt_pinmux_proc_init())
		//return -ENOMEM;

	return 0;
}
EXPORT_SYMBOL(nvt_pinmux_probe);

static int nvt_pinctrl_remove(struct udevice *pdev)
{

	return 0;
}
/*
static const struct udevice_id nvt_pinctrl_ids[] = {
	{ .compatible = "nvt,nvt_top", },
	{ },
};

U_BOOT_DRIVER(nvt_pinmux_drv) = {
	.name = "nvt_pinctrl",
    .id		= UCLASS_PINCTRL,
	.of_match = nvt_pinctrl_ids,
	.probe = nvt_pinmux_probe,
	.remove = nvt_pinctrl_remove,
	.priv_auto_alloc_size   = sizeof(struct nvt_pinctrl_info),
};

*/
