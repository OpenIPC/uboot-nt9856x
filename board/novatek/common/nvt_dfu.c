/**
    NVT update handling api

    @file       nvt_ivot_fw_update_utils.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <common.h>
#include <malloc.h>
#include <fs.h>
#include <memalign.h>
#include <linux/ctype.h>

static char *get_dfu_alt_system(char *interface, char *devstr)
{
        return env_get("dfu_alt_system");
}

static char *get_dfu_alt_boot(char *interface, char *devstr)
{
        char *alt_boot;
        alt_boot = DFU_ALT_INFO;

        return alt_boot;
}
void set_dfu_alt_info(char *interface, char *devstr)
{
        size_t buf_size = CONFIG_SET_DFU_ALT_BUF_LEN;
        ALLOC_CACHE_ALIGN_BUFFER(char, buf, buf_size);
        char *alt_info = "Settings not found!";
        char *status = "error!\n";
        char *alt_setting;
        char *alt_sep;
        int offset = 0;

        puts("DFU alt info setting: ");

        alt_setting = get_dfu_alt_boot(interface, devstr);
        if (alt_setting) {
                env_set("dfu_alt_boot", alt_setting);
                offset = snprintf(buf, buf_size, "%s", alt_setting);
        }

        alt_setting = get_dfu_alt_system(interface, devstr);
        if (alt_setting) {
                if (offset)
                        alt_sep = ";";
                else
                        alt_sep = "";

                offset += snprintf(buf + offset, buf_size - offset,
                                    "%s%s", alt_sep, alt_setting);
        }

        if (offset) {
                alt_info = buf;
                status = "done\n";
        }

        env_set("dfu_alt_info", alt_info);
        puts(status);
}