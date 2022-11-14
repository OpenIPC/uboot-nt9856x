#ifndef __NVT_NAND_MISC_PTN_H__
#define __NVT_NAND_MISC_PTN_H__

/*         MISC entry layout 
 *
 * ---------------------------------
 * | Boot Flag     |  512 byte     |
 * | Update Flag   |  512 byte     | 
 * | PPtbl0        | 1536 byte     |
 * | PPtbl1        | 1536 byte     |
 * | Version Info  |  512 byte     |
 * | VTBL          | 1024 byte     |
 * ---------------------------------
 *
 *   NOTE: 
 *	1. MISC max size is 128KB and block alignment.
 *	2. The value of no used area are 0x0.
 */

#define BOOT_FLAG_SZ	(512)
#define UPDATE_FLAG_SZ	(512)
#define PPTBL_FLAG_SZ	(1536)
#define VERSION_INFO	(512)
#define VTBL		(1024)

#define BOOT_FLAG_OFFSET	(0)
#define UPDATE_FLAG_OFFSET	(BOOT_FLAG_OFFSET     + BOOT_FLAG_SZ	)
#define PPTBL0_FLAG_OFFSET	(UPDATE_FLAG_OFFSET   + UPDATE_FLAG_SZ	)
#define PPTBL1_FLAG_OFFSET	(PPTBL0_FLAG_OFFSET   + PPTBL_FLAG_SZ	)
#define VERSION_INFO_OFFSET	(PPTBL1_FLAG_OFFSET   + PPTBL_FLAG_SZ	)
#define VTBL_OFFSET		(VERSION_INFO_OFFSET  + VERSION_INFO	)

#endif /* __NVT_NAND_MISC_PTN_H__ */
