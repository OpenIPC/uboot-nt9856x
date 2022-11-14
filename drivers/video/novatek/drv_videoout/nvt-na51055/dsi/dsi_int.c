/*
    MIPI DSI driver

    MIPI DSI physical driver

    @file       dsi_int.c
    @ingroup    mIDrvDisp_DSI
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifdef __KERNEL__
#include <asm/nvt-common/rcw_macro_bit.h>
//#include <mach/ioaddress.h>
#include <asm/nvt-common/nvt_types.h>
//#include "kwrap/semaphore.h"
//#include "kwrap/flag.h"
//#include "dsi_dbg.h"
#include "include/dsi_int.h"
#else
#if defined(__FREERTOS)
#define __MODULE__    rtos_dsi
#include <kwrap/debug.h>
#include <kwrap/spinlock.h>
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
#include "kwrap/util.h"
#include "dsi.h"
#include "include/dsi_int.h"
#include "gpio.h"
#else
#include "DrvCommon.h"
#include "Utility.h"
#include "dsi.h"
#include "dsi_int.h"
#endif
#if defined(_NVT_FPGA_)
#include "i2c.h"
#endif
#endif
#ifndef DBGD
#define DBGD(x)   printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#endif

#ifndef DBGH
#define DBGH(x)   printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#endif

#ifndef DBG_DUMP
#define DBG_DUMP(fmtstr, args...) printf(fmtstr, ##args)
#endif

#ifndef DBG_ERR
#define DBG_ERR(fmtstr, args...)  printf("\033[0;31mERR:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#ifndef DBG_WRN
#define DBG_WRN(fmtstr, args...)  printf("\033[0;33mWRN:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#if 0
#define DBG_IND(fmtstr, args...) printf("%s(): " fmtstr, __func__, ##args)
#else
#ifndef DBG_IND
#define DBG_IND(fmtstr, args...)
#endif
#endif

#if defined(__FREERTOS)
#if defined(_TC680_) && defined(_NVT_FPGA_)
static  VK_DEFINE_SPINLOCK(my_lock);
#define loc_cpu(flags) vk_spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags) vk_spin_unlock_irqrestore(&my_lock, flags)

// Use PWM 0/1/2 pins
#define TC_SIF_CS   P_GPIO_0
#define TC_SIF_CLK  P_GPIO_1
#define TC_SIF_DAT  P_GPIO_2

#define Delay_DelayUs        vos_util_delay_us
#define Delay_DelayUsPolling vos_util_delay_us_polling
#define Delay_DelayMs        vos_util_delay_ms

void dsi_tc680_init(void)
{
	gpio_setDir(TC_SIF_CS,      GPIO_DIR_OUTPUT);
	gpio_setDir(TC_SIF_CLK,     GPIO_DIR_OUTPUT);
	gpio_setDir(TC_SIF_DAT,     GPIO_DIR_OUTPUT);

	gpio_setPin(TC_SIF_CS);
	gpio_setPin(TC_SIF_CLK);
	gpio_setPin(TC_SIF_DAT);

	Delay_DelayMs(200);

	// Turn off SLVS-EC termination
	{
		UINT32 i, uiValue = 0;

		for (i = 0; i < 8; i++) {
			dsi_tc680_readreg(0x2003 + (0x100 * i),  &uiValue);
			uiValue &= ~0x1;
			dsi_tc680_writereg(0x2003 + (0x100 * i), uiValue);
		}

		dsi_tc680_readreg(0x2F03,  &uiValue);
		uiValue &= ~0x1;
		dsi_tc680_writereg(0x2F03, uiValue);

		dsi_tc680_readreg(0x2F00,  &uiValue);
		uiValue &= ~0x1;
		dsi_tc680_writereg(0x2F00, uiValue);

		dsi_tc680_readreg(0x2F06,  &uiValue);
		uiValue &= ~0x1;
		dsi_tc680_writereg(0x2F06, uiValue);

		dsi_tc680_readreg(0x2F0D,  &uiValue);
		uiValue &= ~0x30;
		dsi_tc680_writereg(0x2F0D, uiValue);

		dsi_tc680_readreg(0x2F07,  &uiValue);
		uiValue &= ~0x1;
		dsi_tc680_writereg(0x2F07, uiValue);

		dsi_tc680_readreg(0x2C11,  &uiValue);
		uiValue &= ~0x1;
		dsi_tc680_writereg(0x2C11, uiValue);

		dsi_tc680_readreg(0x2C13,  &uiValue);
		uiValue &= ~0x50;
		dsi_tc680_writereg(0x2C13, uiValue);
	}

	dsi_tc680_writereg(0x3C11, 0x00);      // Vx1 PHY Power Down
	dsi_tc680_writereg(0x0008, 0x005348FF);//Power Down Audio Codec
	dsi_tc680_writereg(0x0009, 0x93F);     //Power Down Audio Codec

}

ER dsi_tc680_writereg(UINT32 ui_offset, UINT32 ui_value)
{
	UINT32 i, dly = 25;
	UINT64 dataout;
	unsigned long flag = 0;

	DBG_DUMP("SET TC680 REG[0x%04X] = 0x%08X\r\n", (unsigned int)ui_offset, (unsigned int)ui_value);

	// write data
	dataout = (UINT64)ui_value + (((UINT64)ui_offset & 0x7FFF) << 32) + (((UINT64)1) << 47);

	loc_cpu(flag);

	for (i = 0; i < 48; i++) {
		gpio_clearPin(TC_SIF_CLK);
		gpio_clearPin(TC_SIF_CS);

		if (dataout & ((UINT64)1 << (47 - i))) {
			gpio_setPin(TC_SIF_DAT);
		} else {
			gpio_clearPin(TC_SIF_DAT);
		}

		Delay_DelayUsPolling(dly);

		gpio_setPin(TC_SIF_CLK);

		Delay_DelayUsPolling(dly);

	}

	gpio_setPin(TC_SIF_CS);
	gpio_setPin(TC_SIF_DAT);
	unl_cpu(flag);

	// bug patch for tc96680
	dsi_tc680_readreg(ui_offset, &dly);
	return E_OK;

}

ER dsi_tc680_readreg(UINT32 ui_offset, UINT32 *pui_value)
{
	UINT32 i, Ret = 0, dly = 25;
	UINT32 dataout;
	unsigned long flag = 0;


	// write data
	dataout = ui_offset & 0x7FFF;

	loc_cpu(flag);

	// Write
	for (i = 0; i < 16; i++) {
		gpio_clearPin(TC_SIF_CLK);
		gpio_clearPin(TC_SIF_CS);

		if (dataout & (1 << (15 - i))) {
			gpio_setPin(TC_SIF_DAT);
		} else {
			gpio_clearPin(TC_SIF_DAT);
		}

		Delay_DelayUsPolling(dly);

		gpio_setPin(TC_SIF_CLK);

		Delay_DelayUsPolling(dly);

	}

	// Dont Care T
	{
		gpio_clearPin(TC_SIF_CLK);
		gpio_clearPin(TC_SIF_CS);

		gpio_setDir(TC_SIF_DAT, GPIO_DIR_INPUT);

		Delay_DelayUsPolling(dly);

		gpio_setPin(TC_SIF_CLK);

		Delay_DelayUsPolling(dly);

	}


	// Read
	for (i = 0; i < 32; i++) {
		gpio_clearPin(TC_SIF_CLK);
		gpio_clearPin(TC_SIF_CS);

		Ret |= (gpio_getPin(TC_SIF_DAT) << (31 - i));

		Delay_DelayUsPolling(dly);

		gpio_setPin(TC_SIF_CLK);

		Delay_DelayUsPolling(dly);

	}

	gpio_setPin(TC_SIF_CS);
	gpio_setDir(TC_SIF_DAT, GPIO_DIR_OUTPUT);
	gpio_setPin(TC_SIF_DAT);
	unl_cpu(flag);

	*pui_value = Ret;

	return E_OK;
}
#endif

#if defined(_TC18039_) && defined(_NVT_FPGA_)
static I2C_SESSION  g_dsi_i2c_ses = I2C_TOTAL_SESSION;
static PI2C_OBJ     p_dsi_i2c_obj;

void dsi_tc18039_init(void)
{
	// For PHY board I2C connection
	p_dsi_i2c_obj = i2c_getDrvObject(0);
	if (p_dsi_i2c_obj->open(&g_dsi_i2c_ses) != E_OK) {
		DBG_ERR("Open I2C driver fail\r\n");
		return;
	}

	p_dsi_i2c_obj->setConfig(g_dsi_i2c_ses, I2C_CONFIG_ID_MODE,        I2C_MODE_MASTER);
	p_dsi_i2c_obj->setConfig(g_dsi_i2c_ses, I2C_CONFIG_ID_BUSCLOCK,    80000);
	p_dsi_i2c_obj->setConfig(g_dsi_i2c_ses, I2C_CONFIG_ID_HANDLE_NACK, TRUE);
	//======clock setting========
	// MPLL AXI CK, APB, MIPI clock 120M
	//MPLL page
	dsi_write_phy_reg(0xFF, 0x19);
	dsi_write_phy_reg(0x00, 0x04);
	dsi_write_phy_reg(0x01, 0x04);
	//MPLL bank
	dsi_write_phy_reg(0xFF, 0x18);
	//ratio_24b = freq(MHz)*2^17/12
	dsi_write_phy_reg(0x6a, 0x00);
	dsi_write_phy_reg(0x6b, 0x00);
	dsi_write_phy_reg(0x6c, 0x14);
	// EMMC_4XCK, DSI clock 960M
	dsi_write_phy_reg(0xFF, 0x19);
	dsi_write_phy_reg(0x00, 0x04);
	dsi_write_phy_reg(0x10, 0x20);
	//MPLL bank
	dsi_write_phy_reg(0xFF, 0x18);
	// EMMC = EMMC_4XCK/4
	////ratio_24b = freq(MHz)*2^17/12
	dsi_write_phy_reg(0xc4, 0x00);
	dsi_write_phy_reg(0xc5, 0x00);
	dsi_write_phy_reg(0xc6, 0x02);//01: 24M; 0a: 240M; 28: 960M
	// ATV_DEI_CK, LP clock 60M
	dsi_write_phy_reg(0xFF, 0x19);
	dsi_write_phy_reg(0x00, 0x02);
	dsi_write_phy_reg(0x14, 0x04);
	//MPLL bank
	dsi_write_phy_reg(0xFF, 0x18);
	//ratio_24b = freq(MHz)*2^17/12
	dsi_write_phy_reg(0x79, 0x00);
	dsi_write_phy_reg(0x7a, 0x00);
	dsi_write_phy_reg(0x7b, 0x0a);
	//======pinmux setting========
	//pinmux DSI
	dsi_write_phy_reg(0xFF, 0x00);
	dsi_write_phy_reg(0xFD, 0xF2);
	//dsi_write_phy_reg(0x14, 0x04);
	//GPIO page
	dsi_write_phy_reg(0xFF, 0x20);
	//switch EMMC IO to input
	dsi_write_phy_reg(0x18, 0x02);
	dsi_write_phy_reg(0x19, 0x02);
	dsi_write_phy_reg(0x1a, 0x02);
	dsi_write_phy_reg(0x1b, 0x02);
	dsi_write_phy_reg(0x1c, 0x02);
	dsi_write_phy_reg(0x1d, 0x02);
	dsi_write_phy_reg(0x1e, 0x02);
	dsi_write_phy_reg(0x1f, 0x02);
	dsi_write_phy_reg(0x20, 0x02);
	dsi_write_phy_reg(0x21, 0x02);
	dsi_write_phy_reg(0x22, 0x02);
	dsi_write_phy_reg(0x23, 0x02);
	dsi_write_phy_reg(0x24, 0x02);
	dsi_write_phy_reg(0x25, 0x02);
	dsi_write_phy_reg(0x26, 0x02);
	dsi_write_phy_reg(0x27, 0x02);
	dsi_write_phy_reg(0x28, 0x02);
	dsi_write_phy_reg(0x29, 0x02);
	dsi_write_phy_reg(0x2a, 0x02);
	dsi_write_phy_reg(0x2b, 0x02);

	//======controller setting setting========
	// page mipi_top
	dsi_write_phy_reg(0xFF, 0x09);
	//dsi phy enable, write DSI APB addr 0 => 0x2(00000002)
	//B0
	dsi_write_phy_reg(0x90, 0x02);
	//00, B1
	dsi_write_phy_reg(0x00, 0x00);
	//XX, B2
	dsi_write_phy_reg(0x00, 0x00);
	//XX, B3
	dsi_write_phy_reg(0x00, 0x00);
	// PHY_LP_RX_DAT EN
	//B0(0~7)//
	dsi_write_phy_reg(0x90, 0x14);
	//00, B1(8~15)//
	dsi_write_phy_reg(0xD8, 0x00);
	//XX, B2 (16~23)//
	dsi_write_phy_reg(0x00, 0x00);
	//XX, B3 (24~31)//
	dsi_write_phy_reg(0x00, 0xF0);

	//Set 3.3V GPIO input(GPIO 054~GPIO 091)
	//Set TOP Bank1 cfgreg
	dsi_write_phy_reg(0xFF, 0x20);
	dsi_write_phy_reg(0x08, 0xaa);
	dsi_write_phy_reg(0x09, 0xaa);
	dsi_write_phy_reg(0x0a, 0xaa);
	dsi_write_phy_reg(0x0b, 0xaa);
	dsi_write_phy_reg(0x0c, 0xaa);
	dsi_write_phy_reg(0x0d, 0xaa);
	dsi_write_phy_reg(0x0e, 0xaa);
	dsi_write_phy_reg(0x0f, 0xaa);
	dsi_write_phy_reg(0x10, 0xaa);
	dsi_write_phy_reg(0x11, 0xfa);
	//Set EMMC (1.8V) GPIO output
	dsi_write_phy_reg(0x2c, 0x01);
	dsi_write_phy_reg(0x2d, 0x01);
	dsi_write_phy_reg(0x2e, 0x01);
	dsi_write_phy_reg(0x2f, 0x01);
	dsi_write_phy_reg(0x30, 0x01);
	dsi_write_phy_reg(0x31, 0x01);
	dsi_write_phy_reg(0x32, 0x01);
	dsi_write_phy_reg(0x33, 0x01);
	dsi_write_phy_reg(0x34, 0x01);
	dsi_write_phy_reg(0x35, 0x01);
	dsi_write_phy_reg(0x36, 0x01);
	dsi_write_phy_reg(0x37, 0x01);
	dsi_write_phy_reg(0x38, 0x01);
	dsi_write_phy_reg(0x39, 0x01);
	dsi_write_phy_reg(0x3a, 0x01);
	dsi_write_phy_reg(0x3b, 0x01);
	dsi_write_phy_reg(0x3c, 0x01);

	//======controller setting setting========
	// page mipi_top
	dsi_write_phy_reg(0xFF, 0x09);
	//dsi phy enable, write DSI APB addr 0 => 0x2(00000002)
	//B0
	dsi_write_phy_reg(0x80, 0x02);
	//00, B1
	dsi_write_phy_reg(0x00, 0x00);
	//XX, B2
	dsi_write_phy_reg(0x00, 0x00);
	//XX, B3
	dsi_write_phy_reg(0x00, 0x00);

	p_dsi_i2c_obj->close(g_dsi_i2c_ses);
}
#endif

/*    Write DSI PHY register
	@param[in] uiOffset     register address
	@param[in] uiValue      register value
	@return
		- @b E_OK: success
*/
ER dsi_write_phy_reg(UINT32 uiOffset, UINT32 uiValue) {
#if defined(_TC18039_) && defined(_NVT_FPGA_)
	I2C_DATA    i2c_data;
	I2C_BYTE    i2c_byte[4];
	I2C_STS     i2c_sts;
	UINT32      addr = 0xC0;

	i2c_lock(g_dsi_i2c_ses);
	i2c_data.VersionInfo     = DRV_VER_98520;
	i2c_data.pByte           = i2c_byte;
	i2c_data.ByteCount       = I2C_BYTE_CNT_3;

	i2c_byte[0].uiValue      = addr;
	i2c_byte[0].Param        = I2C_BYTE_PARAM_START;
	i2c_byte[1].uiValue      = uiOffset;
	i2c_byte[1].Param        = I2C_BYTE_PARAM_NONE;
	i2c_byte[2].uiValue      = uiValue;
	i2c_byte[2].Param        = I2C_BYTE_PARAM_STOP;

	DBG_DUMP("I2c Tx 0x%X 0x%X 0x%X\r\n", addr, uiOffset, uiValue);
	i2c_sts = i2c_transmit(&i2c_data);
	if (i2c_sts != I2C_STS_OK) {
		DBG_ERR("Tx Err. %d\r\n", i2c_sts);
	}

	i2c_unlock(g_dsi_i2c_ses);
#else
	// Real board
	//DSI_SETREG(DSI_PHY_BASE_OFS + uiOffset * 4, uiValue);
	//DSI_GETREG(DSI_PHY_BASE_OFS + uiOffset * 4);
#endif
	return E_OK;
}

/*    Read DSI PHY register
	@param[in] uiOffset     register address
	@param[out] puiValue    register value
	@return
		- @b E_OK: success
*/
ER dsi_read_phy_reg(UINT32 uiOffset, UINT32 *puiValue) {
#if defined(_TC18039_) && defined(_NVT_FPGA_)
	I2C_DATA    i2c_data;
	I2C_BYTE    i2c_byte[4];
	I2C_STS     i2c_sts;
	UINT32      addr = 0xC0;

	i2c_lock(g_dsi_i2c_ses);

	i2c_data.VersionInfo     = DRV_VER_98520;
	i2c_data.pByte           = i2c_byte;
	i2c_data.ByteCount       = I2C_BYTE_CNT_3;

	i2c_byte[0].uiValue      = addr;
	i2c_byte[0].Param        = I2C_BYTE_PARAM_START;
	i2c_byte[1].uiValue      = uiOffset;
	i2c_byte[1].Param        = I2C_BYTE_PARAM_NONE;
	i2c_byte[2].uiValue      = addr | 0x1;
	i2c_byte[2].Param        = I2C_BYTE_PARAM_START;

	i2c_sts = i2c_transmit(&i2c_data);

	if (i2c_sts != I2C_STS_OK) {
		DBG_ERR("Tx Err1. %d\r\n", i2c_sts);
	}

	i2c_data.ByteCount   = I2C_BYTE_CNT_1;
	i2c_byte[0].Param    = I2C_BYTE_PARAM_NACK | I2C_BYTE_PARAM_STOP;
	if (i2c_receive(&i2c_data) != I2C_STS_OK) {
		DBG_ERR("RX Err. %d\r\n", i2c_sts);
	}

	i2c_unlock(g_dsi_i2c_ses);

	*puiValue = i2c_byte[0].uiValue;
#else
		// Real board
		//*puiValue = DSI_GETREG(DSI_PHY_BASE_OFS + uiOffset * 4);
#endif
	return E_OK;
}


#endif

/*
     DSI get mode configuration

     Need to be set before operation

     @return DSI_MODESEL
		- @b DSI_MODE_MANUAL_MODE  : manual mode
		- @b DSI_MODE_AUTO_MODE1   : auto mode 1
		- @b DSI_MODE_AUTO_MODE2   : auto mode 2
		- @b DSI_MODE_AUTO_MODE3   : auto mode 3
*/
DSI_MODESEL dsi_get_mode(void)
{
	T_DSI_CTRL0_REG reg;

	reg.reg = DSI_GETREG(DSI_CTRL0_REG_OFS);

	return (DSI_MODESEL)reg.bit.MODE;
}

#if 0
/*
     DSI escape command control

     Configure DSI escape command register

     @param[in]     ui_cmd_reg    Emulation of DSI_CFG_ESCAPE_CMD_CTRL
     @param[in,out] param       Depend on specific Emulation
     @return Description of data returned.
		- @b E_OK:     Success
		- @b E_SYS:    fail
*/
ER dsi_set_escape_cmd_register(DSI_CFG_ESCAPE_CMD_CTRL ui_cmd_reg, UINT32 param)
{
	T_DSI_ESCCTRL0_REG   cmd_reg0;
	T_DSI_ESCCTRL1_REG   cmd_reg1;

	if (ui_cmd_reg <= DSI_SET_CLK_ULP_SEL) {
		cmd_reg0.reg = DSI_GETREG(DSI_ESCCTRL0_REG_OFS);
	} else {
		cmd_reg1.reg = DSI_GETREG(DSI_ESCCTRL1_REG_OFS);
	}

	switch (ui_cmd_reg) {
	case DSI_SET_DAT0_ESC_START:
		cmd_reg0.bit.DAT0_ESC_START = param;
		break;

	case DSI_SET_DAT1_ESC_START:
		cmd_reg0.bit.DAT1_ESC_START = param;
		break;

	case DSI_SET_DAT0_ESC_STOP:
		cmd_reg0.bit.DAT0_ESC_STOP = param;
		break;

	case DSI_SET_DAT1_ESC_STOP:
		cmd_reg0.bit.DAT1_ESC_STOP = param;
		break;

	case DSI_SET_CLK_ULP_SEL:
		cmd_reg0.bit.CLK_ULP_SEL = param;
		break;

	case DSI_SET_DAT0_ESC_CMD:
		cmd_reg1.bit.DAT0_ESC_CMD = param;
		break;

	case DSI_SET_DAT1_ESC_CMD:
		cmd_reg1.bit.DAT1_ESC_CMD = param;
		break;

	default:
		DSI_ERR_MSG("dsi_set_escape_cmd_register error command[%d]\r\n", ui_cmd_reg);
		return E_SYS;
		//break;
	}

	if (ui_cmd_reg <= DSI_SET_CLK_ULP_SEL) {
		DSI_SETREG(DSI_ESCCTRL0_REG_OFS, cmd_reg0.reg);
	} else {
		DSI_SETREG(DSI_ESCCTRL1_REG_OFS, cmd_reg1.reg);
	}
	return E_OK;
}


ER dsi_set_mode(DSI_CONFIG_ID config_sel, UINT32 ui_param)
{
	return E_OK;
}

ER dsi_set_pixel_format(DSI_CONFIG_ID config_sel, UINT32 ui_param)
{
	return E_OK;
}

ER dsi_set_pixpkt_mode(DSI_CONFIG_ID config_sel, UINT32 ui_param)
{
	return E_OK;
}

ER dsi_set_vdopkt_type(DSI_CONFIG_ID config_sel, UINT32 ui_param)
{
	return E_OK;
}

ER dsi_set_rxecc_chk_en(DSI_CONFIG_ID config_sel, UINT32 ui_param)
{
	return E_OK;
}

ER dsi_set_frmend_bta_en(DSI_CONFIG_ID config_sel, UINT32 ui_param)
{
	return E_OK;
}

ER dsi_set_eotpkt_en(DSI_CONFIG_ID config_sel, UINT32 ui_param)
{
	return E_OK;
}

ER dsi_set_blank_ctrl(DSI_CONFIG_ID config_sel, UINT32 ui_param)
{
	return E_OK;
}
#endif
