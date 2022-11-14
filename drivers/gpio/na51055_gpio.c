/*
 *  gpio/na51000_gpio.c
 *
 *  Author:	Howard Chang
 *  Created:	July 3, 2018
 *  Copyright:	Novatek Inc.
 *
 */
#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/gpio.h>

#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>

struct nvt_gpio_bank {
	phys_addr_t base;
};

#define GPIO_GETREG(ofs)	INW(IOADDR_GPIO_REG_BASE+(ofs))
#define GPIO_SETREG(ofs, value)	OUTW(IOADDR_GPIO_REG_BASE +(ofs),(value))

#define NVT_GPIO_STG_DATA_0		(0)
#define NVT_GPIO_STG_DIR_0		(0x20)
#define NVT_GPIO_STG_SET_0		(0x40)
#define NVT_GPIO_STG_CLR_0		(0x60)

static int gpio_validation(unsigned pin)
{
	if ((pin < C_GPIO(0)) || (pin > C_GPIO(22) && pin < P_GPIO(0)) || \
	(pin > P_GPIO(25) && pin < S_GPIO(0)) || \
	(pin > S_GPIO(12) && pin < L_GPIO(0)) || \
	(pin > L_GPIO(24) && pin < D_GPIO(0)) || \
	(pin > D_GPIO(10) && pin < H_GPIO(0)) || \
	(pin > H_GPIO(11) && pin < A_GPIO(0)) || (pin > A_GPIO(2))) {
		debug("The gpio number is out of range\n");
		return E_NOSPT;
	} else
		return 0;
}

static int nvt_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	u32 reg_data;
	u32 ofs = (offset >> 5) << 2;

	if (gpio_validation(offset) < 0)
		return E_NOSPT;

	offset &= (32-1);

	reg_data = GPIO_GETREG(NVT_GPIO_STG_DIR_0 + ofs);

	reg_data &= ~(1<<offset);   /*input*/

	GPIO_SETREG(NVT_GPIO_STG_DIR_0 + ofs, reg_data);

	return 0;
}

static int nvt_gpio_direction_output(struct udevice *dev, unsigned offset, int value)
{
	u32 reg_data, tmp;
	u32 ofs = (offset >> 5) << 2;

	if (gpio_validation(offset) < 0)
		return E_NOSPT;

	offset &= (32-1);

	tmp = (1<<offset);

	reg_data = GPIO_GETREG(NVT_GPIO_STG_DIR_0 + ofs);

	reg_data |= (1<<offset);    /*output*/

	GPIO_SETREG(NVT_GPIO_STG_DIR_0 + ofs, reg_data);

	if (value)
		GPIO_SETREG(NVT_GPIO_STG_SET_0 + ofs, tmp);
	else
		GPIO_SETREG(NVT_GPIO_STG_CLR_0 + ofs, tmp);

	return 0;
}

static int nvt_gpio_get_value(struct udevice *dev, unsigned offset)
{
	u32 tmp;
	u32 ofs = (offset >> 5) << 2;

	if (gpio_validation(offset) < 0)
		return E_NOSPT;

	offset &= (32-1);

	tmp = (1<<offset);

	return (GPIO_GETREG(NVT_GPIO_STG_DATA_0 + ofs) & tmp) != 0;
}

static int nvt_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
	u32 tmp;
	u32 ofs = (offset >> 5) << 2;

	if (gpio_validation(offset) < 0)
		return E_NOSPT;

	offset &= (32-1);

	tmp = (1<<offset);

	if (value)
		GPIO_SETREG(NVT_GPIO_STG_SET_0 + ofs, tmp);
	else
		GPIO_SETREG(NVT_GPIO_STG_CLR_0 + ofs, tmp);

	return 0;
}

static int nvt_gpio_get_function(struct udevice *dev, unsigned offset)
{
	u32 reg_data;
	u32 ofs = (offset >> 5) << 2;

	if (gpio_validation(offset) < 0)
		return E_NOSPT;

	offset &= (32-1);

	reg_data = GPIO_GETREG(NVT_GPIO_STG_DIR_0 + ofs);

	if (reg_data & (1<<offset))
		return GPIOF_OUTPUT;		

	return GPIOF_INPUT;
}

static const struct dm_gpio_ops gpio_nvt_ops = {
	.direction_input	= nvt_gpio_direction_input,
	.direction_output	= nvt_gpio_direction_output,
	.get_value		= nvt_gpio_get_value,
	.set_value		= nvt_gpio_set_value,
	.get_function		= nvt_gpio_get_function,
};

static int nvt_gpio_probe(struct udevice *dev)
{
	return 0;
}

static int nvt_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = 256;
	uc_priv->bank_name = "nvt-gpio";

	return 0;
}

static const struct udevice_id nvt_gpio_ids[] = {
	{ .compatible = "nvt,nvt_gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_msm) = {
	.name	= "gpio_nvt",
	.id	= UCLASS_GPIO,
	.of_match = nvt_gpio_ids,
	.ofdata_to_platdata = nvt_gpio_ofdata_to_platdata,
	.probe	= nvt_gpio_probe,
	.ops	= &gpio_nvt_ops,
	.priv_auto_alloc_size = sizeof(struct nvt_gpio_bank),
};
