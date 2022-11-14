/**

    @file       crypto.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <common.h>
#include <asm/armv7.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/IOAddress.h>
#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/crypto.h>
#include <asm/arch/efuse_protected.h>

#define CRYPTO_SETREG(ofs,value)    	OUTW(IOADDR_CRYPTO_REG_BASE+(ofs),(value))
#define CRYPTO_GETREG(ofs)          	INW(IOADDR_CRYPTO_REG_BASE+(ofs))

#define CRYPTO_REG_BASE_ADDR			IOADDR_CRYPTO_REG_BASE
#define CLOCK_GEN_REG_BASE_ADDR			IOADDR_CG_REG_BASE
#define CLOCK_GEN_ENABLE_REG0           (CLOCK_GEN_REG_BASE_ADDR + 0x70)


#define CRYPTO_DMA_CONFIG_REG_OFS       0x08
#define CRYPTO_DMA_STS_REG_OFS          0x0C

#define CRYPTO_SRC_ADDR_REG_OFS         0x50
#define CRYPTO_DST_ADDR_REG_OFS         0xA4

#define CRYPTO_DMA_CONTROL_REG_OFS      0x04

#define CRYPTO_KEY000_REG_OFS           0x30
#define CRYPTO_KEY032_REG_OFS           0x34
#define CRYPTO_KEY064_REG_OFS           0x38
#define CRYPTO_KEY096_REG_OFS           0x3C

// Crypto
#define CRYPTO_CONFREG_SWRST          	(1<<0)
#define CRYPTO_CONFREG_CRYPTO_EN     	(1<<1)
#define CRYPTO_CONFREG_AES128         	(2<<4)
#define CRYPTO_CONFREG_DECRYPT        	(1<<8)
#define CRYPTO_PIO_DONE               	(1<<0)


#define dma_getPhyAddr(addr)            ((((UINT32)(addr))>=0x60000000UL)?((UINT32)(addr)-0x60000000UL):(UINT32)(addr))


#define SCE_DES_TABLE_NUM        		(40)      //8(header DesTab) + 32(addr DesTab)
#define SCE_DES_WORD_SIZE        		(4)       //1 DesTab = 4 word
#define SCE_DES_IV_OFS           		(8)
#define SCE_DES_CNT_OFS          		(12)
#define SCE_DES_HAEDER_OFS       		(16)
#define SCE_DES_CV_OFS           		(20)
#define SCE_DES_S0_OFS           		(24)
#define SCE_DES_GHASH_OFS        		(28)
#define SCE_DES_BLOCK_CFG_OFS    		(32)

#define SCE_DMA_COUNT                   (1)
#define ROM_AES_SIZE                    (16)

#define _ALIGNED(x) __attribute__((aligned(x)))
static _ALIGNED(32) UINT32 vuiSCE_DesTab[SCE_DES_TABLE_NUM*SCE_DES_WORD_SIZE];


#define INREG32(x)          (*((volatile UINT32*)(x)))          ///< Read 32bits IO register
#define OUTREG32(x, y)      (*((volatile UINT32*)(x)) = (y))    ///< Write 32bits IO register
#define SETREG32(x, y)      OUTREG32((x), INREG32(x) | (y))     ///< Set 32bits IO register
#define CLRREG32(x, y)      OUTREG32((x), INREG32(x) & ~(y))    ///< Clear 32bits IO register


#define dma_getPhyAddr(addr)            ((((UINT32)(addr))>=0x60000000UL)?((UINT32)(addr)-0x60000000UL):(UINT32)(addr))


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

static u32 crypto_set_mode(u32 uiMode, u32 uiOPMode, u32 uiType)
{
	if (uiMode != CRYPTO_AES) {
		return -1;
	}

	if (uiOPMode >= CRYPTO_OPMODE_NUM) {
		return -1;
	}

	//OUTREG32(CRYPRO_REG_BASE_ADDR, /*(1 << 9) | */(uiMode<<4) | (uiType << 8) | CRYPTO_CONFREG_CRYPTO_EN/* | CRYPTO_CONFREG_SWRST*/); // big endian, descrypt, AES-128, sw-reset
	OUTREG32(CRYPTO_REG_BASE_ADDR, CRYPTO_CONFREG_CRYPTO_EN); // big endian, descrypt, AES-128, sw-reset
	return 0;
}

  //Enable Crypto engine clock    SETREG32(CLOCK_GEN_REG_BASE_ADDR + 0x80, (0x01 << 23));//    uart_putSystemUARTStr("keyoffset=");//    uart_putSystemUARTStr(Dec2HexStr(key_offset));//    uart_putSystemUARTStr("\r\n");    OUTREG32(CRYPRO_REG_BASE_ADDR+0x30, 0x13141516);    OUTREG32(CRYPRO_REG_BASE_ADDR+0x34, 0x09101112);    OUTREG32(CRYPRO_REG_BASE_ADDR+0x38, 0x05060708);    OUTREG32(CRYPRO_REG_BASE_ADDR+0x3C, 0x01020304);    OUTREG32(CRYPRO_REG_BASE_ADDR, (1<<9)|(1<<8)|(2<<1)|(1<<0));    // big endian, descrypt, AES-128, sw-reset//        OUTREG32(CRYPRO_REG_BASE_ADDR, (0<<9)|(1<<8)|(2<<1)|(1<<0));    // little endian, descrypt, AES-128, sw-reset    while (1)       // wait sw reset clear    {        uiReg = INREG32(CRYPRO_REG_BASE_ADDR);        if ((uiReg & (1<<0)) == 0) break;    }


static void crypto_dma_enable(u32 uiMode, u32 uiOPMode, u32 uiType, u32 SrcAddr, u32 DstAddr, u32 Len)
{
	UINT32 	uiReg;
	UINT32 	header_config=0;
	UINT32	block_config =0;
	//clear DMA interrupt status
	uiReg = CRYPTO_GETREG(CRYPTO_DMA_STS_REG_OFS);
	CRYPTO_SETREG(CRYPTO_DMA_STS_REG_OFS, uiReg);

	//Disable interrupt enable
	CRYPTO_SETREG(CRYPTO_DMA_CONFIG_REG_OFS, 0x0);

	memset((void*)&vuiSCE_DesTab[0], 0, SCE_DES_TABLE_NUM*SCE_DES_WORD_SIZE*4);

	//Configure DMA buffer address and size
	header_config = (UINT32)((1 << 12 ) |((uiOPMode) << 8 ) | (uiMode << 4 ) | uiType);
	//Flush cache

	vuiSCE_DesTab[SCE_DES_HAEDER_OFS] = (UINT32)header_config;

	block_config = 0;
    vuiSCE_DesTab[SCE_DES_BLOCK_CFG_OFS]    = dma_getPhyAddr(SrcAddr);
    vuiSCE_DesTab[SCE_DES_BLOCK_CFG_OFS+1]  = dma_getPhyAddr(DstAddr);
    vuiSCE_DesTab[SCE_DES_BLOCK_CFG_OFS+2]  = Len;

    block_config = (UINT32)(1);

    vuiSCE_DesTab[SCE_DES_BLOCK_CFG_OFS+3]  = block_config;
   	flush_dcache_range((ulong)&vuiSCE_DesTab[0], (ulong)&vuiSCE_DesTab[0] + roundup(SCE_DES_TABLE_NUM *SCE_DES_WORD_SIZE*4, ARCH_DMA_MINALIGN));
   	flush_dcache_range(SrcAddr, SrcAddr + roundup(Len, ARCH_DMA_MINALIGN));
	invalidate_dcache_range(DstAddr, DstAddr + roundup(Len, ARCH_DMA_MINALIGN));


	CRYPTO_SETREG(CRYPTO_SRC_ADDR_REG_OFS, dma_getPhyAddr((UINT32)&vuiSCE_DesTab[0]));
	//Set DMA Enable
	CRYPTO_SETREG(CRYPTO_DMA_CONTROL_REG_OFS, (0x1<<4));
	while (!(CRYPTO_GETREG(CRYPTO_DMA_STS_REG_OFS) & (0x1<<4)));
	CRYPTO_SETREG(CRYPTO_DMA_STS_REG_OFS, (0x1<<4));
}

s32 crypto_data_operation(EFUSE_OTP_KEY_SET_FIELD key_set, CRYPT_OP crypt_op_param)
{

	UINT32  reg;
	UINT32  ret;
	reg = INREG32(CLOCK_GEN_REG_BASE_ADDR + 0x1C);
	reg &= ~(0x3 << 20);
	OUTREG32(CLOCK_GEN_REG_BASE_ADDR + 0x1C, reg);

	SETREG32(CLOCK_GEN_ENABLE_REG0, (0x01 << 23));


	ret = crypto_set_mode(CRYPTO_AES, crypt_op_param.op_mode, crypt_op_param.en_de_crypt);
	if (ret < 0) {
		return ret;
	}
	otp_set_key_destination(key_set);
	crypto_dma_enable(CRYPTO_AES, crypt_op_param.op_mode, crypt_op_param.en_de_crypt, crypt_op_param.src_addr, crypt_op_param.dst_addr, crypt_op_param.length);
	CLRREG32(CLOCK_GEN_ENABLE_REG0, (0x01 << 23));
	return 0;
}



//CRYPTO_AES, crypt_op_param.op_mode, crypt_op_param.en_de_crypt, CRYPTO_DMA);

static void crypto_setKey(UINT8 *ucKey)
{
	UINT32  u32_key = (UINT32)(ucKey);

	CRYPTO_SETREG(CRYPTO_KEY000_REG_OFS, *(UINT32 *)(u32_key + 0));
	CRYPTO_SETREG(CRYPTO_KEY032_REG_OFS, *(UINT32 *)(u32_key + 4));
	CRYPTO_SETREG(CRYPTO_KEY064_REG_OFS, *(UINT32 *)(u32_key + 8));
	CRYPTO_SETREG(CRYPTO_KEY096_REG_OFS, *(UINT32 *)(u32_key + 12));
}

s32 crypto_data_operation_by_key(UINT8 * key, CRYPT_OP crypt_op_param)
{
	s32  ret;
	UINT32  reg;

	reg = INREG32(CLOCK_GEN_REG_BASE_ADDR + 0x1C);
	reg &= ~(0x3 << 20);
	OUTREG32(CLOCK_GEN_REG_BASE_ADDR + 0x1C, reg);

	SETREG32(CLOCK_GEN_ENABLE_REG0, (0x01 << 23));


	ret = crypto_set_mode(CRYPTO_AES, crypt_op_param.op_mode, crypt_op_param.en_de_crypt);
	if (ret < 0) {
		return ret;
	}
	crypto_setKey(key);

	crypto_dma_enable(CRYPTO_AES, crypt_op_param.op_mode, crypt_op_param.en_de_crypt, crypt_op_param.src_addr, crypt_op_param.dst_addr, crypt_op_param.length);

	CLRREG32(CLOCK_GEN_ENABLE_REG0, (0x01 << 23));

	return 0;
}

