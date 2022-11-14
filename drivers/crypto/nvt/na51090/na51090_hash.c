/*
 * Hash API.
 *
 * Support for Novatek NVT Hash Hardware acceleration.
 *
 * Copyright (c) 2021 Novatek Inc.
 *
 */

#ifdef CONFIG_NVT_HASH
#include <common.h>
#include <asm/io.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/nvt_hash.h>

/*************************************************************************************
 *  IO Read/Write Definition
 *************************************************************************************/
#define IO_REG_RD(_base, _ofs)          readl(_base+_ofs)
#define IO_REG_WR(_base, _ofs, _v)      writel(_v, (_base+_ofs))

/*************************************************************************************
 *  Global Definition
 *************************************************************************************/
//#define NVT_HASH_DEBUG
#define NVT_HASH_DMA_36BIT_SUPPORT      1

#define PLAT_CACHE_LINE_SIZE            CONFIG_SYS_CACHELINE_SIZE
#define PLAT_CACHE_LINE_MASK            (CONFIG_SYS_CACHELINE_SIZE-1)
#define DMA_ADDR_LO(x)                  ((uint32_t)((uint64_t)(x)))
#define DMA_ADDR_HI(x)                  ((uint32_t)(((uint64_t)(x)) >> 32))

#define IV_BYTE_REVERSE(x)              ((((x)&0xff)<<24) | ((((x)>>8)&0xff)<<16) | ((((x)>>16)&0xff)<<8) | (((x)>>24)&0xff))

/*************************************************************************************
 *  Debug Message Print Definition
 *************************************************************************************/
#ifdef NVT_HASH_DEBUG
#define hash_dbg(...)                   printf("[HASH_DBG]: " __VA_ARGS__)
#else
#define hash_dbg(...)
#endif

#define hash_err(...)                   printf("[HASH_ERR]: " __VA_ARGS__)
#define hash_inf(...)                   printf("[HASH_INF]: " __VA_ARGS__)

/*************************************************************************************
 *  Register read/write Definition
 *************************************************************************************/
#define hash_read(_oft)                 IO_REG_RD(IOADDR_HASH_REG_BASE, _oft)
#define hash_write(_oft, _v)            IO_REG_WR(IOADDR_HASH_REG_BASE, _oft, _v)

/*************************************************************************************
 *  Register Definition
 *************************************************************************************/
#define NVT_HASH_CFG_REG                0x00           ///< configuration
#define NVT_HASH_PADLEN_REG             0x04           ///< message length for DMA mode auto padding used
#define NVT_HASH_INT_ENB_REG            0x08           ///< interrupt enable
#define NVT_HASH_INT_STS_REG            0x0C           ///< interrupt status
#define NVT_HASH_KEY0_REG               0x10           ///< key 0   ~ 31  bit
#define NVT_HASH_KEY1_REG               0x14           ///< key 32  ~ 63  bit
#define NVT_HASH_KEY2_REG               0x18           ///< key 64  ~ 95  bit
#define NVT_HASH_KEY3_REG               0x1C           ///< key 96  ~ 127 bit
#define NVT_HASH_KEY4_REG               0x20           ///< key 128 ~ 159 bit
#define NVT_HASH_KEY5_REG               0x24           ///< key 160 ~ 191 bit
#define NVT_HASH_KEY6_REG               0x28           ///< key 192 ~ 223 bit
#define NVT_HASH_KEY7_REG               0x2C           ///< key 224 ~ 255 bit
#define NVT_HASH_KEY8_REG               0x30           ///< key 256 ~ 287 bit
#define NVT_HASH_KEY9_REG               0x34           ///< key 288 ~ 319 bit
#define NVT_HASH_KEY10_REG              0x38           ///< key 320 ~ 351 bit
#define NVT_HASH_KEY11_REG              0x3C           ///< key 352 ~ 383 bit
#define NVT_HASH_KEY12_REG              0x40           ///< key 384 ~ 415 bit
#define NVT_HASH_KEY13_REG              0x44           ///< key 416 ~ 447 bit
#define NVT_HASH_KEY14_REG              0x48           ///< key 448 ~ 479 bit
#define NVT_HASH_KEY15_REG              0x4C           ///< key 480 ~ 511 bit
#define NVT_HASH_IV0_REG                0x50           ///< initial vector 0   ~ 31  bit
#define NVT_HASH_IV1_REG                0x54           ///< initial vector 32  ~ 63  bit
#define NVT_HASH_IV2_REG                0x58           ///< initial vector 64  ~ 95  bit
#define NVT_HASH_IV3_REG                0x5c           ///< initial vector 96  ~ 127 bit
#define NVT_HASH_IV4_REG                0x60           ///< initial vector 128 ~ 159 bit
#define NVT_HASH_IV5_REG                0x64           ///< initial vector 160 ~ 191 bit
#define NVT_HASH_IV6_REG                0x68           ///< initial vector 192 ~ 223 bit
#define NVT_HASH_IV7_REG                0x6c           ///< initial vector 224 ~ 255 bit
#define NVT_HASH_OUT0_REG               0x70           ///< output data  0   ~ 31  bit
#define NVT_HASH_OUT1_REG               0x74           ///< output data  32  ~ 63  bit
#define NVT_HASH_OUT2_REG               0x78           ///< output data  64  ~ 95  bit
#define NVT_HASH_OUT3_REG               0x7c           ///< output data  96  ~ 127 bit
#define NVT_HASH_OUT4_REG               0x80           ///< output data  128 ~ 159 bit
#define NVT_HASH_OUT5_REG               0x84           ///< output data  160 ~ 191 bit
#define NVT_HASH_OUT6_REG               0x88           ///< output data  192 ~ 223 bit
#define NVT_HASH_OUT7_REG               0x8c           ///< output data  224 ~ 255 bit
#define NVT_HASH_PIO_IN_REG             0x90           ///< input  data  0   ~ 31  bit, PIO mode
#define NVT_HASH_DMA_SRC_REG            0x94           ///< source dma low address, word alignment
#define NVT_HASH_DMA_DST_REG            0x98           ///< destination low dma address, word alignment
#define NVT_HASH_DMA_TX_SIZE_REG        0x9c           ///< hash message size, byte unit and alignment
#define NVT_HASH_KEY_READ_REG           0xA0           ///< hash key readable control
#define NVT_HASH_DMA_SRC_H_REG          0xA4           ///< source dma high address
#define NVT_HASH_DMA_DST_H_REG          0xA8           ///< destination dma high address

/*************************************************************************************
 *  Local Definition
 *************************************************************************************/
static int nvt_hash_opened = 0;

static void nvt_hash_platform_init(void)
{
	uint32_t tmp;

	/* Hash clock select, 0:PLL2(400MHz) 1:PLL13(350MHz) */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x24);
	tmp &= ~(0x1<<8);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x24, tmp);

	/* Hash master clock enable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x70);
	tmp |= (0x1<<29);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x70, tmp);

	/* Hash program clock enable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0xE0);
	tmp |= (0x1<<17);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0xE0, tmp);

	/* Hash reset disable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x94);
	tmp |= (0x1<<20);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x94, tmp);
}

static void nvt_hash_platform_exit(void)
{
	uint32_t tmp;

	/* Hash reset enable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x94);
	tmp &= ~(0x1<<20);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x94, tmp);

	/* Hash master clock disable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x70);
	tmp &= ~(0x1<<29);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x70, tmp);
}

static void nvt_hash_reset(void)
{
	uint32_t value = 0;
	uint32_t cnt   = 0;

	/* disable hash */
	hash_write(NVT_HASH_CFG_REG, 0);

	/* set reset, hardware will auto clear */
	hash_write(NVT_HASH_CFG_REG, 0x01);

	/* check reset done */
	while ((value = hash_read(NVT_HASH_CFG_REG)) & 0x1) {
		if(cnt++ >= 3000000)    ///< 3sec
			break;
		udelay(1);
	}

	/* clear all status */
	hash_write(NVT_HASH_INT_STS_REG, 0x3);

	if (value & 0x1) {
		hash_err("hash hardware reset failed!!\n");
	}
}

int nvt_hash_open(void)
{
	if (!nvt_hash_opened) {
		/* platform pmu init for hash engine */
		nvt_hash_platform_init();

		/* hash engine software reset */
		nvt_hash_reset();

		nvt_hash_opened = 1;
	}

	return 0;
}

void nvt_hash_close(void)
{
	if (nvt_hash_opened) {
		nvt_hash_platform_exit();
		nvt_hash_opened = 0;
	}
}

int nvt_hash_pio_sha(struct nvt_hash_pio_t *p_hash)
{
	int    ret = 0;
	uint32_t cnt;
	uint32_t i, j, index;
	uint64_t len_bits;
	uint32_t padlen;
	uint32_t remain, out_size;
	uint32_t reg_value;
	uint32_t total_blks;
	uint8_t  sha_padding[NVT_HASH_SHA256_BLOCK_SIZE+8];

	if (!nvt_hash_opened) {
		hash_err("hash engine not opened!\n");
		return -1;
	}

	/* check parameter */
	if (!p_hash) {
		hash_err("invalid parameter\n");
		return -1;
	}

	/* check hash mode */
	switch (p_hash->mode) {
	case NVT_HASH_MODE_SHA1:
		out_size = NVT_HASH_SHA1_DIGEST_SIZE;
		break;
	case NVT_HASH_MODE_SHA256:
		out_size = NVT_HASH_SHA256_DIGEST_SIZE;
		break;
	default:
		hash_err("hash mode=%d invalid\n", p_hash->mode);
		return -1;
	}

	hash_dbg("Hash    => %s PIO Mode\n", (p_hash->mode ? "SHA256" : "SHA1"));
	hash_dbg("Src     => addr:0x%08lx size:%u\n", (uintptr_t)p_hash->src,    p_hash->src_size);
	hash_dbg("Digest  => addr:0x%08lx size:%u\n", (uintptr_t)p_hash->digest, p_hash->digest_size);

	/* check source buffer and size, the zero source size is valid */
	if (!p_hash->src && (p_hash->src_size > 0)) {
		hash_err("source buffer=0x%08lx size:%u invalid\n", (uintptr_t)p_hash->src, p_hash->src_size);
		return -1;
	}

	/* check digest buffer and size */
	if (!p_hash->digest || (p_hash->digest_size < out_size)) {
		hash_err("digest buffer=0x%08lx size:%u invalid\n", (uintptr_t)p_hash->digest, p_hash->digest_size);
		return -1;
	}

	/* check pio mode busy or not */
	if (hash_read(NVT_HASH_CFG_REG) & 0x2) {
		hash_err("hash PIO mode busy!!\n");
		return -1;
	}

	/*
	 * The purpose of this padding is to ensure that the padded message is a
	 * multiple of 512 bits (SHA1/SHA256).
	 * The bit "1" is appended at the end of the message followed by
	 * "padlen-1" zero bits. Then a 64 bits block (SHA1/SHA256) equals
	 * to the message length in bits is appended.
	 *
	 * For SHA1/SHA256, padlen is calculated as followed:
	 *  - if message length < 56 bytes then padlen = 56 - message length
	 *  - else padlen = 64 + 56 - message length
	 *
	 */
	len_bits = ((uint64_t)p_hash->src_size)<<3;
	remain   = p_hash->src_size%NVT_HASH_SHA256_BLOCK_SIZE;
	padlen   = (remain < 56) ? (56 - remain) : ((64 + 56) - remain);
	sha_padding[0] = 0x80;
	if (padlen > 1) {
		memset(&sha_padding[1], 0, padlen-1);
	}
	for (i=0; i<8; i++) {
		sha_padding[padlen+i] = ((unsigned char *)&len_bits)[7-i]; ///< byte order to little endian
	}

	/* set config */
	if (p_hash->mode)
		hash_write(NVT_HASH_CFG_REG, (NVT_HASH_MODE_SHA256<<4));
	else
		hash_write(NVT_HASH_CFG_REG, (NVT_HASH_MODE_SHA1<<4));

	/* clear status */
	hash_write(NVT_HASH_INT_STS_REG, 0x1);

	/* start hash data */
	total_blks = (p_hash->src_size + padlen + 8)/NVT_HASH_SHA256_BLOCK_SIZE;
	for (i=0; i<total_blks; i++) {
		/* set state and enable hash */
		reg_value  = hash_read(NVT_HASH_CFG_REG);
		reg_value &= ~((0x1<<20)|(0x1<<1));
		if (i == 0)
			reg_value |= ((0x1<<20)|(0x1<<1));  ///< enable hash and enable initial state
		else
			reg_value |= (0x1<<1);              ///< enable hash and disable initial state
		hash_write(NVT_HASH_CFG_REG, reg_value);

		/* set block data (64 bytes) */
		for(j=0; j<NVT_HASH_SHA256_BLOCK_SIZE; j+=4) {
			index = i*NVT_HASH_SHA256_BLOCK_SIZE + j;

			reg_value  = ((index >= p_hash->src_size) ? sha_padding[index - p_hash->src_size] : p_hash->src[index]);
			index++;
			reg_value |= ((uint32_t)((index >= p_hash->src_size) ? sha_padding[index - p_hash->src_size] : p_hash->src[index]))<<8;
			index++;
			reg_value |= ((uint32_t)((index >= p_hash->src_size) ? sha_padding[index - p_hash->src_size] : p_hash->src[index]))<<16;
			index++;
			reg_value |= ((uint32_t)((index >= p_hash->src_size) ? sha_padding[index - p_hash->src_size] : p_hash->src[index]))<<24;

			hash_write(NVT_HASH_PIO_IN_REG, reg_value);

			hash_dbg("Hash In => 0x%08x\n", reg_value);
		}

		/* polling status */
		cnt = 0;
		reg_value = hash_read(NVT_HASH_INT_STS_REG);
		while ((reg_value & 0x1) == 0) {
			udelay(2);
			reg_value = hash_read(NVT_HASH_INT_STS_REG);
			cnt++;
			if ((cnt%500000) == 0)
				hash_inf("wait hash data complete...\n");
			if (cnt > 2500000)
				break;
		}
		if ((reg_value & 0x1) == 0) {
			hash_err("hash PIO mode timeout!!\n");
			ret = -1;
			goto exit;
		}
	}

	/* delay to wait result update to register after hash transfer done */
	udelay(3);

	/* get hash digest result */
	for (i=0; i<out_size; i+=4) {
		reg_value = hash_read(NVT_HASH_OUT0_REG+i);
		memcpy(&p_hash->digest[i], &reg_value, 4);
		hash_dbg("Hash Out=> 0x%08x\n", reg_value);
	}

exit:
	return ret;
}

int nvt_hash_dma_sha(struct nvt_hash_pio_t *p_hash)
{
	int      i, ret = 0;
	uint32_t cnt = 0;
	uint32_t reg_value;
	uint32_t out_size;

	if (!nvt_hash_opened) {
		hash_err("hash engine not opened!\n");
		return -1;
	}

	/* check parameter */
	if (!p_hash) {
		hash_err("invalid parameter\n");
		return -1;
	}

	/* check hash mode */
	switch (p_hash->mode) {
	case NVT_HASH_MODE_SHA1:
		out_size = NVT_HASH_SHA1_DIGEST_SIZE;
		break;
	case NVT_HASH_MODE_SHA256:
		out_size = NVT_HASH_SHA256_DIGEST_SIZE;
		break;
	default:
		hash_err("hash mode=%d invalid\n", p_hash->mode);
		return -1;
	}

	hash_dbg("Hash    => %s DMA Mode\n", (p_hash->mode ? "SHA256" : "SHA1"));
	hash_dbg("Src     => addr:0x%08lx size:%u\n", (uintptr_t)p_hash->src,    p_hash->src_size);
	hash_dbg("Digest  => addr:0x%08lx size:%u\n", (uintptr_t)p_hash->digest, p_hash->digest_size);

	/* check source buffer and size, the zero source size is valid */
	if ((!p_hash->src && (p_hash->src_size > 0)) || (p_hash->src && (((uintptr_t)p_hash->src)&PLAT_CACHE_LINE_MASK))) {
		hash_err("source buffer=0x%08lx size:%u invalid\n", (uintptr_t)p_hash->src, p_hash->src_size);
		return -1;
	}

	/* check digest buffer and size */
	if (!p_hash->digest || (p_hash->digest_size < out_size)) {
		hash_err("digest buffer=0x%08lx size:%u invalid\n", (uintptr_t)p_hash->digest, p_hash->digest_size);
		return -1;
	}

	/* check hardware busy or not */
	if (hash_read(NVT_HASH_CFG_REG) & 0x2) {
		hash_err("hash DMA mode busy!!\n");
		return -1;
	}

	/* source buffer memory flush */
	flush_dcache_range((unsigned long)p_hash->src, ((unsigned long)p_hash->src) + roundup(p_hash->src_size, ARCH_DMA_MINALIGN));

	/* set total hash length for hardware auto padding on last block */
	hash_write(NVT_HASH_PADLEN_REG, p_hash->src_size);

	/* set DMA address and length */
	hash_write(NVT_HASH_DMA_SRC_REG,   DMA_ADDR_LO(p_hash->src));
#ifdef NVT_HASH_DMA_36BIT_SUPPORT
    hash_write(NVT_HASH_DMA_SRC_H_REG, DMA_ADDR_HI(p_hash->src));
#endif
	hash_write(NVT_HASH_DMA_TX_SIZE_REG, p_hash->src_size);

	/* clear DMA interrupt status */
	hash_write(NVT_HASH_INT_STS_REG, 0x1);

	/* disable DMA interrupt mask */
	hash_write(NVT_HASH_INT_ENB_REG, 0);

	/* set config and trigger DMA, use default IV and enable hardware auto padding for last block */
	reg_value = 0x2 | (p_hash->mode<<4)	| (0x1<<12) | (0x1<<13);
	hash_write(NVT_HASH_CFG_REG, reg_value);

	/* polling status */
	reg_value = hash_read(NVT_HASH_INT_STS_REG);
	while ((reg_value & 0x1) == 0) {
		udelay(2);
		reg_value = hash_read(NVT_HASH_INT_STS_REG);
		cnt++;
		if ((cnt%500000) == 0)
			hash_inf("wait hash data complete...\n");
		if (cnt > 2500000)
			break;
	}
	if ((reg_value & 0x1) == 0) {
		hash_err("hash DMA mode timeout!!\n");
		ret = -1;
		goto exit;
	}

	/* delay to wait result update to register after hash transfer done */
	udelay(3);

	/* get hash digest result */
	for (i=0; i<out_size; i+=4) {
		reg_value = hash_read(NVT_HASH_OUT0_REG+i);
		memcpy(&p_hash->digest[i], &reg_value, 4);
		hash_dbg("Hash Out=> 0x%08x\n", reg_value);
	}

exit:
	return ret;
}
#endif  /* CONFIG_NVT_HASH */
