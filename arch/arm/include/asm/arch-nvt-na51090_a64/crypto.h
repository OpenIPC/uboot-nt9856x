/*
 * Copyright (C) 2014 Novatek Microelectronics Corp. All rights reserved.
 * Author: SP-KSW <SP_KSW_MailGrp@novatek.com.tw>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_CRYPTO_H_
#define __ARCH_CRYPTO_H_

#include <asm/nvt-common/nvt_types.h>
#include <asm/arch/efuse_protected.h>

/**
    Crypto engine crypto mode
*/
typedef enum {
    CRYPTO_RSV0,
    CRYPTO_RSV1,
	CRYPTO_AES,                         ///< Select Crypto mode AES
	CRYPTO_MODE_NUM,
	ENUM_DUMMY4WORD(CRYPTO_MODE)
} CRYPTO_MODE;

/**
    Crypto engine crypto Operating mode
*/
typedef enum {
	CRYPTO_EBC = 0x00,                  ///< Select Crypto opmode EBC
	CRYPTO_CBC,							///< Select Crypto opmode CBC
	CRYPTO_OPMODE_NUM,
	ENUM_DUMMY4WORD(CRYPTO_OPMODE)
} CRYPTO_OPMODE;

/**
    Crypto engine encrypt or decrypt
*/
typedef enum {
	CRYPTO_ENCRYPT = 0x00,              ///< Select Crypto engine encrypt
	CRYPTO_DECRYPT,                     ///< Select Crypto engine decrypt
	CRYPTO_TYPE_NUM,

	ENUM_DUMMY4WORD(CRYPTO_TYPE)
} CRYPTO_TYPE;

/**
    Crypto engine Access Mode selection
*/
typedef enum {
	CRYPTO_PIO = 0x00,                ///< Select Crypto PIO Mode
	CRYPTO_DMA,                       ///< Select Crypto DMA Mode

	CRYPTO_ACCMOD_NUM,
	ENUM_DUMMY4WORD(CRYPTO_ACCMOD)
} CRYPTO_ACCMOD;

/**
    Crypto engine clock rate
*/
typedef enum {
	CRYPTO_CLOCK_80MHz = 0x00,          ///< Select Crypto clock rate 80 MHz
	CRYPTO_CLOCK_160MHz,                ///< Select Crypto clock rate 160 MHz
	CRYPTO_CLOCK_240MHz,                ///< Select Crypto clock rate 240 MHz

	CRYPTO_CLOCK_RATE_NUM,
	ENUM_DUMMY4WORD(CRYPTO_CLOCK_RATE)
} CRYPTO_CLOCK_RATE;

typedef struct _CRYPT_OP {
    CRYPTO_OPMODE   op_mode;            ///< Operation Mode (now support ECB only)
    CRYPTO_TYPE     en_de_crypt;        ///< Encrypt or decrypt (CRYPTO_ENCRYPT or CRYPTO_DECRYPT)
    UINT32          src_addr;           ///< Source address
    UINT32          dst_addr;           ///< Destination address
    UINT32          length;             ///< length
    UINT32        	iv;					///< initial vector during CBC encrypt
} CRYPT_OP;

#endif /* __ARCH_CRYPTO_H_ */

extern s32 crypto_data_operation_by_key(UINT8 * key, CRYPT_OP crypt_op_param);
extern s32 crypto_data_operation(EFUSE_OTP_KEY_SET_FIELD key_set, CRYPT_OP crypt_op_param);



