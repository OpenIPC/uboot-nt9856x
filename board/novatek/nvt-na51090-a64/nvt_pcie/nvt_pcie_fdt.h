/**
    NVT pcie FDT utilities

    @file       nvt_pcie_fdt.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __NVT_PCIE_FDT_H__
#define __NVT_PCIE_FDT_H__

//extern int nvt_pcie_find_dma_ranges(void *blob, int phb_off);

extern int nvt_pcie_dts_load_inbound(struct pci_region *region, char *rc_dts_name);
extern int nvt_pcie_dts_load_ep_inbound(struct pci_region *region, char *ep_dts_name);

#endif
