/*
 * Novatek PWM driver on NVT_FT platform.
 *
 * Copyright (C) 2020 Novatek MicroElectronics Corp.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <div64.h>
#include <pwm.h>
#include <asm/io.h>
#ifndef CONFIG_TARGET_NA51000
#include <asm/nvt-common/nvt_common.h>
#include <linux/libfdt.h>
#endif

#ifdef CONFIG_TARGET_NA51000
#define NUM_PWM                 20
#else
#define NUM_PWM                 12

#ifdef CONFIG_TARGET_NA51090_A64
#define PIN_PWM_CFG_PWM0      0x0001          ///< Enable PWM0 @P_GPIO30[PWM0]
#define PIN_PWM_CFG_PWM1      0x0010          ///< Enable PWM1 @P_GPIO31[PWM1]
#define PIN_PWM_CFG_PWM2      0x0100          ///< Enable PWM2 @P_GPIO32[PWM2]
#define PIN_PWM_CFG_PWM3      0x1000          ///< Enable PWM3 @D_GPIO0 [PWM3]
#endif

#define PIN_PWM_CFG_PWM0_1      0x01          ///< Enable PWM0 @P_GPIO0[PWM0_1]
#define PIN_PWM_CFG_PWM0_2      0x02          ///< Enable PWM0 @MC13[PWM0_2]
#define PIN_PWM_CFG_PWM0_3      0x04          ///< Enable PWM0 @S_GPIO1[PWM0_3]
#define PIN_PWM_CFG_PWM0_4      0x08          ///< Enable PWM0 @MC4[PWM0_4]
#define PIN_PWM_CFG_PWM0_5      0x10          ///< Enable PWM0 @DSI_GPIO0[PWM0_5]
#define PIN_PWM_CFG_PWM1_1      0x20          ///< Enable PWM1 @P_GPIO1[PWM1_1]
#define PIN_PWM_CFG_PWM1_2      0x40          ///< Enable PWM1 @MC14[PWM1_2]
#define PIN_PWM_CFG_PWM1_3      0x80          ///< Enable PWM1 @S_GPIO2[PWM1_3]
#define PIN_PWM_CFG_PWM1_4      0x100         ///< Enable PWM1 @MC5[PWM1_4]
#define PIN_PWM_CFG_PWM1_5      0x200         ///< Enable PWM1 @DSI_GPIO1[PWM1_5]
#define PIN_PWM_CFG_PWM2_1      0x400         ///< Enable PWM2 @P_GPIO2[PWM2_1]
#define PIN_PWM_CFG_PWM2_2      0x800         ///< Enable PWM2 @MC15[PWM2_2]
#define PIN_PWM_CFG_PWM2_3      0x1000        ///< Enable PWM2 @S_GPIO3[PWM2_3]
#define PIN_PWM_CFG_PWM2_4      0x2000        ///< Enable PWM2 @MC6[PWM2_4]
#define PIN_PWM_CFG_PWM2_5      0x4000        ///< Enable PWM2 @DSI_GPIO2[PWM2_5]
#define PIN_PWM_CFG_PWM3_1      0x8000        ///< Enable PWM3 @P_GPIO3[PWM3_1]
#define PIN_PWM_CFG_PWM3_2      0x10000       ///< Enable PWM3 @MC16[PWM3_2]
#define PIN_PWM_CFG_PWM3_3      0x20000       ///< Enable PWM3 @S_GPIO4[PWM3_3]
#define PIN_PWM_CFG_PWM3_4      0x40000       ///< Enable PWM3 @MC7[PWM3_4]
#define PIN_PWM_CFG_PWM3_5      0x80000       ///< Enable PWM3 @DSI_GPIO3[PWM3_5]
#define PIN_PWM_CFG_PWM8_1      0x100000      ///< Enable PWM8 @P_GPIO8[PWM8_1]
#define PIN_PWM_CFG_PWM8_2      0x200000      ///< Enable PWM8 @MC19[PWM8_2]
#define PIN_PWM_CFG_PWM8_3      0x400000      ///< Enable PWM8 @HSI_GPIO6[PWM8_3]
#define PIN_PWM_CFG_PWM8_4      0x800000      ///< Enable PWM8 @D_GPIO8[PWM8_4]
#define PIN_PWM_CFG_PWM8_5      0x1000000     ///< Enable PWM8 @DSI_GPIO8[PWM8_5]
#define PIN_PWM_CFG_PWM9_1      0x2000000     ///< Enable PWM9 @P_GPIO9[PWM9_1]
#define PIN_PWM_CFG_PWM9_2      0x4000000     ///< Enable PWM9 @MC20[PWM9_2]
#define PIN_PWM_CFG_PWM9_3      0x8000000     ///< Enable PWM9 @HSI_GPIO7[PWM9_3]
#define PIN_PWM_CFG_PWM9_4      0x10000000    ///< Enable PWM9 @D_GPIO9[PWM9_4]
#define PIN_PWM_CFG_PWM9_5      0x20000000    ///< Enable PWM9 @DSI_GPIO9[PWM9_5]

#define PIN_PWM_CFG2_PWM4_1     0x01          ///< Enable PWM4 @P_GPIO4[PWM4_1]
#define PIN_PWM_CFG2_PWM4_2     0x02          ///< Enable PWM4 @MC11[PWM4_2]
#define PIN_PWM_CFG2_PWM4_3     0x04          ///< Enable PWM4 @S_GPIO5[PWM4_3]
#define PIN_PWM_CFG2_PWM4_4     0x08          ///< Enable PWM4 @D_GPIO3[PWM4_4]
#define PIN_PWM_CFG2_PWM4_5     0x10          ///< Enable PWM4 @DSI_GPIO4[PWM4_5]
#define PIN_PWM_CFG2_PWM5_1     0x20          ///< Enable PWM5 @P_GPIO5[PWM5_1]
#define PIN_PWM_CFG2_PWM5_2     0x40          ///< Enable PWM5 @MC12[PWM5_2]
#define PIN_PWM_CFG2_PWM5_3     0x80          ///< Enable PWM5 @S_GPIO6[PWM5_3]
#define PIN_PWM_CFG2_PWM5_4     0x100         ///< Enable PWM5 @D_GPIO4[PWM5_4]
#define PIN_PWM_CFG2_PWM5_5     0x200         ///< Enable PWM5 @DSI_GPIO5[PWM5_5]
#define PIN_PWM_CFG2_PWM6_1     0x400         ///< Enable PWM6 @P_GPIO6[PWM6_1]
#define PIN_PWM_CFG2_PWM6_2     0x800         ///< Enable PWM6 @MC17[PWM6_2]
#define PIN_PWM_CFG2_PWM6_3     0x1000        ///< Enable PWM6 @S_GPIO7[PWM6_3]
#define PIN_PWM_CFG2_PWM6_4     0x2000        ///< Enable PWM6 @D_GPIO5[PWM6_4]
#define PIN_PWM_CFG2_PWM6_5     0x4000        ///< Enable PWM6 @DSI_GPIO6[PWM6_5]
#define PIN_PWM_CFG2_PWM7_1     0x8000        ///< Enable PWM7 @P_GPIO7[PWM7_1]
#define PIN_PWM_CFG2_PWM7_2     0x10000       ///< Enable PWM7 @MC18[PWM7_2]
#define PIN_PWM_CFG2_PWM7_3     0x20000       ///< Enable PWM7 @S_GPIO8[PWM7_3]
#define PIN_PWM_CFG2_PWM7_4     0x40000       ///< Enable PWM7 @D_GPIO6[PWM7_4]
#define PIN_PWM_CFG2_PWM7_5     0x80000       ///< Enable PWM7 @DSI_GPIO7[PWM7_5]
#define PIN_PWM_CFG2_PWM10_1    0x100000      ///< Enable PWM10 @P_GPIO10[PWM10_1]
#define PIN_PWM_CFG2_PWM10_2    0x200000      ///< Enable PWM10 @MC21[PWM10_2]
#define PIN_PWM_CFG2_PWM10_3    0x400000      ///< Enable PWM10 @HSI_GPIO8[PWM10_3]
#define PIN_PWM_CFG2_PWM10_4    0x800000      ///< Enable PWM10 @D_GPIO10[PWM10_4]
#define PIN_PWM_CFG2_PWM10_5    0x1000000     ///< Enable PWM10 @DSI_GPIO10[PWM10_5]
#define PIN_PWM_CFG2_PWM11_1    0x2000000     ///< Enable PWM11 @P_GPIO11[PWM11_1]
#define PIN_PWM_CFG2_PWM11_2    0x4000000     ///< Enable PWM11 @MC22[PWM11_2]
#define PIN_PWM_CFG2_PWM11_3    0x8000000     ///< Enable PWM11 @HSI_GPIO9[PWM11_3]
#define PIN_PWM_CFG2_PWM11_4    0x10000000    ///< Enable PWM11 @D_GPIO7[PWM11_4]
#define PIN_PWM_CFG2_PWM11_5    0x20000000    ///< Enable PWM11 @L_GPIO0[PWM11_5]
#endif

#define SRC_FREQ             120000000
#define PCLK_FREQ            250000

#define CONFIG_TOP_BASE      0xF0010000
#define CONIFG_CG_BASE       0xF0020000

#ifdef CONFIG_TARGET_NA51090_A64
#define CONFIG_PWM_BASE      0x2F0120000
#else
#define CONFIG_PWM_BASE      0xF0210000
#endif

/* PWM Control Register */
#define PWM_CTRL_REG(id)     ((0x08 * id) + 0x00)
#define PWM_CYCLE_MASK       GENMASK(15, 0)
#define PWM_TYPE_BIT         BIT(16)

/* PWM Period Register */
#define PWM_PRD_REG(id)      ((0x08 * id) + 0x04)
#define PWM_RISE_MASK        GENMASK(7, 0)
#define PWM_FALL_MASK        GENMASK(15, 8)
#define PWM_PERIOD_MASK      GENMASK(23, 16)
#define PWM_INV_BIT          BIT(28)

/* PWM Expand Period Register */
#define PWM_PRD_HI_REG(id)   ((0x04 * id) + 0x230)
#define PWM_RISE_HI_MASK     GENMASK(7, 0)
#define PWM_FALL_HI_MASK     GENMASK(15, 8)
#define PWM_PERIOD_HI_MASK   GENMASK(23, 16)

/* PWM Enable Register */
#define PWM_EN_REG           0x100

/* PWM Disable Register */
#define PWM_DIS_REG          0x104

u32 pclk_freq = PCLK_FREQ;
u32 pwm_pinmux = 0;
u32 pwm2_pinmux = 0;

static inline void nvt_pwm_write_reg(int reg, u32 val)
{
	writel(val, CONFIG_PWM_BASE + reg);
}

static inline u32 nvt_pwm_read_reg(int reg)
{
	return readl(CONFIG_PWM_BASE + reg);
}

void nvt_pwm_set_freq(int pwm_id, u32 freq)
{
	u32 div;

	if (!freq || (freq > SRC_FREQ)) {
		return;
	}

	div = SRC_FREQ / freq;
	div = div - 1;

	pclk_freq = freq;

	printf("pclk_freq(%u) div(%u)\n", pclk_freq, div);

#ifdef CONFIG_TARGET_NA51000
	if (pwm_id < 4) {
		*(u32 *)(CONIFG_CG_BASE + 0x50) &= ~(0x3FFF);
		*(u32 *)(CONIFG_CG_BASE + 0x50) |= (div & 0x3FFF);
	} else if (pwm_id < 8) {
		*(u32 *)(CONIFG_CG_BASE + 0x50) &= ~(0x3FFF << 16);
		*(u32 *)(CONIFG_CG_BASE + 0x50) |= ((div << 16) & (0x3FFF << 16));
	} else if (pwm_id < 12) {
		*(u32 *)(CONIFG_CG_BASE + 0x54) &= ~(0x3FFF);
		*(u32 *)(CONIFG_CG_BASE + 0x54) |= (div & 0x3FFF);
	} else if (pwm_id == 12) {
		*(u32 *)(CONIFG_CG_BASE + 0x54) &= ~(0x3FFF << 16);
		*(u32 *)(CONIFG_CG_BASE + 0x54) |= ((div << 16) & (0x3FFF << 16));
	} else if (pwm_id == 13) {
		*(u32 *)(CONIFG_CG_BASE + 0x58) &= ~(0x3FFF);
		*(u32 *)(CONIFG_CG_BASE + 0x58) |= (0x5 & 0x3FFF);
	} else if (pwm_id == 14) {
		*(u32 *)(CONIFG_CG_BASE + 0x58) &= ~(0x3FFF << 16);
		*(u32 *)(CONIFG_CG_BASE + 0x58) |= ((div << 16) & (0x3FFF << 16));
	} else if (pwm_id == 15) {
		*(u32 *)(CONIFG_CG_BASE + 0x5C) &= ~(0x3FFF);
		*(u32 *)(CONIFG_CG_BASE + 0x5C) |= (div & 0x3FFF);
	} else if (pwm_id == 16) {
		*(u32 *)(CONIFG_CG_BASE + 0x5C) &= ~(0x3FFF << 16);
		*(u32 *)(CONIFG_CG_BASE + 0x5C) |= ((div << 16) & (0x3FFF << 16));
	} else if (pwm_id == 17) {
		*(u32 *)(CONIFG_CG_BASE + 0x60) &= ~(0x3FFF);
		*(u32 *)(CONIFG_CG_BASE + 0x60) |= (div & 0x3FFF);
	} else if (pwm_id == 18) {
		*(u32 *)(CONIFG_CG_BASE + 0x60) &= ~(0x3FFF << 16);
		*(u32 *)(CONIFG_CG_BASE + 0x60) |= ((div << 16) & (0x3FFF << 16));
	} else if (pwm_id == 19) {
		*(u32 *)(CONIFG_CG_BASE + 0x64) &= ~(0x3FFF);
		*(u32 *)(CONIFG_CG_BASE + 0x64) |= (div & 0x3FFF);
	}
#elif defined CONFIG_TARGET_NA51090_A64
	if (pwm_id == 0) {
		*(u32 *)(CONIFG_CG_BASE + 0x34) &= ~(0x3FFF);
		*(u32 *)(CONIFG_CG_BASE + 0x34) |= (div & 0x3FFF);
	} else if (pwm_id == 1) {
		*(u32 *)(CONIFG_CG_BASE + 0x34) &= ~(0x3FFF << 16);
		*(u32 *)(CONIFG_CG_BASE + 0x34) |= ((div & 0x3FFF) << 16);
	} else if (pwm_id == 2) {
		*(u32 *)(CONIFG_CG_BASE + 0x38) &= ~(0x3FFF);
		*(u32 *)(CONIFG_CG_BASE + 0x38) |= (div & 0x3FFF);
	} else if (pwm_id == 3) {
		*(u32 *)(CONIFG_CG_BASE + 0x38) &= ~(0x3FFF << 16);
		*(u32 *)(CONIFG_CG_BASE + 0x38) |= ((div & 0x3FFF) << 16);
	}
#else
	if (pwm_id < 4) {
		*(u32 *)(CONIFG_CG_BASE + 0x50) &= ~(0x3FFF);
		*(u32 *)(CONIFG_CG_BASE + 0x50) |= (div & 0x3FFF);
	} else if (pwm_id < 8) {
		*(u32 *)(CONIFG_CG_BASE + 0x50) &= ~(0x3FFF << 16);
		*(u32 *)(CONIFG_CG_BASE + 0x50) |= ((div << 16) & (0x3FFF << 16));
	} else if (pwm_id == 8) {
		*(u32 *)(CONIFG_CG_BASE + 0x54) &= ~(0x3FFF);
		*(u32 *)(CONIFG_CG_BASE + 0x54) |= (div & 0x3FFF);
	} else if (pwm_id == 9) {
		*(u32 *)(CONIFG_CG_BASE + 0x54) &= ~(0x3FFF << 16);
		*(u32 *)(CONIFG_CG_BASE + 0x54) |= ((div << 16) & (0x3FFF << 16));
	} else if (pwm_id == 10) {
		*(u32 *)(CONIFG_CG_BASE + 0x58) &= ~(0x3FFF);
		*(u32 *)(CONIFG_CG_BASE + 0x58) |= (div & 0x3FFF);
	} else if (pwm_id == 11) {
		*(u32 *)(CONIFG_CG_BASE + 0x58) &= ~(0x3FFF << 16);
		*(u32 *)(CONIFG_CG_BASE + 0x58) |= ((div << 16) & (0x3FFF << 16));
	}
#endif
}

#ifndef CONFIG_TARGET_NA51000
void nvt_pwm_set_pinmux(int pwm_id, u32 pwm_pinmux)
{
	printf("pwm_id(%d) pwm_pinmux(0x%x)\n", pwm_id, pwm_pinmux);

#ifdef CONFIG_TARGET_NA51090_A64
	if (pwm_id == 0) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM0) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x1 << (pwm_id * 2));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x1 << (pwm_id * 2));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << 30);
		}
	} else if (pwm_id == 1) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM1) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x1 << (pwm_id * 2));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x1 << (pwm_id * 2));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << 31);
		}
	} else if (pwm_id == 2) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM2) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x1 << (pwm_id * 2));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x1 << (pwm_id * 2));
			*(u32 *)(CONFIG_TOP_BASE + 0xAC) &= ~(0x1 << 0);
		}
	} else if (pwm_id == 3) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM3) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x1 << (pwm_id * 2));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x1 << (pwm_id * 2));
			*(u32 *)(CONFIG_TOP_BASE + 0xB4) &= ~(0x1 << 0);
		}
	}
#else
	if (pwm_id == 0) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM0_1) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_2) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 13);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_3) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xB0) &= ~(0x1 << 1);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_4) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 4);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM0_5) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
#ifdef CONFIG_TARGET_NA51055
			*(u32 *)(CONFIG_TOP_BASE + 0xB8) &= ~(0x1 << 0);
#else
			*(u32 *)(CONFIG_TOP_BASE + 0xE8) &= ~(0x1 << 0);
#endif
		} else {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
		}
	} else if (pwm_id == 1) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM1_1) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_2) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 14);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_3) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xB0) &= ~(0x1 << 2);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_4) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 5);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM1_5) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
#ifdef CONFIG_TARGET_NA51055
			*(u32 *)(CONFIG_TOP_BASE + 0xB8) &= ~(0x1 << 1);
#else
			*(u32 *)(CONFIG_TOP_BASE + 0xE8) &= ~(0x1 << 1);
#endif
		} else {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
		}
	} else if (pwm_id == 2) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM2_1) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_2) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 15);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_3) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xB0) &= ~(0x1 << 3);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_4) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 6);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM2_5) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
#ifdef CONFIG_TARGET_NA51055
			*(u32 *)(CONFIG_TOP_BASE + 0xB8) &= ~(0x1 << 2);
#else
			*(u32 *)(CONFIG_TOP_BASE + 0xE8) &= ~(0x1 << 2);
#endif
		} else {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
		}
	} else if (pwm_id == 3) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM3_1) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_2) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 16);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_3) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xB0) &= ~(0x1 << 4);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_4) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 7);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM3_5) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
#ifdef CONFIG_TARGET_NA51055
			*(u32 *)(CONFIG_TOP_BASE + 0xB8) &= ~(0x1 << 3);
#else
			*(u32 *)(CONFIG_TOP_BASE + 0xE8) &= ~(0x1 << 3);
#endif
		} else {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
		}
	} else if (pwm_id == 4) {
		if (pwm_pinmux & PIN_PWM_CFG2_PWM4_1) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM4_2) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 11);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM4_3) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xB0) &= ~(0x1 << 5);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM4_4) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xD0) &= ~(0x1 << 3);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM4_5) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
#ifdef CONFIG_TARGET_NA51055
			*(u32 *)(CONFIG_TOP_BASE + 0xB8) &= ~(0x1 << 4);
#else
			*(u32 *)(CONFIG_TOP_BASE + 0xE8) &= ~(0x1 << 4);
#endif
		} else {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
		}
	} else if (pwm_id == 5) {
		if (pwm_pinmux & PIN_PWM_CFG2_PWM5_1) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM5_2) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 12);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM5_3) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xB0) &= ~(0x1 << 6);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM5_4) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xD0) &= ~(0x1 << 4);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM5_5) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
#ifdef CONFIG_TARGET_NA51055
			*(u32 *)(CONFIG_TOP_BASE + 0xB8) &= ~(0x1 << 5);
#else
			*(u32 *)(CONFIG_TOP_BASE + 0xE8) &= ~(0x1 << 5);
#endif
		} else {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
		}
	} else if (pwm_id == 6) {
		if (pwm_pinmux & PIN_PWM_CFG2_PWM6_1) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM6_2) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 17);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM6_3) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xB0) &= ~(0x1 << 7);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM6_4) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xD0) &= ~(0x1 << 5);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM6_5) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
#ifdef CONFIG_TARGET_NA51055
			*(u32 *)(CONFIG_TOP_BASE + 0xB8) &= ~(0x1 << 6);
#else
			*(u32 *)(CONFIG_TOP_BASE + 0xE8) &= ~(0x1 << 6);
#endif
		} else {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
		}
	} else if (pwm_id == 7) {
		if (pwm_pinmux & PIN_PWM_CFG2_PWM7_1) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x1 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM7_2) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x2 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 18);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM7_3) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x3 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xB0) &= ~(0x1 << 8);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM7_4) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x4 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0xD0) &= ~(0x1 << 6);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM7_5) {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x5 << (pwm_id * 3));
#ifdef CONFIG_TARGET_NA51055
			*(u32 *)(CONFIG_TOP_BASE + 0xB8) &= ~(0x1 << 7);
#else
			*(u32 *)(CONFIG_TOP_BASE + 0xE8) &= ~(0x1 << 7);
#endif
		} else {
			*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x7 << (pwm_id * 3));
		}
	} else if (pwm_id == 8) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM8_1) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x1 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM8_2) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x2 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 19);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM8_3) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x3 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xD8) &= ~(0x1 << 6);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM8_4) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x4 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xD0) &= ~(0x1 << 8);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM8_5) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x5 << ((pwm_id - 8) * 3 + 16));
#ifdef CONFIG_TARGET_NA51055
			*(u32 *)(CONFIG_TOP_BASE + 0xB8) &= ~(0x1 << 8);
#else
			*(u32 *)(CONFIG_TOP_BASE + 0xE8) &= ~(0x1 << 8);
#endif
		} else {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
		}
	} else if (pwm_id == 9) {
		if (pwm_pinmux & PIN_PWM_CFG_PWM9_1) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x1 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM9_2) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x2 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 20);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM9_3) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x3 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xD8) &= ~(0x1 << 7);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM9_4) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x4 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xD0) &= ~(0x1 << 9);
		} else if (pwm_pinmux & PIN_PWM_CFG_PWM9_5) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x5 << ((pwm_id - 8) * 3 + 16));
#ifdef CONFIG_TARGET_NA51055
			*(u32 *)(CONFIG_TOP_BASE + 0xB8) &= ~(0x1 << 9);
#else
			*(u32 *)(CONFIG_TOP_BASE + 0xE8) &= ~(0x1 << 9);
#endif
		} else {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
		}
	} else if (pwm_id == 10) {
		if (pwm_pinmux & PIN_PWM_CFG2_PWM10_1) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x1 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM10_2) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x2 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 21);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM10_3) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x3 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xD8) &= ~(0x1 << 8);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM10_4) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x4 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xD0) &= ~(0x1 << 10);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM10_5) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x5 << ((pwm_id - 8) * 3 + 16));
#ifdef CONFIG_TARGET_NA51055
			*(u32 *)(CONFIG_TOP_BASE + 0xB8) &= ~(0x1 << 10);
#else
			*(u32 *)(CONFIG_TOP_BASE + 0xE8) &= ~(0x1 << 10);
#endif
		} else {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
		}
	} else if (pwm_id == 11) {
		if (pwm_pinmux & PIN_PWM_CFG2_PWM11_1) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x1 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << pwm_id);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM11_2) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x2 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xA0) &= ~(0x1 << 22);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM11_3) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x3 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xD8) &= ~(0x1 << 9);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM11_4) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x4 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0xD0) &= ~(0x1 << 7);
		} else if (pwm_pinmux & PIN_PWM_CFG2_PWM11_5) {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
			*(u32 *)(CONFIG_TOP_BASE + 0x18) |= (0x5 << ((pwm_id - 8) * 3 + 16));
#ifdef CONFIG_TARGET_NA51055
			*(u32 *)(CONFIG_TOP_BASE + 0xB8) &= ~(0x1 << 11);
#else
			*(u32 *)(CONFIG_TOP_BASE + 0xB8) &= ~(0x1 << 0);
#endif
		} else {
			*(u32 *)(CONFIG_TOP_BASE + 0x18) &= ~(0x7 << ((pwm_id - 8) * 3 + 16));
		}
	}
#endif /* CONFIG_TARGET_NA51090_A64 */
}
#endif

int pwm_init(int pwm_id, int div, int invert)
{
	u32 prd_reg = 0x0;
#ifndef CONFIG_TARGET_NA51000
	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};
#endif

#ifdef CONFIG_TARGET_NA51000
	/* Enable clk */
	*(u32 *)(CONIFG_CG_BASE + 0x8C) |= (0x1 << pwm_id);
	*(u32 *)(CONIFG_CG_BASE + 0xA8) |= (0x1 << 8);
#else
#ifdef CONFIG_TARGET_NA51090_A64
	/* Enable clk */
	*(u32 *)(CONIFG_CG_BASE + 0x7C) |= (0x1 << pwm_id);
	*(u32 *)(CONIFG_CG_BASE + 0x9C) |= (0x1 << 9);
#else
	/* Enable clk */
	*(u32 *)(CONIFG_CG_BASE + 0x7C) |= (0x1 << pwm_id);
	*(u32 *)(CONIFG_CG_BASE + 0x88) |= (0x1 << 8);
#endif
	/* Enable pinmux depends on dts */
	if ((pwm_id < 4) || (pwm_id == 8) || (pwm_id == 9)) {
		sprintf(path, "/top@%x/pwm", CONFIG_TOP_BASE);
		nodeoffset = fdt_path_offset((const void *)fdt_addr, path);
		cell = (u32 *)fdt_getprop((const void *)fdt_addr, nodeoffset, "pinmux", NULL);
		pwm_pinmux = __be32_to_cpu(cell[0]);
		printf("pwm_pinmux = 0x%x\n", pwm_pinmux);
		nvt_pwm_set_pinmux(pwm_id, pwm_pinmux);
	} else {
		sprintf(path, "/top@%x/pwm2", CONFIG_TOP_BASE);
		nodeoffset = fdt_path_offset((const void *)fdt_addr, path);
		cell = (u32 *)fdt_getprop((const void *)fdt_addr, nodeoffset, "pinmux", NULL);
		pwm2_pinmux = __be32_to_cpu(cell[0]);
		printf("pwm2_pinmux = 0x%x\n", pwm2_pinmux);
		nvt_pwm_set_pinmux(pwm_id, pwm2_pinmux);
	}
#endif

	/* Disable pwm before inverting */
	nvt_pwm_write_reg(PWM_DIS_REG, (u32)(0x1 << pwm_id));

	/* Invert pwm */
	prd_reg = nvt_pwm_read_reg(PWM_PRD_REG(pwm_id));

	if (invert == 1) {
		prd_reg |= PWM_INV_BIT;
	} else {
		prd_reg &= ~PWM_INV_BIT;
	}

	nvt_pwm_write_reg(PWM_PRD_REG(pwm_id), prd_reg);

	return 0;
}

int pwm_config(int pwm_id, int duty_ns, int period_ns)
{
	u32 prd_reg = 0x0;
	u32 prd_hi_reg = 0x0;
	u64 period = 0, duty = 0;

	/* Set free run mode */
	nvt_pwm_write_reg(PWM_CTRL_REG(pwm_id), (u32)0x0);

	period = (u64)pclk_freq * (u64)period_ns;
	do_div(period, 1000000000);

	duty = (u64)pclk_freq * (u64)duty_ns;
	do_div(duty, 1000000000);

	prd_reg = nvt_pwm_read_reg(PWM_PRD_REG(pwm_id));

	prd_reg &= ~PWM_RISE_MASK;  /* Output is high since started */

	prd_reg &= ~PWM_FALL_MASK;
	prd_reg |= (PWM_FALL_MASK & (duty << 8));

	prd_reg &= ~PWM_PERIOD_MASK;
	prd_reg |= (PWM_PERIOD_MASK & (period << 16));

	if (pwm_id < 16) {
		nvt_pwm_write_reg(PWM_PRD_REG(pwm_id), prd_reg);
	} else {
		nvt_pwm_write_reg(PWM_PRD_REG(pwm_id) + 0x20, prd_reg);
	}

	if (pwm_id < 8) {
		prd_hi_reg &= ~PWM_RISE_HI_MASK;  /* Output is high since started */

		prd_hi_reg &= ~PWM_FALL_HI_MASK;
		prd_hi_reg |= (PWM_FALL_HI_MASK & ((duty >> 8) << 8));

		prd_hi_reg &= ~PWM_PERIOD_HI_MASK;
		prd_hi_reg |= (PWM_PERIOD_HI_MASK & ((period >> 8) << 16));

		nvt_pwm_write_reg(PWM_PRD_HI_REG(pwm_id), prd_hi_reg);
	}

	return 0;
}

int pwm_enable(int pwm_id)
{
	nvt_pwm_write_reg(PWM_EN_REG, (u32)(0x1 << pwm_id));

	/* Enable pinmux */
#ifdef CONFIG_TARGET_NA51000
	*(u32 *)(CONFIG_TOP_BASE + 0x1C) |= (0x1 << pwm_id);
	if (pwm_id == 0) {
		*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x1 << 23);
	}

	*(u32 *)(CONFIG_TOP_BASE + 0xA8) &= ~(0x1 << pwm_id);
#endif

	return 0;
}

void pwm_disable(int pwm_id)
{
	nvt_pwm_write_reg(PWM_DIS_REG, (u32)(0x1 << pwm_id));

	/* Disable pinmux */
#ifdef CONFIG_TARGET_NA51000
	*(u32 *)(CONFIG_TOP_BASE + 0x1C) &= ~(0x1 << pwm_id);

	*(u32 *)(CONFIG_TOP_BASE + 0xA8) |= (0x1 << pwm_id);
#endif

}

static int do_pwm(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int id, inv = 0;
	u32 period_ns = 0x0, duty_ns = 0x0;
	u32 freq = 0x0;
	u32 pinmux = 0x0;

	if (!strncmp(argv[1], "config", 6)) {
		if (argc < 6) {
			return CMD_RET_USAGE;
		}

		id = simple_strtoul(argv[2], NULL, 10);

		inv = simple_strtoul(argv[3], NULL, 10);

		period_ns = simple_strtoul(argv[4], NULL, 10);

		duty_ns = simple_strtoul(argv[5], NULL, 10);

		if ((id >= NUM_PWM) || (inv > 1) || (duty_ns > period_ns)) {
			return CMD_RET_USAGE;
		}

		pwm_init(id, 0, inv);

		pwm_config(id, duty_ns, period_ns);

		return 0;
	} else if (!strncmp(argv[1], "enable", 6)) {
		if (argc < 3) {
			return CMD_RET_USAGE;
		}

		id = simple_strtoul(argv[2], NULL, 10);
		if (id >= NUM_PWM) {
			return CMD_RET_USAGE;
		}

		pwm_enable(id);

		return 0;
	} else if (!strncmp(argv[1], "disable", 7)) {
		if (argc < 3) {
			return CMD_RET_USAGE;
		}

		id = simple_strtoul(argv[2], NULL, 10);
		if (id >= NUM_PWM) {
			return CMD_RET_USAGE;
		}

		pwm_disable(id);

		return 0;
	} else if (!strncmp(argv[1], "freq", 4)) {
		if (argc < 4) {
			return CMD_RET_USAGE;
		}

		id = simple_strtoul(argv[2], NULL, 10);
		if (id >= NUM_PWM) {
			return CMD_RET_USAGE;
		}

		freq = simple_strtoul(argv[3], NULL, 10);

		nvt_pwm_set_freq(id, freq);

		return 0;
#ifndef CONFIG_TARGET_NA51000
	} else if (!strncmp(argv[1], "pinmux", 6)) {
		if (argc < 4) {
			return CMD_RET_USAGE;
		}

		id = simple_strtoul(argv[2], NULL, 10);
		if (id >= NUM_PWM) {
			return CMD_RET_USAGE;
		}

		pinmux = simple_strtoul(argv[3], NULL, 10);

		nvt_pwm_set_pinmux(id, pinmux);

		return 0;
#endif
	} else {
		return CMD_RET_USAGE;
	}
}

#ifdef CONFIG_TARGET_NA51090_A64
void pwm_set(int pwm_id, int level)
{
	u32 duty = 0;

	if (level <= 100) {
		pwm_init(pwm_id, 0, 0);

		duty = 84 * level;

		pwm_config(pwm_id, duty, 8400);

		pwm_enable(pwm_id);
	}
}

static int do_power_ctl(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	u32 level = 0x0;

	if (argc == 2) {
		level = simple_strtoul(argv[1], NULL, 10);
		pwm_set(0, level);

		return 0;
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	power_ctl,  2,  0,   do_power_ctl,
	"nvt core power control",
	"[level]\n"
);
#endif

U_BOOT_CMD(
	pwm,  6,  0,   do_pwm,
	"nvt pwm operation",
	"config [id] [invert] [period_ns] [duty_ns]\n"
	"pwm enable [id]\n"
	"pwm disable [id]\n"
	"pwm freq [id] [freq(Hz)]\n"
	"Note:\n"
	"Defalut freq is 250KHz,\n"
	"therefore each unit of [period_ns] and [duty_ns] should be 4000 ns,\n"
	"for channel 0 to 7 the maximum unit is 65535, so legal range of [period_ns] is 8000 to 262140000 ns,\n"
	"for other channels the maximum unit is 255, so legal range of [period_ns] is 8000 to 1020000 ns.\n"
);
