/**
    nvt-opt key manager
    This file will Enable and disable SRAM shutdown
    @file       nvt-otp.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <common.h>
#include <asm/arch/efuse_protected.h>
#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>
#include "nvt_otp_reg.h"

#ifndef CHKPNT
#define CHKPNT    printf("\033[37mCHK: %d, %s\033[0m\r\n", __LINE__, __func__)
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

#define INREG32(x)      	*((volatile UINT32*)(x))    ///< Write 32bits IO register
#define OUTREG32(x, y)      (*((volatile UINT32*)(x)) = (y))    ///< Write 32bits IO register

extern INT32    local_load(UINT32 uiRowAddress);
extern INT32    local_store(UINT32 uiRowAddress);

#define PLL_CLKSEL_APB_MASK       	(0x03 << PLL_CLKSEL_APB)
#define PLL_CLKSEL_APB           	8
#define PLL_CLKSEL_APB_48         	(0x00 << PLL_CLKSEL_APB)      //< Select APB  48MHz
#define PLL_CLKSEL_APB_60         	(0x01 << PLL_CLKSEL_APB)      //< Select APB  60MHz
#define PLL_CLKSEL_APB_80         	(0x02 << PLL_CLKSEL_APB)      //< Select APB  80MHz
#define PLL_CLKSEL_APB_120        	(0x03 << PLL_CLKSEL_APB)      //< Select APB 120MHz

#define OTP_48_TIMING				0x692618
#define OTP_60_TIMING				0x8B3026
#define OTP_80_TIMING				0xAF4032
#define OTP_120_TIMING				0xFF6050

void otp_init(void)
{
	UINT32	uiReg;

	DBG_DUMP("otp_init!\r\n");

	uiReg = INREG32(IOADDR_CG_REG_BASE + 0x10);


	if((uiReg & PLL_CLKSEL_APB_MASK) == PLL_CLKSEL_APB_48) {
		DBG_DUMP("48MHz\r\n");
		OUTREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS, OTP_48_TIMING);
	} else if((uiReg & PLL_CLKSEL_APB_MASK) == PLL_CLKSEL_APB_60) {
		DBG_DUMP("60MHz\r\n");
		OUTREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS, OTP_60_TIMING);
	} else if((uiReg & PLL_CLKSEL_APB_MASK) == PLL_CLKSEL_APB_80) {
		DBG_DUMP("80MHz\r\n");
		OUTREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS, OTP_80_TIMING);
	} else if((uiReg & PLL_CLKSEL_APB_MASK) == PLL_CLKSEL_APB_120) {
		DBG_DUMP("120MHz\r\n");
		OUTREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS, OTP_120_TIMING);
	}

	uiReg = INREG32(KEY_MANAGER_TIMING_CONFIG_ADDRESS);
	DBG_DUMP("otp_timing_reg= 0x%x\r\n", (int)uiReg);

}


static INT32 key_program_bit(UINT32 rol, UINT32 col)
{
	return local_store(rol | (col << 5));
}


static INT32 key_write_data(UINT32 addr, UINT32 data)
{
	UINT32  ui_bits;
	UINT32  ui_data;

	if (addr < 8) {
		DBG_ERR("key_write_data addr = 0x%08x error(need >= 8)\r\n", (int)addr);
		return EFUSE_PARAM_ERR;
	}

	ui_data = data;

	ui_bits = 0;
	while (ui_data) {
		ui_bits = __builtin_ctz(ui_data);
		ui_data &= ~(1 << ui_bits);
		if (key_program_bit(addr, ui_bits) != E_OK) {
			DBG_ERR("%s,eFuse program addr[%03lx] bit[%2d] fail\r\n", __func__, (UINT32)addr, (int)ui_bits);
			return EFUSE_OPS_ERR;
		}
	}
	return EFUSE_SUCCESS;
}

#define KEY_MANAGER_READ_PARAM()        otp_key_manager(8)
#define KEY_MANAGER_WRITE_PARAM(m)      key_write_data(8, m)


/*
     efuse_set_key_destination

     Destination of efuse get key field value

     @return IC revision of specific package revision
        - @b   NT9666X_VER_A    NT9666X version A
        - @b   UNKNOWN_DIE_VER  Unknown IC die version(system must halt)
*/
static ER set_key_destination(OTP_KEY_DESTINATION key_dst, SCE_KEY_SET_TO_OTP_FIELD key_field_set)
{
	if (key_dst >= OTP_KEY_MANAGER_CNT) {
		DBG_ERR("key_dst out of range %d\r\n", key_dst);
		return EFUSE_PARAM_ERR;
	}

	if (key_dst == OTP_KEY_MANAGER_CRYPTO &&  key_field_set >= EFUSE_CRYPTO_ENGINE_KEY_CNT) {
		DBG_ERR("Dest[Crypto]=>key_field_set out of range %d > %d\r\n", key_field_set, EFUSE_CRYPTO_ENGINE_KEY_CNT);
		return EFUSE_PARAM_ERR;
	}

	if ((key_dst == OTP_KEY_MANAGER_RSA || key_dst == OTP_KEY_MANAGER_HASH) &&  key_field_set >= EFUSE_TOTAL_KEY_SET_FIELD) {
		DBG_ERR("Dest[RSA] or [HASH] =>key_field_set out of range %d > %d\r\n", key_field_set, EFUSE_CRYPTO_ENGINE_KEY_CNT);
		return EFUSE_PARAM_ERR;
	}

	OUTREG32(KEY_MANAGER_DESTINATION_ADDRESS, key_dst);

	OUTREG32(KEY_MANAGER_KEY_INDEX_ADDRESS, key_field_set);

	return E_OK;
}


/*
     efuse_get_unique_id

     efuse get unique id (56bits)

     @param[out]     id_L   LSB of ID (32bit)
     @param[out]     id_H   MSB of ID (17bit)

     @return IC success or fail
        - @b   	EFUSE_SUCCESS      	success
        - @b	EFUSE_OPS_ERR		fail
*/
ER efuse_get_unique_id(UINT32 * id_L, UINT32 * id_H)
{
	UINT64  value;
	UINT32	xy;
	UINT64  dec6 = 0x81BF1000ul;
	UINT64  dec5 = 0x39AA400ul;
	union T_EFUSE_LOT_ID1_REG lot_id_1_reg;
    union T_EFUSE_LOT_ID2_REG lot_id_2_reg;

    lot_id_1_reg.Reg = otp_key_manager(EFUSE_LOT_ID1);
    lot_id_2_reg.Reg = otp_key_manager(EFUSE_LOT_ID2);

#if 0
    lot_id_1_reg.Bit.lot_id_no_1_5_0 = 36;
    lot_id_1_reg.Bit.lot_id_no_2_5_0 = 36;
    lot_id_1_reg.Bit.lot_id_no_3_5_0 = 36;
    lot_id_1_reg.Bit.lot_id_no_4_5_0 = 36;
    lot_id_1_reg.Bit.lot_id_no_5_5_0 = 36;
    lot_id_2_reg.Bit.wafer_id = 26;
#endif
	//catch 36bit
    value  = (INT64)(lot_id_1_reg.Bit.lot_id_no_1_5_0);
    DBG_IND("Unique ID value=[0x%llx]\r\n", value);
    value += (INT64)(lot_id_1_reg.Bit.lot_id_no_2_5_0 * 36);
    DBG_IND("Unique ID value=[0x%llx]\r\n", value);
    value += (INT64)(lot_id_1_reg.Bit.lot_id_no_3_5_0 * 1296);
    DBG_IND("Unique ID value=[0x%llx]\r\n", value);
    value += (INT64)(lot_id_1_reg.Bit.lot_id_no_4_5_0 * 46656);
    DBG_IND("Unique ID value=[0x%llx]\r\n", value);
    value += (INT64)(lot_id_1_reg.Bit.lot_id_no_5_5_0 * 1679616);
    DBG_IND("Unique ID value=[0x%llx]\r\n", value);

    value += (UINT64)(((lot_id_2_reg.Bit.lot_id_no_6_5_2 << 2) | lot_id_1_reg.Bit.lot_id_no_6_1_0) * dec5);
	value += (INT64)(lot_id_2_reg.Bit.wafer_id * dec6);

	xy  = lot_id_2_reg.Bit.x_coordinate;
	xy += (lot_id_2_reg.Bit.y_coordinate * 80);

	* id_L = (UINT32)value;
	* id_H = ((((value >> 32) & 0xF) | (xy << 4)) & 0x1FFFF);

	if((*id_L + *id_H) == 0)
		return EFUSE_OPS_ERR;
	else
		return EFUSE_SUCCESS;
}


/**
    otp_write_key

    Write specific key into specific key set (0~3)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~3)
    @param[in] ucKey           key (16bytes)
    @return Description of data returned.
        - @b E_OK:   Success
        - @b E_SYS:  Fail
*/
INT32 otp_write_key(EFUSE_OTP_KEY_SET_FIELD key_set_index, UINT8 *uc_key)
{
	s32   	result = EFUSE_SUCCESS;
	u32		key_data;
	u32  	u32_key = (UINT32)(uc_key);
	u32  	data[4];
	u32  	key_field_start_index = 16;
	u32  	index_cnt;
	u32  	enable_secure_number = 32;

	u32  	key_field_index;
	u32		param;

	param 	= KEY_MANAGER_READ_PARAM();
	switch (key_set_index) {
	//Note: >>>1st Key set is dedicate for secure boot usage<<<
	case EFUSE_OTP_1ST_KEY_SET_FIELD:
		key_field_start_index = 16;
		if (is_1st_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_1ST_KEY_SET_PROGRAMMED;
		}
		break;

	case EFUSE_OTP_2ND_KEY_SET_FIELD:
		key_field_start_index = 20;
		if (is_2nd_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_2ND_KEY_SET_PROGRAMMED;
		}
		break;

	case EFUSE_OTP_3RD_KEY_SET_FIELD:
		key_field_start_index = 24;
		if (is_3rd_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_3RD_KEY_SET_PROGRAMMED;
		}
		break;

	case EFUSE_OTP_4TH_KEY_SET_FIELD:
		key_field_start_index = 28;
		if (is_4th_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_4TH_KEY_SET_PROGRAMMED;
		}
		break;

	case EFUSE_OTP_5TH_KEY_SET_FIELD:
		key_field_start_index = 12;
		if (is_5th_key_programmed() == 1) {
			result = EFUSE_FREEZE_ERR;
		} else {
			enable_secure_number = SECUREBOOT_5TH_KEY_SET_PROGRAMMED;
		}
		break;

	default:
		DBG_ERR("Unknow key set[%d] => should be 0~4\r\n", (int)key_set_index + 1);
		result = EFUSE_OPS_ERR;
		break;
	}

	if (result != EFUSE_SUCCESS) {
		if (result == EFUSE_FREEZE_ERR) {
			DBG_ERR("key set[%d], key field not enpty\r\n", (int)key_set_index);
		}
		return result;
	}
	//Need porting
	data[0] = *(UINT32 *)(u32_key + 0);

	data[1] = *(UINT32 *)(u32_key + 4);

	data[2] = *(UINT32 *)(u32_key + 8);

	data[3] = *(UINT32 *)(u32_key + 12);

	for (index_cnt = 0; index_cnt < EFUSE_OTP_KEY_FIELD_CNT; index_cnt++) {
		key_field_index = key_field_start_index + index_cnt;
		//4th key is 28,09,30,31
		if(key_field_index == 29 && ((param & OTP_NEW_KEY_RULE_ID) == (UINT32)OTP_NEW_KEY_RULE_ID)) {
			key_field_index -= 20;
		}
		//DBG_DUMP("cnt[%d][0x%08x]\r\n", key_field_index, data[index_cnt]);
		result = key_write_data(key_field_index, data[index_cnt]);

		if (result < 0) {
			DBG_ERR("[%d]set key => write addr[%2d][0x%08x] fail\r\n", (int)(key_set_index+1), (int)key_field_index, (int)data[index_cnt]);
			break;
		} else {
			key_data = otp_key_manager(key_field_index);
			if(key_data != data[index_cnt]) {
				DBG_ERR("[%d]set key => write addr[%2d][0x%08x] fail\r\n", (int)(key_set_index+1), (int)(key_field_index), (int)data[index_cnt]);
				result = EFUSE_CONTENT_ERR;
				break;
			}
		}
	}
	if(result == EFUSE_SUCCESS) {
		enable_secure_boot(enable_secure_number);
	}
	return result;
}

/**
    otp_set_key_destination

    Durung encrypt or decrypt, configure specific key set as AES key(0~3) to crypto engine

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~4)
    @return Description of data returned.
        - @b E_OK:   Success
*/
INT32 otp_set_key_destination(EFUSE_OTP_KEY_SET_FIELD key_set_index)
{
	UINT32  key_field_start_index = 16;
	u32		param;

	param 	= KEY_MANAGER_READ_PARAM();

	switch (key_set_index) {
	//Note: >>>1st Key set is dedicate for secure boot usage<<<
	case EFUSE_OTP_1ST_KEY_SET_FIELD:
		key_field_start_index = 16;
		break;

	case EFUSE_OTP_2ND_KEY_SET_FIELD:
		key_field_start_index = 20;
		break;

	case EFUSE_OTP_3RD_KEY_SET_FIELD:
		key_field_start_index = 24;
		break;

	case EFUSE_OTP_4TH_KEY_SET_FIELD:
		key_field_start_index = 28;
		break;

	case EFUSE_OTP_5TH_KEY_SET_FIELD:
		key_field_start_index = 12;
		break;

	default:
		DBG_ERR("Unknow key set[%d] => should be 0~4\r\n", (int)key_set_index);
		return EFUSE_OPS_ERR;
	}
	set_key_destination(OTP_KEY_MANAGER_CRYPTO, 0);
	otp_key_manager(key_field_start_index);
	set_key_destination(OTP_KEY_MANAGER_CRYPTO, 1);
	if(key_field_start_index == 28 && ((param & OTP_NEW_KEY_RULE_ID) == (UINT32)OTP_NEW_KEY_RULE_ID)) {	//Now = 28 + 1
		//New = 9
		otp_key_manager(9);
    } else {
    	//OLD = 28+1 = 29
		otp_key_manager(key_field_start_index + 1);
	}
	set_key_destination(OTP_KEY_MANAGER_CRYPTO, 2);
	otp_key_manager(key_field_start_index + 2);
	set_key_destination(OTP_KEY_MANAGER_CRYPTO, 3);
	otp_key_manager(key_field_start_index + 3);
	return E_OK;
}

/**
    otp_set_key_read_lock

    Once otp_set_key_read_lock set, this key set field will not allow read value by CPU

    (Only can operate by key manager)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~4)
    @return Description of data returned.
        - @b E_OK:   Success
*/
INT32 otp_set_key_read_lock(EFUSE_OTP_KEY_SET_FIELD key_set_index)
{
	UINT32  key_set_read_lock_index = SECUREBOOT_1ST_KEY_SET_READ_LOCK;
	switch (key_set_index) {
	case EFUSE_OTP_1ST_KEY_SET_FIELD:
		key_set_read_lock_index = SECUREBOOT_1ST_KEY_SET_READ_LOCK;
		break;

	case EFUSE_OTP_2ND_KEY_SET_FIELD:
		key_set_read_lock_index = SECUREBOOT_2ND_KEY_SET_READ_LOCK;
		break;

	case EFUSE_OTP_3RD_KEY_SET_FIELD:
		key_set_read_lock_index = SECUREBOOT_3RD_KEY_SET_READ_LOCK;
		break;

	case EFUSE_OTP_4TH_KEY_SET_FIELD:
		key_set_read_lock_index = SECUREBOOT_4TH_KEY_SET_READ_LOCK;
		break;

	case EFUSE_OTP_5TH_KEY_SET_FIELD:
		key_set_read_lock_index = SECUREBOOT_5TH_KEY_SET_READ_LOCK;
		break;

	default:
		DBG_ERR("Unknow key set[%d] => should be 0~4\r\n", (int)key_set_index);
		return EFUSE_OPS_ERR;
	}

	enable_secure_boot(key_set_read_lock_index);

	return E_OK;
}


/**
    otp_set_key_engine_access_right

    Once otp_set_key_engine_access_right set, this key set field will not transfer key to destination engine(RSA/HASH/Crypto)

    @Note: key set 0 is for secure boot use

    @param[in] key_set_index   key set (0~4)
    @return Description of data returned.
        - @b E_OK:   Success
*/
INT32 otp_set_key_engine_access_right(EFUSE_OTP_KEY_SET_FIELD key_set_index)
{
	UINT32	key_set_engine_access_right_index;
	switch (key_set_index) {
	case EFUSE_OTP_1ST_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_1ST_KEY_BIT_START;
		break;

	case EFUSE_OTP_2ND_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_2ND_KEY_BIT_START;
		break;

	case EFUSE_OTP_3RD_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_3RD_KEY_BIT_START;
		break;

	case EFUSE_OTP_4TH_KEY_SET_FIELD:
		if(is_new_key_rule () == TRUE) {
			key_set_engine_access_right_index = OTP_4TH_KEY_BIT_START_NEW;
		} else {
			key_set_engine_access_right_index = OTP_4TH_KEY_BIT_START;
		}
		break;

	case EFUSE_OTP_5TH_KEY_SET_FIELD:
		key_set_engine_access_right_index = OTP_5TH_KEY_BIT_START;
		break;

	default:
		DBG_ERR("Unknow key set[%d] => should be 0~4\r\n", (int)key_set_index);
		return EFUSE_OPS_ERR;
	}
	EFUSE_SET_ENGINE_ACCESS_RIGHT(key_set_engine_access_right_index);

	return EFUSE_SUCCESS;
}

//Bit[0] & Bit[5] == 1
/**
    quary_secure_boot

    Quary now is what kind of secure boot type

    @Note: key set 0 is for secure boot use

    @param[in] scu_status   status
    @return Description of data returned.
        - @b  TRUE:   enabled
        - @b FALSE:   disabled
*/
BOOL quary_secure_boot(SECUREBOOT_STATUS scu_status)
{
	UINT32  sec;
	BOOL    result = FALSE;

	sec = KEY_MANAGER_READ_PARAM();

	switch (scu_status) {
	case SECUREBOOT_SECURE_EN:
		if ((sec & (OTP_HW_SECURE_EN | OTP_FW_SECURE_EN)) == (OTP_HW_SECURE_EN | OTP_FW_SECURE_EN)) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_DATA_AREA_ENCRYPT:
		if ((sec & OTP_DATA_ENCRYPT_EN) == OTP_DATA_ENCRYPT_EN) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_SIGN_RSA:
		if ((sec & OTP_SIGNATURE_RSA) == OTP_SIGNATURE_RSA) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_SIGN_RSA_CHK:
		if ((sec & OTP_SIGNATURE_RSA_CHK_EN) == OTP_SIGNATURE_RSA_CHK_EN) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_JTAG_DISABLE_EN:
		if ((sec & OTP_JTAG_DISABLE_EN) == OTP_JTAG_DISABLE_EN) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_1ST_KEY_SET_PROGRAMMED:
		if ((sec & OTP_1ST_KEY_PROGRAMMED) == OTP_1ST_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_2ND_KEY_SET_PROGRAMMED:
		if ((sec & OTP_2ND_KEY_PROGRAMMED) == OTP_2ND_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_3RD_KEY_SET_PROGRAMMED:
		if ((sec & OTP_3RD_KEY_PROGRAMMED) == OTP_3RD_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_4TH_KEY_SET_PROGRAMMED:
		if ((sec & OTP_4TH_KEY_PROGRAMMED) == OTP_4TH_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_5TH_KEY_SET_PROGRAMMED:
		if ((sec & OTP_5TH_KEY_PROGRAMMED) == OTP_5TH_KEY_PROGRAMMED) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_1ST_KEY_SET_READ_LOCK:
		if ((sec & OTP_1ST_KEY_READ_LOCK) == OTP_1ST_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_2ND_KEY_SET_READ_LOCK:
		if ((sec & OTP_2ND_KEY_READ_LOCK) == OTP_2ND_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_3RD_KEY_SET_READ_LOCK:
		if ((sec & OTP_3RD_KEY_READ_LOCK) == OTP_3RD_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_4TH_KEY_SET_READ_LOCK:
		if ((sec & OTP_4TH_KEY_READ_LOCK) == OTP_4TH_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_5TH_KEY_SET_READ_LOCK:
		if ((sec & OTP_5TH_KEY_READ_LOCK) == OTP_5TH_KEY_READ_LOCK) {
			result = TRUE;
		}
		break;

	case SECUREBOOT_NEW_KEY_RULE:
		if ((sec & OTP_NEW_KEY_RULE_ID) == OTP_NEW_KEY_RULE_ID) {
			result = TRUE;
		}
		break;

	default:
		break;
	}
	return result;
}

/**
    set_secure_boot

    Quary now is what kind of secure boot type

    @Note: key set 0 is for secure boot use

    @param[in] scu_status   status
    @return Description of data returned.
        - @b  TRUE:   enabled
        - @b FALSE:   something wrong(already configured)
*/
BOOL enable_secure_boot(SECUREBOOT_STATUS scu_status)
{
	UINT32  sec;
	BOOL    result = FALSE;
	sec = KEY_MANAGER_READ_PARAM();

	switch (scu_status) {
	case SECUREBOOT_SECURE_EN:
		if ((sec & (OTP_HW_SECURE_EN | OTP_FW_SECURE_EN)) != 0x0) {
			DBG_ERR("Secure already enabled ...");
		} else {
			result = TRUE;
			KEY_MANAGER_WRITE_PARAM((OTP_HW_SECURE_EN | OTP_FW_SECURE_EN));
		}
		break;

	case SECUREBOOT_DATA_AREA_ENCRYPT:
		if ((sec & OTP_DATA_ENCRYPT_EN) != 0x0) {
			DBG_ERR("Data area encrypted bit already set ...");
		} else {
			result = TRUE;
			KEY_MANAGER_WRITE_PARAM(OTP_DATA_ENCRYPT_EN);
		}
		break;

	case SECUREBOOT_SIGN_RSA:
		if ((sec & OTP_SIGNATURE_RSA) != 0x0) {
			DBG_ERR("Signature use RSA bit already set ...");
		} else {
			result = TRUE;
			KEY_MANAGER_WRITE_PARAM(OTP_SIGNATURE_RSA);
		}
		break;

	case SECUREBOOT_SIGN_RSA_CHK:

		if ((sec & OTP_SIGNATURE_RSA_CHK_EN) != 0x0) {
			DBG_ERR("Signature use RSA and process hash checksum bit already set ...\r\n");
		} else {
			result = TRUE;
			KEY_MANAGER_WRITE_PARAM(OTP_SIGNATURE_RSA_CHK_EN);
		}
		break;

	case SECUREBOOT_JTAG_DISABLE_EN:
		KEY_MANAGER_WRITE_PARAM(OTP_JTAG_DISABLE_EN);
		result = TRUE;
		break;

	case SECUREBOOT_1ST_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_1ST_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_2ND_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_2ND_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_3RD_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_3RD_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_4TH_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_4TH_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_5TH_KEY_SET_PROGRAMMED:
		KEY_MANAGER_WRITE_PARAM(OTP_5TH_KEY_PROGRAMMED);
		result = TRUE;
		break;

	case SECUREBOOT_1ST_KEY_SET_READ_LOCK:
		KEY_MANAGER_WRITE_PARAM(OTP_1ST_KEY_READ_LOCK);
		result = TRUE;
		break;

	//2ND & 3RD key set can not configured as read lock(because of RSA checksum)
#if 0
	case SECUREBOOT_2ND_KEY_SET_READ_LOCK:
		KEY_MANAGER_WRITE_PARAM(OTP_2ND_KEY_READ_LOCK);
		result = TRUE;
		break;

	case SECUREBOOT_3RD_KEY_SET_READ_LOCK:
		KEY_MANAGER_WRITE_PARAM(OTP_3RD_KEY_READ_LOCK);
		result = TRUE;
		break;
#endif

	case SECUREBOOT_4TH_KEY_SET_READ_LOCK:
		KEY_MANAGER_WRITE_PARAM(OTP_4TH_KEY_READ_LOCK);
		result = TRUE;
		break;

	case SECUREBOOT_5TH_KEY_SET_READ_LOCK:
		KEY_MANAGER_WRITE_PARAM(OTP_5TH_KEY_READ_LOCK);
		result = TRUE;
		break;
	case SECUREBOOT_2ND_KEY_SET_READ_LOCK:
	case SECUREBOOT_3RD_KEY_SET_READ_LOCK:
		DBG_ERR("2ND & 3RD key set can not configured as read lock(RSA checksum)");
	default:
		result = FALSE;
		break;
	}
	return result;
}

/*
    otp read data

    Read specific eFuse addressing(32bits/per time)
     ^
     |-- (row address)
    @param[in] rowAddress       row address

    @return read half-word efuse data
        - @b  Positive: Valid data
        - @b    E_SYS : Invalid data
*/
UINT32 otp_key_manager(UINT32 rowAddress)
{
	INT32  uiData;

	uiData = local_load(rowAddress);

	return uiData;
}
