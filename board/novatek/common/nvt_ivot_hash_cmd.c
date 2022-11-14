/*
 * Copyright (c) 2021 Novatek Inc.
 *
 */

#include <common.h>
#include <asm/arch/nvt_hash.h>

DECLARE_GLOBAL_DATA_PTR;

#define _ALIGNED(x)         __attribute__((aligned(x)))
#define CACHE_LINE_SIZE     CONFIG_SYS_CACHELINE_SIZE

static uint8_t _ALIGNED(CACHE_LINE_SIZE) PLAINTEXT[] = {      ///< 56 bytes
	0x61, 0x62, 0x63, 0x64, 0x62, 0x63, 0x64, 0x65, 0x63, 0x64, 0x65, 0x66, 0x64, 0x65, 0x66, 0x67,
	0x65, 0x66, 0x67, 0x68, 0x66, 0x67, 0x68, 0x69, 0x67, 0x68, 0x69, 0x6A, 0x68, 0x69, 0x6A, 0x6B,
	0x69, 0x6A, 0x6B, 0x6C, 0x6A, 0x6B, 0x6C, 0x6D, 0x6B, 0x6C, 0x6D, 0x6E, 0x6C, 0x6D, 0x6E, 0x6F,
	0x6D, 0x6E, 0x6F, 0x70, 0x6E, 0x6F, 0x70, 0x71
};

static uint8_t SHA1_DIGEST[] = {    ///< 20 bytes
	0x84, 0x98, 0x3E, 0x44, 0x1C, 0x3B, 0xD2, 0x6E, 0xBA, 0xAE,
	0x4A, 0xA1, 0xF9, 0x51, 0x29, 0xE5, 0xE5, 0x46, 0x70, 0xF1
};

static uint8_t SHA256_DIGEST[] = {  ///< 32 bytes
	0x24, 0x8D, 0x6A, 0x61, 0xD2, 0x06, 0x38, 0xB8,
	0xE5, 0xC0, 0x26, 0x93, 0x0C, 0x3E, 0x60, 0x39,
	0xA3, 0x3C, 0xE4, 0x59, 0x64, 0xFF, 0x21, 0x67,
	0xF6, 0xEC, 0xED, 0xD4, 0x19, 0xDB, 0x06, 0xC1
};

/**
 * do_nvt_hash() - Handle the "nvt_hash" command-line command
 * @cmdtp:	Command data struct pointer
 * @flag:	Command flag
 * @argc:	Command-line argument count
 * @argv:	Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_nvt_hash(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0;
	int mode;
	int dma_mode;
	uint8_t *golden_digest;
	uint8_t digest[NVT_HASH_SHA256_DIGEST_SIZE];
	struct nvt_hash_pio_t hash;

	/* Check argument */
	if (argc < 4)
		return CMD_RET_USAGE;

	/* Mode */
	if (!strncmp(argv[1], "sha1", 4)) {
		mode          = NVT_HASH_MODE_SHA1;
		golden_digest = SHA1_DIGEST;
	}
	else if (!strncmp(argv[1], "sha256", 6)) {
		mode          = NVT_HASH_MODE_SHA256;
		golden_digest = SHA256_DIGEST;
	}
	else
		return CMD_RET_USAGE;

	/* PIO/DMA */
	if (!strncmp(argv[2], "pio", 3)) {
		dma_mode = 0;
	}
	else if (!strncmp(argv[2], "dma", 3)) {
		dma_mode = 1;
	}
	else
		return CMD_RET_USAGE;

	/* Command */
	if (strncmp(argv[3], "verify", 6))
		return CMD_RET_USAGE;

	/* Open Hash Device */
	ret = nvt_hash_open();
	if (ret < 0) {
		printf("nvt hash open failed!\n");
		return CMD_RET_FAILURE;
	}

	/* Hash Data */
	hash.mode        = mode;
	hash.src         = PLAINTEXT;
	hash.digest      = digest;
	hash.src_size    = sizeof(PLAINTEXT);
	hash.digest_size = (mode == NVT_HASH_MODE_SHA256) ? NVT_HASH_SHA256_DIGEST_SIZE : NVT_HASH_SHA1_DIGEST_SIZE;
	ret = (dma_mode) ? nvt_hash_dma_sha(&hash) : nvt_hash_pio_sha(&hash);
	if (ret < 0) {
		printf("nvt hash %s failed!\n", argv[1]);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	/* Digest Compare */
	if (memcmp(digest, golden_digest, hash.digest_size))
		printf("nvt hash %s verify failed!\n", argv[1]);
	else
		printf("nvt hash %s verify passed!\n", argv[1]);

exit:
	/* Close Hash Device */
	nvt_hash_close();

	return ret;
}

/***************************************************/
U_BOOT_CMD(
	nvt_hash, 4, 0, do_nvt_hash,
	"nvt_hash operation",
	"\nnvt_hash sha1 [pio/dma] verify - for verify sha1"
	"\nnvt_hash sha256 [pio/dma] verify - for verify sha256"
);
