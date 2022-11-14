/*
 *  board/novatek/evb670btk/nvt_tzone.h
 *
 *  Author:	Alvin lin
 *  Created:	Oct 18, 2015
 *  Copyright:	Novatek Inc.
 *
 */
#ifndef __NVT_TRUST_ZONE_HDR_H__
#define __NVT_TRUST_ZONE_HDR_H__
#define SECOS_HEADER_MAGIC_CODE                 "SECOS"

typedef struct secos_header {
	unsigned char MagicCode[8];
	unsigned int Version[2];
	unsigned int BinSize;
	unsigned int BinOffset;
	unsigned int Reserve[2];
} secos_header_t;

#define SECOS_ENC_BUFFER_START			(0x02000000)										/* Load address of encrypted Secure OS */
#define SECOS_ENC_BUFFER_BLOCK			(SECOS_ENC_BUFFER_START - sizeof(secos_header_t))	/* Load address of header of encryped Secure OS */
#define SECOS_BUFFER_START				(0x00008000)										/* Entry point and load address of Secure OS */
#define SECOS_BUFFER_BLOCK				(SECOS_BUFFER_START - sizeof(secos_header_t))		/* Load address of header of Secure OS */
#define MAILBOX_REG_BASE                (0xfd0201c0)
#define MAILBOX1                                (*((volatile unsigned long *)(MAILBOX_REG_BASE + 0x8)))
#define MAILBOX2                                (*((volatile unsigned long *)(MAILBOX_REG_BASE + 0x10)))
#define MAILBOX3                                (*((volatile unsigned long *)(MAILBOX_REG_BASE + 0x18)))
//#define NVT_XBOOT_DTB_BUFF_ADDR 		0x20100000
#define NVT_XBOOT_DTB_BUFF_ADDR 		0x1E200000

int dec_secos_header(unsigned long enc_buf, unsigned long buf, const char *magic);
#endif
