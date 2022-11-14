#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/io.h>
#include <stdlib.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/nvt_pcie.h>
#include <asm/arch/IOAddress.h>
#include <pci.h>

#define RC_INBOUND_REGION_COUNT		(3)

static int nvt_pcie_find_dma_ranges(struct pci_region *region, char *rc_dts_name, int is_rc)
//static int nvt_pcie_find_dma_ranges(void *blob, int phb_off)
{
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	int addrcell, sizecell, r, offset;
	int len, end;
	u32 *cell = NULL;
	char path[20] = {0};
	u32 *dma_range;
	/* sized based on pci addr cells, size-cells, & address-cells */
	u32 dma_ranges[(3 + 2 + 2) * RC_INBOUND_REGION_COUNT];

	sprintf(path, rc_dts_name);
//	sprintf(path, "/nvt_pcie");
	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n",__func__, __LINE__);
		return -1;
	}

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "#address-cells", &len);
	if (len == 0) {
		printf("%s(%d) len = 0\n",__func__, __LINE__);
		addrcell = 1;
	} else {
		addrcell = __be32_to_cpu(cell[0]);
	}
//	addrcell = fdt_getprop_u32_default(fdt_addr, path, "#address-cells", 1);
	printf("%s: addrcell result %d\r\n", __func__, addrcell);

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "#size-cells", &len);
	if (len == 0) {
		printf("%s(%d) len = 0\n",__func__, __LINE__);
		sizecell = 1;
	} else {
		sizecell = __be32_to_cpu(cell[0]);
	}
	printf("%s: sizecell result %d\r\n", __func__, sizecell);

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "dma-ranges", &len);
	printf("%s: dma-ranges len %d\r\n", __func__, len);
	end = len / sizeof(__be32);

	for (offset=0; offset<end; offset+=(1+sizecell*3)) {
		u32 flag;
		u64 pci_addr;
		u64 cpu_addr;
		u64 range_size;

		flag = of_read_number(cell+offset, 1);
		pci_addr = of_read_number(cell+offset+1, sizecell);
		cpu_addr = of_read_number(cell+offset+1+sizecell, sizecell);
		range_size = of_read_number(cell+offset+1+sizecell*2, sizecell);
		printf("%s: offset %d, flag 0x%x, pci addr 0x%llx, cpu addr 0x%llx, size 0x%llx\r\n",
			__func__, offset, flag, pci_addr, cpu_addr, range_size);
		if (is_rc) {
			struct pci_region *p_region;
			// only expect RC has 2 inbound region
			if (flag & 0x01) {
				// APB
				p_region = &region[1];
			} else {
				// MAU
				p_region = &region[0];
			}

			p_region->bus_start = pci_addr;
			p_region->phys_start = cpu_addr;
			p_region->size = range_size;
		}
	}
#if 0
	dma_range = &dma_ranges[0];
	for (r = 0; r < hose->region_count; r++) {
		u64 bus_start, phys_start, size;
	}
#endif
	return 0;
}

static int nvt_pcie_load_ep_reg(struct pci_region *region, char *ep_dts_name)
{
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	int addrcell, sizecell, r, offset;
	int len, end;
	u32 *cell = NULL;
	char path[20] = {0};
	u32 *dma_range;
	/* sized based on pci addr cells, size-cells, & address-cells */
	u32 dma_ranges[(3 + 2 + 2) * RC_INBOUND_REGION_COUNT];

	sprintf(path, ep_dts_name);
	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n",__func__, __LINE__);
		return -1;
	}

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "reg", &len);
	printf("%s: dma-ranges len %d\r\n", __func__, len);
	end = len / sizeof(__be32);

	for (offset=0; offset<end; offset+=(2*2)) {
		u64 pci_addr;
		u64 range_size;

		pci_addr = of_read_number(cell+offset, 2);
		range_size = of_read_number(cell+offset+2, 2);
		printf("%s: offset %d, pci addr 0x%llx, size 0x%llx\r\n",
			__func__, offset, pci_addr, range_size);
		region->bus_start = pci_addr;
		region->phys_start = pci_addr;
		region->size = range_size;

		region++;
	}
#if 0
	dma_range = &dma_ranges[0];
	for (r = 0; r < hose->region_count; r++) {
		u64 bus_start, phys_start, size;
	}
#endif
	return 0;
}

int nvt_pcie_dts_load_inbound(struct pci_region *region, char *rc_dts_name)
{
	return nvt_pcie_find_dma_ranges(region, rc_dts_name, 1);
}

int nvt_pcie_dts_load_ep_inbound(struct pci_region *region, char *ep_dts_name)
{
	return nvt_pcie_load_ep_reg(region, ep_dts_name);
}


