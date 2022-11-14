/*
 * Crypto API.
 *
 * Support for Novatek NVT Crypto Hardware acceleration.
 *
 * Copyright (c) 2020 Novatek Inc.
 *
 */

#ifdef CONFIG_NVT_CRYPTO
#include <common.h>
#include <asm/io.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/nvt_crypto.h>
#include <asm/arch/efuse_protected.h>

/*************************************************************************************
 *  IO Read/Write Definition
 *************************************************************************************/
#define IO_REG_RD(_base, _ofs)          readl(_base+_ofs)
#define IO_REG_WR(_base, _ofs, _v)      writel(_v, (_base+_ofs))

/*************************************************************************************
 *  Global Definition
 *************************************************************************************/
//#define NVT_CRYPTO_DEBUG

/*************************************************************************************
 *  Debug Message Print Definition
 *************************************************************************************/
#ifdef NVT_CRYPTO_DEBUG
#define crypto_dbg(...)                   printf("[CRYPTO_DBG]: " __VA_ARGS__)
#else
#define crypto_dbg(...)
#endif

#define crypto_err(...)                   printf("[CRYPTO_ERR]: " __VA_ARGS__)
#define crypto_inf(...)                   printf("[CRYPTO_INF]: " __VA_ARGS__)

/*************************************************************************************
 *  Register read/write Definition
 *************************************************************************************/
#define crypto_read(_oft)                 IO_REG_RD(IOADDR_CRYPTO_REG_BASE, _oft)
#define crypto_write(_oft, _v)            IO_REG_WR(IOADDR_CRYPTO_REG_BASE, _oft, _v)

/*************************************************************************************
 *  Register Definition
 *************************************************************************************/
#define NVT_CRYPTO_CFG_REG                0x00           ///< crypto configuration
#define NVT_CRYPTO_CTRL_REG               0x04           ///< crypto control
#define NVT_CRYPTO_INT_ENB_REG            0x08           ///< interrupt enable
#define NVT_CRYPTO_INT_STS_REG            0x0C           ///< interrupt status
#define NVT_CRYPTO_KEY0_REG               0x10           ///< key 0   ~ 31  bit
#define NVT_CRYPTO_KEY1_REG               0x14           ///< key 32  ~ 63  bit
#define NVT_CRYPTO_KEY2_REG               0x18           ///< key 64  ~ 95  bit
#define NVT_CRYPTO_KEY3_REG               0x1C           ///< key 96  ~ 127 bit
#define NVT_CRYPTO_KEY4_REG               0x20           ///< key 128 ~ 159 bit
#define NVT_CRYPTO_KEY5_REG               0x24           ///< key 160 ~ 191 bit
#define NVT_CRYPTO_KEY6_REG               0x28           ///< key 192 ~ 223 bit
#define NVT_CRYPTO_KEY7_REG               0x2C           ///< key 224 ~ 255 bit
#define NVT_CRYPTO_IN0_REG                0x30           ///< input  data  0  ~ 31  bit
#define NVT_CRYPTO_IN1_REG                0x34           ///< input  data 32  ~ 63  bit
#define NVT_CRYPTO_IN2_REG                0x38           ///< input  data 64  ~ 95  bit
#define NVT_CRYPTO_IN3_REG                0x3C           ///< input  data 96  ~ 127 bit
#define NVT_CRYPTO_OUT0_REG               0x40           ///< output data  0  ~ 31  bit
#define NVT_CRYPTO_OUT1_REG               0x44           ///< output data 32  ~ 63  bit
#define NVT_CRYPTO_OUT2_REG               0x48           ///< output data 64  ~ 95  bit
#define NVT_CRYPTO_OUT3_REG               0x4C           ///< output data 96  ~ 127 bit
#define NVT_CRYPTO_DMA0_ADDR_REG          0x50           ///< DMA channel 0 descriptor starting address, DMA mode only
#define NVT_CRYPTO_DMA1_ADDR_REG          0x54           ///< DMA channel 1 descriptor starting address, DMA mode only
#define NVT_CRYPTO_DMA2_ADDR_REG          0x58           ///< DMA channel 2 descriptor starting address, DMA mode only
#define NVT_CRYPTO_DMA3_ADDR_REG          0x5C           ///< DMA channel 0 descriptor starting address, DMA mode only
#define NVT_CRYPTO_KEY_READ_REG           0x60           ///< crypto key readable control

/*************************************************************************************
 *  DMA Descriptor Definition
 *************************************************************************************/
#define NVT_CRYPTO_MAX_IV_SIZE            16             ///< 16  Bytes, 128 bits
#define NVT_CRYPTO_MAX_KEY_SIZE           32             ///< 32  Bytes, 256 bits
#define NVT_CRYPTO_MAX_DMA_BLOCK_NUM      8              ///< hardware maximun up to 32

struct nvt_crypto_dma_block {
	uint32_t    src_addr;
	uint32_t    dst_addr;
	uint32_t    length;
	uint32_t    block_cfg;
} __attribute__((packed, aligned(4)));

struct nvt_crypto_dma_desc {
	uint32_t                     key[NVT_CRYPTO_MAX_KEY_SIZE/sizeof(uint32_t)];     ///< crypto input key value
	uint32_t                     iv[NVT_CRYPTO_MAX_IV_SIZE/sizeof(uint32_t)];       ///< crypto input IV value
	uint32_t                     counter[NVT_CRYPTO_MAX_IV_SIZE/sizeof(uint32_t)];  ///< crypto input counter value in the CTR
	uint32_t                     header_cfg;                                        ///< DMA descriptor header configuration
	uint32_t                     reserved[3];                                       ///< reserve bytes
	uint32_t                     cv[NVT_CRYPTO_MAX_IV_SIZE/sizeof(uint32_t)];       ///< current IV
	uint32_t                     s0[NVT_CRYPTO_MAX_IV_SIZE/sizeof(uint32_t)];       ///< E(K,Y0) or S0 in the GCM
	uint32_t                     ghash[NVT_CRYPTO_MAX_IV_SIZE/sizeof(uint32_t)];    ///< GHASH output data in the GCM
	struct nvt_crypto_dma_block  block[NVT_CRYPTO_MAX_DMA_BLOCK_NUM];               ///< DMA process blocks
} __attribute__((packed, aligned(4)));

static struct nvt_crypto_dma_desc g_dma_desc __attribute__((aligned(32)));          ///< align address to 32 byte for cache line operation

/*************************************************************************************
 *  Local Definition
 *************************************************************************************/
static int nvt_crypto_opened = 0;

static void nvt_crypto_platform_init(void)
{
	uint32_t tmp;

	/* Crypto clock select, 0:240MHz 1:320MHz 2: RVD 3: PLL9 */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x1C);
	tmp &= ~(0x3<<20);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x1C, tmp);

	/* Crypto clock enable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x70);
	tmp |= (0x1<<23);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x70, tmp);

	/* Crypto reset disable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x80);
	tmp |= (0x1<<23);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x80, tmp);
}

static void nvt_crypto_platform_exit(void)
{
	uint32_t tmp;

	/* Crypto reset enable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x80);
	tmp &= ~(0x1<<23);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x80, tmp);

	/* Crypto clock disable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x70);
	tmp &= ~(0x1<<23);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x70, tmp);
}

static void nvt_crypto_reset(void)
{
	uint32_t value = 0;
	uint32_t cnt   = 0;

	/* disable crypto */
	crypto_write(NVT_CRYPTO_CFG_REG, 0);

	/* set reset, hardware will auto clear */
	crypto_write(NVT_CRYPTO_CFG_REG, 0x01);

	/* check reset done */
	while ((value = crypto_read(NVT_CRYPTO_CFG_REG)) & 0x1) {
		if(cnt++ >= 3000000)    ///< 3sec
			break;
		udelay(1);
	}

	/* clear all status */
	crypto_write(NVT_CRYPTO_INT_STS_REG, 0xFF1);

	if (value & 0x1) {
		crypto_err("crypto hardware reset failed!!\n");
	}
}

int nvt_crypto_open(void)
{
	if (!nvt_crypto_opened) {
		/* platform pmu init for crypto engine */
		nvt_crypto_platform_init();

		/* crypto engine software reset */
		nvt_crypto_reset();

		nvt_crypto_opened = 1;
	}

	return 0;
}

void nvt_crypto_close(void)
{
	if (nvt_crypto_opened) {
		nvt_crypto_platform_exit();
		nvt_crypto_opened = 0;
	}
}

int nvt_crypto_pio_aes(struct nvt_crypto_pio_t *p_crypto)
{
	int    i;
	int    ret = 0;
	uint32_t cnt = 0;
	uint32_t reg_value;
	uint32_t key[8];
	uint32_t iv[4];
	uint32_t src[4];
	uint32_t dst[4];

	if (!nvt_crypto_opened) {
		crypto_err("crypto engine not opened!\n");
		return -1;
	}

	/* check parameter */
	if (!p_crypto) {
		crypto_err("invalid parameter\n");
		return -1;
	}

	/* check opmode */
	if (p_crypto->opmode >= NVT_CRYPTO_OPMODE_MAX) {
		crypto_err("opmode:%d invalid\n", p_crypto->opmode);
		return -1;
	}

	/* check iv */
	if ((p_crypto->opmode != NVT_CRYPTO_OPMODE_ECB) && (!p_crypto->iv || (p_crypto->iv_size < NVT_CRYPTO_AES_BLOCK_SIZE))) {
		crypto_err("iv buffer=0x%x size:%u invalid\n", (uint32_t)p_crypto->iv, p_crypto->iv_size);
		return -1;
	}

	/* check source buffer and size */
	if (!p_crypto->src || !p_crypto->src_size || (p_crypto->src_size%NVT_CRYPTO_AES_BLOCK_SIZE)) {
		crypto_err("source buffer=0x%x size:%u invalid\n", (uint32_t)p_crypto->src, p_crypto->src_size);
		return -1;
	}

	/* check destination buffer and size */
	if (!p_crypto->dst || !p_crypto->dst_size || (p_crypto->dst_size < p_crypto->src_size)) {
		crypto_err("destination buffer=0x%x size:%u invalid\n", (uint32_t)p_crypto->dst, p_crypto->dst_size);
		return -1;
	}

	/* check key buffer and size */
	if (p_crypto->key_src == NVT_CRYPTO_KEY_SRC_EFUSE) {
		/* efuse key only support AES128 and key offset must be 0,4,8,12,16 */
		if ((p_crypto->key_size != NVT_CRYPTO_AES_128_KEY_SIZE) || (p_crypto->key_ofs%4)) {
			crypto_err("key efuse size:%u offset:%d invalid\n", p_crypto->key_size, p_crypto->key_ofs);
			return -1;
		}
	}
	else {
		if (!p_crypto->key || ((p_crypto->key_size != NVT_CRYPTO_AES_128_KEY_SIZE) && (p_crypto->key_size != NVT_CRYPTO_AES_256_KEY_SIZE))) {
			crypto_err("key buffer=0x%x size:%u invalid\n", (uint32_t)p_crypto->key, p_crypto->key_size);
			return -1;
		}
	}

	/* check pio mode busy or not */
	if (crypto_read(NVT_CRYPTO_CTRL_REG) & 0x1) {
		crypto_err("crypto PIO mode busy!!\n");
		return -1;
	}

	crypto_dbg("PIO     => AES%d %s Key_Offset:%d OPMode:%d\n", p_crypto->key_size*8, (p_crypto->encrypt ? "Encrypt" : "Decrypt"), p_crypto->key_ofs, p_crypto->opmode);

	/* set IV */
	if (p_crypto->opmode == NVT_CRYPTO_OPMODE_CBC) {
		memcpy(iv, p_crypto->iv, NVT_CRYPTO_AES_BLOCK_SIZE);
		crypto_dbg("IV      => 0x%08x 0x%08x 0x%08x 0x%08x\n", iv[0], iv[1], iv[2], iv[3]);
	}
	else {
		memset(iv, 0, NVT_CRYPTO_AES_BLOCK_SIZE);
	}

	/* set config */
	if (p_crypto->key_size == NVT_CRYPTO_AES_128_KEY_SIZE)
		reg_value = 0x2 | (NVT_CRYPTO_MODE_AES_128<<4);
	else
		reg_value = 0x2 | (NVT_CRYPTO_MODE_AES_256<<4);
	if (!p_crypto->encrypt) {
		reg_value |= (0x1<<8);
	}
	crypto_write(NVT_CRYPTO_CFG_REG, reg_value);

	/* set key */
	if (p_crypto->key_src == NVT_CRYPTO_KEY_SRC_EFUSE) {
		ret = otp_set_key_destination(p_crypto->key_ofs/4);
		if (ret != 0) {
			crypto_err("PIO Key => trigger key from efuse failed!\n");
			ret = -1;
			goto exit;
		}
		crypto_dbg("PIO Key => trigger key from efuse ready\n");
	}
	else {
		memcpy(key, p_crypto->key, p_crypto->key_size);
		crypto_write(NVT_CRYPTO_KEY0_REG, key[0]);
		crypto_write(NVT_CRYPTO_KEY1_REG, key[1]);
		crypto_write(NVT_CRYPTO_KEY2_REG, key[2]);
		crypto_write(NVT_CRYPTO_KEY3_REG, key[3]);
		crypto_dbg("PIO Key => 0x%08x 0x%08x 0x%08x 0x%08x\n", key[0], key[1], key[2], key[3]);

		if (p_crypto->key_size == NVT_CRYPTO_AES_256_KEY_SIZE) {
			crypto_write(NVT_CRYPTO_KEY4_REG, key[4]);
			crypto_write(NVT_CRYPTO_KEY5_REG, key[5]);
			crypto_write(NVT_CRYPTO_KEY6_REG, key[6]);
			crypto_write(NVT_CRYPTO_KEY7_REG, key[7]);
			crypto_dbg("PIO Key => 0x%08x 0x%08x 0x%08x 0x%08x\n", key[4], key[5], key[6], key[7]);
		}
	}

	/* start data process */
	for (i=0; i<(p_crypto->src_size/4); i+=(NVT_CRYPTO_AES_BLOCK_SIZE/4)) {
		/* set input data */
		memcpy(src, &p_crypto->src[i*4], NVT_CRYPTO_AES_BLOCK_SIZE);
		if (p_crypto->opmode == NVT_CRYPTO_OPMODE_CBC) {
			if (p_crypto->encrypt) {
				src[0] ^= iv[0];
				src[1] ^= iv[1];
				src[2] ^= iv[2];
				src[3] ^= iv[3];
			}
		}
		crypto_write(NVT_CRYPTO_IN0_REG, src[0]);
		crypto_write(NVT_CRYPTO_IN1_REG, src[1]);
		crypto_write(NVT_CRYPTO_IN2_REG, src[2]);
		crypto_write(NVT_CRYPTO_IN3_REG, src[3]);
		crypto_dbg("PIO In  => 0x%08x 0x%08x 0x%08x 0x%08x\n", src[0], src[1], src[2], src[3]);

		/* clear PIO status */
		crypto_write(NVT_CRYPTO_INT_STS_REG, 0x1);

		/* trigger PIO mode */
		crypto_write(NVT_CRYPTO_CTRL_REG, 0x1);

		/* polling status */
		cnt = 0;
		reg_value = crypto_read(NVT_CRYPTO_INT_STS_REG);
		while ((reg_value & 0x1) == 0) {
			udelay(2);
			reg_value = crypto_read(NVT_CRYPTO_INT_STS_REG);
			cnt++;
			if ((cnt%500000) == 0)
				crypto_inf("wait crypto data complete...\n");
			if (cnt > 2500000)
				break;
		}

		if (reg_value & 0x1) {
			/* clear status */
			crypto_write(NVT_CRYPTO_INT_STS_REG, 0x1);

			/* read output data */
			dst[0] = crypto_read(NVT_CRYPTO_OUT0_REG);
			dst[1] = crypto_read(NVT_CRYPTO_OUT1_REG);
			dst[2] = crypto_read(NVT_CRYPTO_OUT2_REG);
			dst[3] = crypto_read(NVT_CRYPTO_OUT3_REG);

			if (p_crypto->opmode == NVT_CRYPTO_OPMODE_CBC) {
				if (p_crypto->encrypt) {
					iv[0] = dst[0];
					iv[1] = dst[1];
					iv[2] = dst[2];
					iv[3] = dst[3];
				}
				else {
					dst[0] ^= iv[0];
					dst[1] ^= iv[1];
					dst[2] ^= iv[2];
					dst[3] ^= iv[3];

					iv[0] = src[0];
					iv[1] = src[1];
					iv[2] = src[2];
					iv[3] = src[3];
				}
			}
			memcpy(&p_crypto->dst[i*4], dst, NVT_CRYPTO_AES_BLOCK_SIZE);
			crypto_dbg("PIO Out => 0x%08x 0x%08x 0x%08x 0x%08x\n", dst[0], dst[1], dst[2], dst[3]);
		}
		else {
			crypto_err("crypto PIO mode timeout!!\n");
			ret = -1;
			goto exit;
		}
	}

exit:
	return ret;
}

int nvt_crypto_dma_aes(struct nvt_crypto_pio_t *p_crypto)
{
	int ret = 0;
	int cnt = 0;
	uint32_t reg_value;
	volatile struct nvt_crypto_dma_desc *desc = &g_dma_desc;

	if (!nvt_crypto_opened) {
		crypto_err("crypto engine not opened!\n");
		return -1;
	}

	/* check parameter */
	if (!p_crypto) {
		crypto_err("invalid parameter\n");
		return -1;
	}

	/* check opmode */
	if (p_crypto->opmode >= NVT_CRYPTO_OPMODE_MAX) {
		crypto_err("opmode:%d invalid\n", p_crypto->opmode);
		return -1;
	}

	/* check iv */
	if ((p_crypto->opmode != NVT_CRYPTO_OPMODE_ECB) && (!p_crypto->iv || (p_crypto->iv_size < NVT_CRYPTO_AES_BLOCK_SIZE))) {
		crypto_err("iv buffer=0x%x size:%u invalid\n", (uint32_t)p_crypto->iv, p_crypto->iv_size);
		return -1;
	}

	/* check source buffer and size and alignment, start address must align to 32 for cache line operation */
	if (!p_crypto->src || !p_crypto->src_size || (p_crypto->src_size%NVT_CRYPTO_AES_BLOCK_SIZE) || (((uint32_t)p_crypto->src)&0x1f)) {
		crypto_err("source buffer=0x%x size:%u invalid\n", (uint32_t)p_crypto->src, p_crypto->src_size);
		return -1;
	}

	/* check destination buffer and size, start address must align to 32 for cache line operation */
	if (!p_crypto->dst || !p_crypto->dst_size || (p_crypto->dst_size < p_crypto->src_size) || (((uint32_t)p_crypto->dst)&0x1f)) {
		crypto_err("destination buffer=0x%x size:%u invalid\n", (uint32_t)p_crypto->dst, p_crypto->dst_size);
		return -1;
	}

	/* check key buffer and size */
	if (p_crypto->key_src == NVT_CRYPTO_KEY_SRC_EFUSE) {
		/* efuse key only support AES128 and key offset must be 0,4,8,12,16 */
		if ((p_crypto->key_size != NVT_CRYPTO_AES_128_KEY_SIZE) || (p_crypto->key_ofs%4)) {
			crypto_err("key efuse size:%u offset:%d invalid\n", p_crypto->key_size, p_crypto->key_ofs);
			return -1;
		}
	}
	else {
		if (!p_crypto->key || ((p_crypto->key_size != NVT_CRYPTO_AES_128_KEY_SIZE) && (p_crypto->key_size != NVT_CRYPTO_AES_256_KEY_SIZE))) {
			crypto_err("key buffer=0x%x size:%u invalid\n", (uint32_t)p_crypto->key, p_crypto->key_size);
			return -1;
		}
	}

	/* check dma mode busy or not */
	if (crypto_read(NVT_CRYPTO_CTRL_REG) & (0x1<<4)) {
		crypto_err("crypto DMA mode busy!!\n");
		return -1;
	}

	crypto_dbg("DMA     => AES%d %s Key_Offset:%d OPMode:%d\n", p_crypto->key_size*8, (p_crypto->encrypt ? "Encrypt" : "Decrypt"), p_crypto->key_ofs, p_crypto->opmode);
	crypto_dbg("DMA Src => addr:0x%08x size:%u\n", (uint32_t)p_crypto->src, p_crypto->src_size);
	crypto_dbg("DMA Dst => addr:0x%08x size:%u\n", (uint32_t)p_crypto->dst, p_crypto->dst_size);
	crypto_dbg("DMA DES => addr:0x%08x size:%u\n", (uint32_t)desc, sizeof(struct nvt_crypto_dma_desc));

	/* clear descriptor IV and Key and Counter and CV */
	memset((void *)desc->iv,      0, sizeof(desc->iv));
	memset((void *)desc->key,     0, sizeof(desc->key));
	memset((void *)desc->counter, 0, sizeof(desc->counter));
	memset((void *)desc->cv,      0, sizeof(desc->cv));

	/* set IV */
	if (p_crypto->opmode == NVT_CRYPTO_OPMODE_CBC) {
		memcpy((void *)desc->iv, p_crypto->iv, NVT_CRYPTO_AES_BLOCK_SIZE);
		crypto_dbg("DMA IV  => %08x %08x %08x %08x\n", desc->iv[0], desc->iv[1], desc->iv[2], desc->iv[3]);
	}

	/* set key */
	if (p_crypto->key_src == NVT_CRYPTO_KEY_SRC_EFUSE) {
		ret = otp_set_key_destination(p_crypto->key_ofs/4);
		if (ret != 0) {
			crypto_err("DMA Key => trigger key from efuse failed!\n");
			ret = -1;
			goto exit;
		}
		crypto_dbg("DMA Key => trigger key from efuse ready\n");
	}
	else {
		if (p_crypto->key_size == NVT_CRYPTO_AES_256_KEY_SIZE) {
			memcpy((void *)desc->key, p_crypto->key, NVT_CRYPTO_AES_256_KEY_SIZE);
			crypto_dbg("DMA Key => %08x %08x %08x %08x %08x %08x %08x %08x\n", desc->key[0], desc->key[1], desc->key[2], desc->key[3], desc->key[4], desc->key[5], desc->key[6], desc->key[7]);
		}
		else {
			memcpy((void *)desc->key, p_crypto->key, NVT_CRYPTO_AES_128_KEY_SIZE);
			crypto_dbg("DMA Key => %08x %08x %08x %08x\n", desc->iv[0], desc->iv[1], desc->iv[2], desc->iv[3]);
		}
	}

	/* set header config */
	if (p_crypto->key_size == NVT_CRYPTO_AES_256_KEY_SIZE)
		desc->header_cfg = (NVT_CRYPTO_MODE_AES_256<<4) | (p_crypto->opmode<<8);
	else
		desc->header_cfg = (NVT_CRYPTO_MODE_AES_128<<4) | (p_crypto->opmode<<8);

	if (p_crypto->encrypt == 0)
		desc->header_cfg |= 0x1;        ///< 0:encrypt   1:decrypt

	if (p_crypto->key_src == NVT_CRYPTO_KEY_SRC_EFUSE)
		desc->header_cfg |= (0x1<<12);  ///< 0:from desc 1:from efuse

	/* set crypto block config */
	desc->block[0].src_addr  = (uint32_t)p_crypto->src;
	desc->block[0].dst_addr  = (uint32_t)p_crypto->dst;
	desc->block[0].length    = p_crypto->src_size;
	desc->block[0].block_cfg = 0x1;     /// last block

	/* descriptor buffer memory flush */
	flush_dcache_range((unsigned long)desc, ((unsigned long)desc) + roundup(sizeof(struct nvt_crypto_dma_desc), ARCH_DMA_MINALIGN));

	/* source buffer memory flush */
	flush_dcache_range((unsigned long)p_crypto->src, ((unsigned long)p_crypto->src) + roundup(p_crypto->src_size, ARCH_DMA_MINALIGN));

	/* destination buffer memory invalidate */
	invalidate_dcache_range((unsigned long)p_crypto->dst, ((unsigned long)p_crypto->dst) + roundup(p_crypto->dst_size, ARCH_DMA_MINALIGN));

	/* set crypto enable */
	reg_value = crypto_read(NVT_CRYPTO_CFG_REG);
	crypto_write(NVT_CRYPTO_CFG_REG, (reg_value|0x2));

	/* set interrupt disable */
	reg_value  = crypto_read(NVT_CRYPTO_INT_ENB_REG);
	reg_value &= ~(0xf<<4);
	crypto_write(NVT_CRYPTO_INT_ENB_REG, reg_value);

	/* clear interrupt status */
	crypto_write(NVT_CRYPTO_INT_STS_REG, (0xf<<4));

	/* set DMA descriptor source address */
	crypto_write(NVT_CRYPTO_DMA0_ADDR_REG, (uint32_t)desc);

	/* set DMA channel enable */
	reg_value  = crypto_read(NVT_CRYPTO_CTRL_REG);
	reg_value &= ~(0xf<<4);
	reg_value |= (0x1<<4);
	crypto_write(NVT_CRYPTO_CTRL_REG, reg_value);

	/* polling status */
	reg_value = crypto_read(NVT_CRYPTO_INT_STS_REG);
	while ((reg_value & (0x1<<4)) == 0) {
		udelay(2);
		reg_value = crypto_read(NVT_CRYPTO_INT_STS_REG);
		cnt++;
		if ((cnt%500000) == 0)
			crypto_inf("wait crypto data complete...\n");
		if (cnt > 2500000)
			break;
	}
	if (reg_value & (0x1<<4)) {
		/* clear status */
		crypto_write(NVT_CRYPTO_INT_STS_REG, (0x1<<4));

		crypto_dbg("DMA Out => 0x%08x 0x%08x 0x%08x 0x%08x\n",
				   (p_crypto->dst[0]  | (p_crypto->dst[1]<<8)  | (p_crypto->dst[2]<<16)  | (p_crypto->dst[3]<<24)),
				   (p_crypto->dst[4]  | (p_crypto->dst[5]<<8)  | (p_crypto->dst[6]<<16)  | (p_crypto->dst[7]<<24)),
				   (p_crypto->dst[8]  | (p_crypto->dst[9]<<8)  | (p_crypto->dst[10]<<16) | (p_crypto->dst[11]<<24)),
				   (p_crypto->dst[12] | (p_crypto->dst[13]<<8) | (p_crypto->dst[14]<<16) | (p_crypto->dst[15]<<24)));
	}
	else {
		crypto_err("crypto DMA mode timeout!!\n");
		ret = -1;
		goto exit;
	}

exit:
	return ret;
}
#endif /* CONFIG_NVT_CRYPTO */
