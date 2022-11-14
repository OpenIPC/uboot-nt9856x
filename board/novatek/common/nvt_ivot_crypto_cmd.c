/*
 * Copyright (c) 2021 Novatek Inc.
 *
 */

#include <common.h>
#include <memalign.h>
#include <asm/arch/nvt_crypto.h>

DECLARE_GLOBAL_DATA_PTR;

#define _ALIGNED(x)         __attribute__((aligned(x)))
#define CACHE_LINE_SIZE     CONFIG_SYS_CACHELINE_SIZE

static uint8_t AES_IV[] = {
	0x0B, 0x7C, 0xFC, 0x71, 0x08, 0x11, 0xC2, 0x21, 0x6F, 0x4F, 0xAC, 0xBA, 0xBF, 0x4F, 0x28, 0x8B
};

static uint8_t AES_KEY[] = {
	0x31, 0xD1, 0x32, 0x61, 0xBB, 0xA9, 0x28, 0xFD, 0xF6, 0x9D, 0x8D, 0xD3, 0xFB, 0xE1, 0x23, 0xEC,
	0x6A, 0xAD, 0x01, 0x7D, 0x1C, 0xFC, 0xE9, 0x36, 0x05, 0x9C, 0x60, 0x1A, 0x33, 0xA5, 0x14, 0x09
};

static uint8_t _ALIGNED(CACHE_LINE_SIZE) AES_PLAINTEXT[] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
};

static uint8_t _ALIGNED(CACHE_LINE_SIZE) AES128_ECB_CIPHERTEXT[] = {
	0xCB, 0x4E, 0x39, 0x2D, 0xAC, 0x11, 0x40, 0xFB, 0x4E, 0xD6, 0x31, 0xCD, 0xCC, 0x2C, 0x21, 0x7B,
	0x30, 0xCF, 0xCF, 0xB5, 0xAB, 0xFD, 0x1B, 0x99, 0x19, 0xB6, 0x9F, 0x1A, 0x37, 0x51, 0x5A, 0xCA
};

static uint8_t _ALIGNED(CACHE_LINE_SIZE) AES128_CBC_CIPHERTEXT[] = {
	0x83, 0xC3, 0xEF, 0x24, 0xE4, 0xD4, 0x19, 0xEE, 0x64, 0x3E, 0x8C, 0xA3, 0x98, 0x16, 0x05, 0x20,
	0xBA, 0x8D, 0x5F, 0x5D, 0x13, 0xBA, 0x30, 0x5F, 0xC5, 0x4F, 0x70, 0x5C, 0x89, 0x3C, 0x3A, 0x5A
};

static uint8_t _ALIGNED(CACHE_LINE_SIZE) AES256_ECB_CIPHERTEXT[] = {
	0xC6, 0xE6, 0x0C, 0xC6, 0xF1, 0x3D, 0x6C, 0x73, 0x93, 0x5A, 0x80, 0x92, 0x12, 0x14, 0x0F, 0xC5,
	0xA0, 0x07, 0xDB, 0xFD, 0x63, 0xD3, 0xD2, 0x5E, 0x0B, 0x08, 0xD0, 0xD3, 0x6C, 0xA7, 0x23, 0xC3
};

static uint8_t _ALIGNED(CACHE_LINE_SIZE) AES256_CBC_CIPHERTEXT[] = {
	0x3A, 0xBD, 0xEB, 0x54, 0xAA, 0x71, 0x78, 0xC8, 0xC2, 0x5E, 0xC4, 0x88, 0xA5, 0x63, 0x35, 0xBB,
	0x96, 0x51, 0x5B, 0x8C, 0x84, 0x5C, 0x56, 0x1B, 0x1E, 0x94, 0x1F, 0xF3, 0x1E, 0x0B, 0x13, 0x4B
};

#ifdef CONFIG_NVT_FPGA_EMULATION
/* efuse test key */
static uint32_t EFUSE_TEST_KEY[20] = {
    0x01020304, 0x05060708, 0x09101112, 0x13141516,     ///< Key#0
    0x9b0daa24, 0xb431aef1, 0xc4e45128, 0x1e1d71d1,     ///< Key#1
    0xc5814d95, 0xe52bcc5a, 0xc374c8dd, 0x5ccfaf9f,     ///< Key#2
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     ///< Key#3
    0x00000000, 0x00000000, 0x00000000, 0x00000000      ///< Key#4
};
#endif

#define EFUSE_KEY_0_OFFSET      0
#define EFUSE_KEY_1_OFFSET      4
#define EFUSE_KEY_2_OFFSET      8
#define EFUSE_KEY_3_OFFSET      12
#define EFUSE_KEY_4_OFFSET      16

/******************************************************************
 NA51090 efuse key section 640bit => 20 word
 =============================================================
 key#   offset  word[0]  word[1]  word[2]  word[3]
 =============================================================
 key#0  [ 0]   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  => 128bit
 key#1  [ 4]   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  => 128bit
 key#2  [ 8]   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  => 128bit
 key#3  [12]   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  => 128bit
 key#4  [16]   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  => 128bit
*******************************************************************/

/**
 * do_nvt_crypto() - Handle the "nvt_crypto" command-line command
 * @cmdtp:	Command data struct pointer
 * @flag:	Command flag
 * @argc:	Command-line argument count
 * @argv:	Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_nvt_crypto(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0;
	int key_size;
	int opmode;
	int dma_mode;
	NVT_CRYPTO_KEY_SRC_T key_src;
	uint32_t key_ofs;
	uint8_t *ciphertext;
	uint8_t _ALIGNED(CACHE_LINE_SIZE) dst_buf[32];   ///< start address must alignment to 64 for cache operation in dma mode
	struct nvt_crypto_pio_t crypto;

	/* Check argument */
	if (argc < 6)
		return CMD_RET_USAGE;

	/* Key Length */
	if (!strncmp(argv[1], "aes128", 6))
		key_size = NVT_CRYPTO_AES_128_KEY_SIZE;
	else if (!strncmp(argv[1], "aes256", 6))
		key_size = NVT_CRYPTO_AES_256_KEY_SIZE;
	else
		return CMD_RET_USAGE;

	/* OPmode */
	if (!strncmp(argv[2], "ecb", 3)) {
		opmode     = NVT_CRYPTO_OPMODE_ECB;
		ciphertext = (key_size == NVT_CRYPTO_AES_128_KEY_SIZE) ? AES128_ECB_CIPHERTEXT : AES256_ECB_CIPHERTEXT;
	}
	else if (!strncmp(argv[2], "cbc", 3)) {
		opmode     = NVT_CRYPTO_OPMODE_CBC;
		ciphertext = (key_size == NVT_CRYPTO_AES_128_KEY_SIZE) ? AES128_CBC_CIPHERTEXT : AES256_CBC_CIPHERTEXT;
	}
	else
		return CMD_RET_USAGE;

	/* Key Source */
	if (key_size == NVT_CRYPTO_AES_128_KEY_SIZE) {
		if (!strncmp(argv[3], "ekey0", 5)) {
			key_src = NVT_CRYPTO_KEY_SRC_EFUSE;
			key_ofs = EFUSE_KEY_0_OFFSET;
		}
		else if (!strncmp(argv[3], "ekey1", 5)) {
			key_src = NVT_CRYPTO_KEY_SRC_EFUSE;
			key_ofs = EFUSE_KEY_1_OFFSET;
		}
		else if (!strncmp(argv[3], "ekey2", 5)) {
			key_src = NVT_CRYPTO_KEY_SRC_EFUSE;
			key_ofs = EFUSE_KEY_2_OFFSET;
		}
		else if (!strncmp(argv[3], "ekey3", 5)) {
			key_src = NVT_CRYPTO_KEY_SRC_EFUSE;
			key_ofs = EFUSE_KEY_3_OFFSET;
		}
		else if (!strncmp(argv[3], "ekey4", 5)) {
			key_src = NVT_CRYPTO_KEY_SRC_EFUSE;
			key_ofs = EFUSE_KEY_4_OFFSET;
		}
		else if (!strncmp(argv[3], "ukey", 4)) {
			key_src = NVT_CRYPTO_KEY_SRC_DATA;
			key_ofs = 0;
		}
		else
			return CMD_RET_USAGE;
	}
	else {
		if (!strncmp(argv[3], "ukey", 4)) {
			key_src = NVT_CRYPTO_KEY_SRC_DATA;
			key_ofs = 0;
		}
		else
			return CMD_RET_USAGE;
	}

	/* PIO/DMA */
	if (!strncmp(argv[4], "pio", 3)) {
		dma_mode = 0;
	}
	else if (!strncmp(argv[4], "dma", 3)) {
		dma_mode = 1;
	}
	else
		return CMD_RET_USAGE;

	/* Command */
	if (strncmp(argv[5], "verify", 6))
		return CMD_RET_USAGE;

	/* Open Crypto Device */
	ret = nvt_crypto_open();
	if (ret < 0) {
		printf("nvt crypto open failed!\n");
		return CMD_RET_FAILURE;
	}

	if (key_src == NVT_CRYPTO_KEY_SRC_DATA) {
		/* AES Encryption */
		crypto.encrypt  = 1;
		crypto.opmode   = opmode;
		crypto.src      = AES_PLAINTEXT;
		crypto.dst      = dst_buf;
		crypto.key      = AES_KEY;
		crypto.iv       = (opmode == NVT_CRYPTO_OPMODE_CBC) ? AES_IV  : NULL;
		crypto.key_src  = key_src;
		crypto.key_ofs  = key_ofs;
		crypto.src_size = sizeof(AES_PLAINTEXT);
		crypto.dst_size = sizeof(dst_buf);
		crypto.key_size = key_size;
		crypto.iv_size  = (opmode == NVT_CRYPTO_OPMODE_CBC) ? sizeof(AES_IV) : 0;
		ret = (dma_mode) ? nvt_crypto_dma_aes(&crypto) : nvt_crypto_pio_aes(&crypto);
		if (ret < 0) {
			printf("nvt crypto %s %s encrypt failed!\n", argv[1], argv[2]);
			ret = CMD_RET_FAILURE;
			goto exit;
		}

		/* Output Data Compare */
		if (memcmp(dst_buf, ciphertext, sizeof(dst_buf))) {
			printf("nvt crypto %s %s encryption compare failed!\n", argv[1], argv[2]);
			ret = CMD_RET_FAILURE;
			goto exit;
		}

		/* AES Decryption */
		crypto.encrypt  = 0;
		crypto.opmode   = opmode;
		crypto.src      = ciphertext;
		crypto.dst      = dst_buf;
		crypto.key      = AES_KEY;
		crypto.iv       = (opmode == NVT_CRYPTO_OPMODE_CBC) ? AES_IV : NULL;
		crypto.key_src  = key_src;
		crypto.key_ofs  = key_ofs;
		crypto.src_size = sizeof(dst_buf);
		crypto.dst_size = sizeof(dst_buf);
		crypto.key_size = key_size;
		crypto.iv_size  = (opmode == NVT_CRYPTO_OPMODE_CBC) ? sizeof(AES_IV) : 0;
		ret = (dma_mode) ? nvt_crypto_dma_aes(&crypto) : nvt_crypto_pio_aes(&crypto);
		if (ret < 0) {
			printf("nvt crypto %s %s decrypt failed!\n", argv[1], argv[2]);
			ret = CMD_RET_FAILURE;
			goto exit;
		}

		/* Output Data Compare */
		if (memcmp(dst_buf, AES_PLAINTEXT, sizeof(dst_buf))) {
			printf("nvt crypto %s %s decryption compare failed!\n", argv[1], argv[2]);
			ret = CMD_RET_FAILURE;
			goto exit;
		}
	}
	else {
		uint8_t  *src      = NULL;
		uint8_t  *dst      = NULL;
		uint32_t data_size = 0x2000;

		/* allocate source data buffer */
		src = malloc_cache_aligned(data_size);
		if (!src) {
			printf("nvt crypto %s %s ekey verify to allocate source buffer failed!\n", argv[1], argv[2]);
			ret = CMD_RET_FAILURE;
			goto freemem;
		}

		/* allocate destination data buffer */
		dst = malloc_cache_aligned(data_size);
		if (!dst) {
			printf("nvt crypto %s %s ekey verify to allocate destination buffer failed!\n", argv[1], argv[2]);
			ret = CMD_RET_FAILURE;
			goto freemem;
		}

		/* AES Encryption with efuse key */
		crypto.encrypt  = 1;
		crypto.opmode   = opmode;
		crypto.src      = src;
		crypto.dst      = dst;
		crypto.key      = NULL;
		crypto.iv       = (opmode == NVT_CRYPTO_OPMODE_CBC) ? AES_IV  : NULL;
		crypto.key_src  = key_src;
		crypto.key_ofs  = key_ofs;
		crypto.src_size = data_size;
		crypto.dst_size = data_size;
		crypto.key_size = key_size;
		crypto.iv_size  = (opmode == NVT_CRYPTO_OPMODE_CBC) ? sizeof(AES_IV) : 0;
		ret = (dma_mode) ? nvt_crypto_dma_aes(&crypto) : nvt_crypto_pio_aes(&crypto);
		if (ret < 0) {
			printf("nvt crypto %s %s ekey encrypt failed!\n", argv[1], argv[2]);
			ret = CMD_RET_FAILURE;
			goto freemem;
		}

		/* AES Decryption with efuse test key */
		crypto.encrypt  = 0;
		crypto.opmode   = opmode;
		crypto.src      = dst;
		crypto.dst      = dst;
#ifdef CONFIG_NVT_FPGA_EMULATION
		crypto.key      = (uint8_t *)&EFUSE_TEST_KEY[key_ofs];
		crypto.iv       = (opmode == NVT_CRYPTO_OPMODE_CBC) ? AES_IV : NULL;
		crypto.key_src  = NVT_CRYPTO_KEY_SRC_DATA;
		crypto.key_ofs  = 0;
#else
		crypto.key      = NULL;
		crypto.iv       = (opmode == NVT_CRYPTO_OPMODE_CBC) ? AES_IV : NULL;
		crypto.key_src  = key_src;
		crypto.key_ofs  = key_ofs;
#endif
		crypto.src_size = data_size;
		crypto.dst_size = data_size;
		crypto.key_size = key_size;
		crypto.iv_size  = (opmode == NVT_CRYPTO_OPMODE_CBC) ? sizeof(AES_IV) : 0;
		ret = (dma_mode) ? nvt_crypto_dma_aes(&crypto) : nvt_crypto_pio_aes(&crypto);
		if (ret < 0) {
			printf("nvt crypto %s %s ekey decrypt failed!\n", argv[1], argv[2]);
			ret = CMD_RET_FAILURE;
			goto freemem;
		}

		/* Output Data Compare */
		if (memcmp(src, dst, data_size)) {
			printf("nvt crypto %s %s ekey decryption compare failed!\n", argv[1], argv[2]);
			ret = CMD_RET_FAILURE;
			goto freemem;
		}

freemem:
		if (src)
			free(src);
		if (dst)
			free(dst);
		if (ret != 0)
			goto exit;
	}

	printf("nvt crypto %s %s verify passed!\n", argv[1], argv[2]);

exit:
	/* Close Crypto Device */
	nvt_crypto_close();

	return ret;
}

/***************************************************/
U_BOOT_CMD(
	nvt_crypto, 6, 0, do_nvt_crypto,
	"nvt_crypto operation",
	"\nnvt_crypto aes128 [ecb/cbc] [ekey0-4/ukey] [pio/dma] verify - for verify aes128 encryption/decryption"
	"\nnvt_crypto aes256 [ecb/cbc] [ekey0-4/ukey] [pio/dma] verify - for verify aes256 encryption/decryption"
);
