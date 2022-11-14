/*
 *  nvt_vtbl_check.h
 *
 *  Author:	Alvin lin
 *  Created:	Nov 26, 2015
 *  Copyright:	Novatek Inc.
 *
 */
#ifndef __NVT_VTABLE_CHECK_H__
#define __NVT_VTABLE_CHECK_H__
int nvt_emmc_write_vtbl_img(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int buf_sz, void *args);
int nvt_emmc_write_vtbl_img_uuid(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int buf_sz, void *args);
#endif

