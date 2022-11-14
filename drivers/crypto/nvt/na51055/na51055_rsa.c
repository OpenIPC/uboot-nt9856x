/*
 * RSA API.
 *
 * Support for Novatek NVT RSA Hardware acceleration.
 *
 * Copyright (c) 2020 Novatek Inc.
 *
 */

#ifdef CONFIG_NVT_RSA
#include <common.h>
#include <asm/io.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/nvt_rsa.h>

/*************************************************************************************
 *  IO Read/Write Definition
 *************************************************************************************/
#define IO_REG_RD(_base, _ofs)          readl(_base+_ofs)
#define IO_REG_WR(_base, _ofs, _v)      writel(_v, (_base+_ofs))

/*************************************************************************************
 *  Global Definition
 *************************************************************************************/
//#define NVT_RSA_DEBUG
#define NVT_RSA_OUTPUT_REVERSE          1
#define NVT_RSA_BUFFER_LEN              512     ///< bytes, hardware internal SRAM size, NA51084 = 512, NA51055 = 256

/*************************************************************************************
 *  Debug Message Print Definition
 *************************************************************************************/
#ifdef NVT_RSA_DEBUG
#define rsa_dbg(...)                    printf("[RSA_DBG]: " __VA_ARGS__)
#else
#define rsa_dbg(...)
#endif

#define rsa_err(...)                    printf("[RSA_ERR]: " __VA_ARGS__)
#define rsa_inf(...)                    printf("[RSA_INF]: " __VA_ARGS__)

/*************************************************************************************
 *  Register read/write Definition
 *************************************************************************************/
#define rsa_read(_oft)                 IO_REG_RD(IOADDR_RSA_REG_BASE, _oft)
#define rsa_write(_oft, _v)            IO_REG_WR(IOADDR_RSA_REG_BASE, _oft, _v)

/*************************************************************************************
 *  Register Definition
 *************************************************************************************/
#define NVT_RSA_CFG_REG                0x00           ///< configuration
#define NVT_RSA_CTRL_REG               0x04           ///< control
#define NVT_RSA_INT_ENB_REG            0x08           ///< interrupt enable
#define NVT_RSA_INT_STS_REG            0x0C           ///< interrupt status
#define NVT_RSA_KEY_N_REG              0x10           ///< key N register
#define NVT_RSA_KEY_N_ADDR_REG         0x14           ///< key N config ram address
#define NVT_RSA_KEY_ED_REG             0x18           ///< key E/D register
#define NVT_RSA_KEY_ED_ADDR_REG        0x1C           ///< key E/D config ram address
#define NVT_RSA_DATA_REG               0x20           ///< input data register
#define NVT_RSA_DATA_ADDR_REG          0x24           ///< input data config ram address
#define NVT_RSA_KEY_READ_REG           0x28           ///< rsa key readable control
#define NVT_RSA_CRC32_DEFAULT_REG      0x30           ///< crc32 default value
#define NVT_RSA_CRC32_POLY_REG         0x34           ///< crc32 polynomial parameter
#define NVT_RSA_CRC32_OUTPUT_REG       0x38           ///< rsa key crc32 result

/*************************************************************************************
 *  Local Definition
 *************************************************************************************/
static int nvt_rsa_opened = 0;

static void nvt_rsa_platform_init(void)
{
	uint32_t tmp;

	/* RSA clock select, 0:240MHz 1:320MHz 2: RVD 3: PLL9 */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x1C);
	tmp &= ~(0x3<<22);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x1C, tmp);

	/* RSA clock enable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x78);
	tmp |= (0x1<<26);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x78, tmp);

	/* RSA reset disable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x88);
	tmp |= (0x1<<26);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x88, tmp);

	/* RSA SRAM shutdown disable */
	tmp = IO_REG_RD(IOADDR_TOP_REG_BASE, 0x1000);
	tmp &= ~(0x1<<25);
	IO_REG_WR(IOADDR_TOP_REG_BASE, 0x1000, tmp);
}

static void nvt_rsa_platform_exit(void)
{
	uint32_t tmp;

	/* RSA SRAM shutdown enable */
	tmp = IO_REG_RD(IOADDR_TOP_REG_BASE, 0x1000);
	tmp |= (0x1<<25);
	IO_REG_WR(IOADDR_TOP_REG_BASE, 0x1000, tmp);

	/* RSA reset enable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x88);
	tmp &= ~(0x1<<26);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x88, tmp);

	/* RSA clock disable */
	tmp = IO_REG_RD(IOADDR_CG_REG_BASE, 0x78);
	tmp &= ~(0x1<<26);
	IO_REG_WR(IOADDR_CG_REG_BASE, 0x78, tmp);
}

static void nvt_rsa_reset(void)
{
	uint32_t value = 0;
	uint32_t cnt   = 0;

	/* disable rsa */
	rsa_write(NVT_RSA_CTRL_REG, 0);
	rsa_write(NVT_RSA_CFG_REG,  0);

	/* set reset, hardware will auto clear */
	rsa_write(NVT_RSA_CFG_REG, 0x01);

	/* check reset done */
	while ((value = rsa_read(NVT_RSA_CFG_REG)) & 0x1) {
		if(cnt++ >= 3000000)    ///< 3sec
			break;
		udelay(1);
	}

	/* clear status */
	rsa_write(NVT_RSA_INT_STS_REG, 0x1);

	if (value & 0x1) {
		rsa_err("rsa hardware reset failed!!\n");
	}
}

int nvt_rsa_open(void)
{
	if (!nvt_rsa_opened) {
		/* platform pmu init for rsa engine */
		nvt_rsa_platform_init();

		/* rsa engine software reset */
		nvt_rsa_reset();

		nvt_rsa_opened = 1;
	}

	return 0;
}

void nvt_rsa_close(void)
{
	if (nvt_rsa_opened) {
		nvt_rsa_platform_exit();
		nvt_rsa_opened = 0;
	}
}

int nvt_rsa_pio_normal(struct nvt_rsa_pio_t *p_rsa)
{
	int    i, j;
	int    buf_len;
	uint32_t cnt;
	uint32_t block_size;
	uint32_t reg_value;
	uint32_t chip_id;

	if (!nvt_rsa_opened) {
		rsa_err("rsa engine not opened!\n");
		return -1;
	}

	/* check parameter */
	if (!p_rsa) {
		rsa_err("invalid parameter\n");
		return -1;
	}

	/* get chip id for feature limitation */
	chip_id = IO_REG_RD(IOADDR_TOP_REG_BASE, 0xf0);

	/* RSA internal SRAM buffer length */
	buf_len = ((chip_id & 0xffff0000) == 0x50210000) ? NVT_RSA_BUFFER_LEN : (NVT_RSA_BUFFER_LEN>>1);

	rsa_dbg("RSA       => key%d\n", 256<<p_rsa->key_w);
	rsa_dbg("Src       => addr:0x%08x size:%u\n", (uint32_t)p_rsa->src,    p_rsa->src_size);
	rsa_dbg("Dst       => addr:0x%08x size:%u\n", (uint32_t)p_rsa->dst,    p_rsa->dst_size);
	rsa_dbg("Key_N     => addr:0x%08x size:%u\n", (uint32_t)p_rsa->key_n,  p_rsa->key_n_size);
	rsa_dbg("Key_ED    => addr:0x%08x size:%u\n", (uint32_t)p_rsa->key_ed, p_rsa->key_ed_size);

	/* check key width */
	switch (p_rsa->key_w) {
	case NVT_RSA_KEY_WIDTH_256:
		block_size = NVT_RSA_256_KEY_SIZE;
		break;
	case NVT_RSA_KEY_WIDTH_512:
		block_size = NVT_RSA_512_KEY_SIZE;
		break;
	case NVT_RSA_KEY_WIDTH_1024:
		block_size = NVT_RSA_1024_KEY_SIZE;
		break;
	case NVT_RSA_KEY_WIDTH_2048:
		block_size = NVT_RSA_2048_KEY_SIZE;
		break;
	case NVT_RSA_KEY_WIDTH_4096:
		if (buf_len != NVT_RSA_BUFFER_LEN) {
			rsa_err("key width=%d not support (CHIP_ID=0x%08x)\n", 256<<p_rsa->key_w, chip_id);
			return -1;
		}
		block_size = NVT_RSA_4096_KEY_SIZE;
		break;
	default:
		rsa_err("key width=%d not support\n", 256<<p_rsa->key_w);
		return -1;
	}

	/* check source buffer and size */
	if (!p_rsa->src || !p_rsa->src_size || (p_rsa->src_size > block_size)) {
		rsa_err("source buffer=0x%08x size:%u invalid\n", (uint32_t)p_rsa->src, p_rsa->src_size);
		return -1;
	}

	/* check destination buffer and size */
	if (!p_rsa->dst || !p_rsa->dst_size || (p_rsa->dst_size < block_size)) {
		rsa_err("destination buffer=0x%08x size:%u invalid\n", (uint32_t)p_rsa->dst, p_rsa->dst_size);
		return -1;
	}

	/* check key_n buffer and size */
	if (!p_rsa->key_n || !p_rsa->key_n_size || (p_rsa->key_n_size > block_size)) {
		rsa_err("key_n buffer=0x%08x size:%u invalid\n", (uint32_t)p_rsa->key_n, p_rsa->key_n_size);
		return -1;
	}

	/* check key_ed buffer and size */
	if (!p_rsa->key_ed || !p_rsa->key_ed_size || (p_rsa->key_ed_size > block_size)) {
		rsa_err("key_ed buffer=0x%08x size:%u invalid\n", (uint32_t)p_rsa->key_ed, p_rsa->key_ed_size);
		return -1;
	}

	/* check pio busy */
	if (rsa_read(NVT_RSA_INT_STS_REG) & 0x2) {
		rsa_err("rsa PIO mode busy!!\n");
		return -1;
	}

	/*
	 *  RSA Input data byte order => big endian
	 *  [0000] 0F 0E 0D 0C 0B 0A 09 08 07 06 05 04 03 02 01 00
	 *
	 *  RSA SRAM data byte order => little endian
	 *  [0000] 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
	 */

	/* set input data */
	rsa_write(NVT_RSA_DATA_ADDR_REG, 0);
	for (i=0; i<buf_len; i+=4) {
		if (i < p_rsa->src_size) {
			reg_value = 0;
			for (j=0; j<4; j++) {
				reg_value |= (((uint32_t)(((i+j) >= p_rsa->src_size) ? 0 : p_rsa->src[p_rsa->src_size-(i+j)-1]))<<(j*8));
			}
			rsa_write(NVT_RSA_DATA_REG, reg_value);
			rsa_dbg("RSA In    => 0x%08x\n", reg_value);
		}
		else {
			rsa_write(NVT_RSA_DATA_REG, 0);
			rsa_dbg("RSA In    => 0x%08x\n", 0);
		}
	}

	/* set key_n */
	rsa_write(NVT_RSA_KEY_N_ADDR_REG, 0);
	for (i=0; i<buf_len; i+=4) {
		if (i < p_rsa->key_n_size) {
			reg_value = 0;
			for (j=0; j<4; j++) {
				reg_value |= (((uint32_t)(((i+j) >= p_rsa->key_n_size) ? 0 : p_rsa->key_n[p_rsa->key_n_size-(i+j)-1]))<<(j*8));
			}
			rsa_write(NVT_RSA_KEY_N_REG, reg_value);
			rsa_dbg("RSA Key_N => 0x%08x\n", reg_value);
		}
		else {
			rsa_write(NVT_RSA_KEY_N_REG, 0);
			rsa_dbg("RSA Key_N => 0x%08x\n", 0);
		}
	}

	/* set key_ed */
	rsa_write(NVT_RSA_KEY_ED_ADDR_REG, 0);
	for (i=0; i<buf_len; i+=4) {
		if (i < p_rsa->key_ed_size) {
			reg_value = 0;
			for (j=0; j<4; j++) {
				reg_value |= (((uint32_t)(((i+j) >= p_rsa->key_ed_size) ? 0 : p_rsa->key_ed[p_rsa->key_ed_size-(i+j)-1]))<<(j*8));
			}
			rsa_write(NVT_RSA_KEY_ED_REG, reg_value);
			rsa_dbg("RSA Key_ED=> 0x%08x\n", reg_value);
		}
		else {
			rsa_write(NVT_RSA_KEY_ED_REG, 0);
			rsa_dbg("RSA Key_ED=> 0x%08x\n", 0);
		}
	}

	/* set key width and mode */
	rsa_write(NVT_RSA_CFG_REG, ((NVT_RSA_MODE_NORMAL<<4) | (p_rsa->key_w<<1)));

	/* clear status */
	rsa_write(NVT_RSA_INT_STS_REG, 0x1);

	/* trigger transfer */
	rsa_write(NVT_RSA_CTRL_REG, 0x1);

	/* polling status */
	cnt = 0;
	reg_value = rsa_read(NVT_RSA_INT_STS_REG);
	while ((reg_value & 0x1) == 0) {
		udelay(2);
		reg_value = rsa_read(NVT_RSA_INT_STS_REG);
		cnt++;
		if ((cnt%500000) == 0)
			rsa_inf("wait rsa data complete...\n");
		if (cnt > 2500000)
			break;
	}

	/* copy output data to destination buffer */
	if (reg_value & 0x1) {
		rsa_write(NVT_RSA_DATA_ADDR_REG, 0);
		for (i=0; i<block_size; i+=4) {
			reg_value = rsa_read(NVT_RSA_DATA_REG);
			for (j=0; j<4; j++) {
#ifdef NVT_RSA_OUTPUT_REVERSE
				p_rsa->dst[block_size-(i+j)-1] = (reg_value>>(j*8))&0xff;    ///< data reverse
#else
				p_rsa->dst[i+j] = (reg_value>>(j*8))&0xff;
#endif
			}
			rsa_dbg("RSA Out   => 0x%08x\n", reg_value);
		}
	}
	else {
		rsa_err("rsa PIO mode timeout!!\n");
		return -1;
	}

	return 0;
}
#endif /* CONFIG_NVT_RSA */
