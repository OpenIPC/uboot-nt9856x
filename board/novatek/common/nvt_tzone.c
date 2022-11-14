/*
 *  board/novatek/evb670btk/nvt_tzone.c
 *
 *  Author:	Alvin lin
 *  Created:	Oct 18, 2015
 *  Copyright:	Novatek Inc.
 *
 */

#include <common.h>
#include <asm/barriers.h>
#include <nvt_emmc_partition.h>
#include <u-boot/sha256.h>
#include "crypto_core.h"
#include "nvt_tzone.h"

/**
 * @brief Decrypt header of secos OS
 *
 * @param [in] Load address of encrypted header
 * @param [in] Load address of header
 * @return Encrypted return 1, not encrypted return 0, error return -1
 */
int dec_secos_header(unsigned long enc_buf, unsigned long buf, const char *magic)
{
	/**
	 * If MagicCode is "SECOS", not need to descrypt
	 */
	if(memcmp(((secos_header_t *)enc_buf)->MagicCode, magic, strlen(magic)) != 0)
	{

		CryptoCoreInit();

		/**
		 * If decrypt header of secos fail, return -1
		 */
		printf("Prepare to decrypt header of %s from %08x to %08x of length %d\n", magic, enc_buf, buf, sizeof(secos_header_t));

		if(dec((unsigned char *)enc_buf, (unsigned char *)buf, sizeof(secos_header_t)))
		{
			return -1;
		}

		if(memcmp(((secos_header_t *)buf)->MagicCode, magic, strlen(magic)) != 0)
		{
			printf("%s header is incorrect\n", magic);
			while(1);
		}
		
		return 1;
	}

	memcpy((void *)buf, (void *)enc_buf, sizeof(secos_header_t));

	return 0;
}

extern void jump_to_secos(void (*kernel)(unsigned long, unsigned long, unsigned long), unsigned long secos, unsigned long dtb);

void arch_preboot_os(void)
{
	unsigned long addr = 0;
	unsigned long dec_len;
	int with_common_header = 0;
	image_header_t *phdr;

	if(env_get_yesno("no_secos")) 
		return;

	phdr = (image_header_t*)(SECOS_BUFFER_START - sizeof(image_header_t));
	if(image_get_magic(phdr) == IH_MAGIC) 
		return;

	if(memcmp(((secos_header_t *)SECOS_ENC_BUFFER_BLOCK)->MagicCode, SECOS_HEADER_MAGIC_CODE, strlen(SECOS_HEADER_MAGIC_CODE)) == 0)
		addr = SECOS_BUFFER_BLOCK;
	else
		addr = SECOS_ENC_BUFFER_BLOCK;
	if(memcmp(((secos_header_t *)SECOS_ENC_BUFFER_BLOCK)->MagicCode, SECOS_HEADER_MAGIC_CODE, strlen(SECOS_HEADER_MAGIC_CODE)) != 0) {
		if(dec_secos_header(SECOS_ENC_BUFFER_BLOCK, SECOS_BUFFER_BLOCK, SECOS_HEADER_MAGIC_CODE) == 1) {
			dec_len = ((secos_header_t *)SECOS_BUFFER_BLOCK)->BinSize;
			printf("prepare decrytpt SECOS from %08x to %08x, length %d\n", SECOS_ENC_BUFFER_START, SECOS_BUFFER_START, dec_len);
			dec((unsigned char *)SECOS_ENC_BUFFER_START, (unsigned char *)SECOS_BUFFER_START, dec_len);
		}
	}
}

static unsigned long dtb_buffer_start;

void arch_preboot_os_post(unsigned long real_fdt_addr)
{
	void (*theKernel) (unsigned long, unsigned long, unsigned long);
	unsigned long ker_addr;

	if(env_get_yesno("no_secos")) 
		return;

	while(MAILBOX1);

	dtb_buffer_start = real_fdt_addr;

	theKernel = (void(*)(unsigned long, unsigned long, unsigned long))(simple_strtoul(env_get("kernel_loadaddr"),NULL,16) + 0x40);

	printf("Request to start secos\n");
	DSB;

	MAILBOX1 = SECOS_BUFFER_START;///< Set entry point of Secure OS in MAILBOX1

	printf("Start to wait secos\n");
	while(1) {
		if(MAILBOX1 != (unsigned int)SECOS_BUFFER_START)
			break;
	}

	/*
	 * secure OS requires
	 *      r2 = DTB buffer address
	 *      r3 = 0: AC on
	 *      r3 = 1: warm boot
	 */
	printf("MAILBOX1 0x%08x\n",MAILBOX1);

	printf("Finish to wait secos\n");

	//v7_outer_cache_enable();
	jump_to_secos(theKernel, MAILBOX1, dtb_buffer_start);

}
