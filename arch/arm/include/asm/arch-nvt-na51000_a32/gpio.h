/*
 * Copyright Novatek Inc
 *
 * SPDX-License-Identifier:      GPL-2.0+
 */

#ifndef __ARCH_GPIO_H
#define __ARCH_GPIO_H

#define C_GPIO(pin)	(pin)
#define P_GPIO(pin)	(pin + 0x40)
#define S_GPIO(pin)	(pin + 0x80)
#define L_GPIO(pin)	(pin + 0xA0)
#define H_GPIO(pin)	(pin + 0xC0)
#define D_GPIO(pin)	(pin + 0xE0)

/*
 * Empty file - cmd_gpio.c requires this. The implementation
 * is in drivers/gpio/na51000_gpio.c instead of inlined here.
 */

#endif /* __ARCH_GPIO_H */
