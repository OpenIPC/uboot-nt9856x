#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <stdlib.h>
#include <asm/arch/IOAddress.h>
#include <asm/nvt-common/nvt_common.h>

#define PCIE_MAPPING_BASE 0x400000000
#define PCIE_SRAM_BASE 0x62E000000

#define EP_BOOT_CPU_BASE 0x620020090
#define EP_BOOT_CPU_VALUE 0xFFFFFCFF
#define EP_LOADER_SIZE 0x10000
#define EP_SRAM_CLK_BASE 0x620020070
#define CNN_CLK_EN 1<<16

#define CC_RC_EP_BOOT_COMM_REG 0x620160120

#define PCIE_STATUS_IDLE 0x544F4F42
#define PCIE_STATUS_DRAM_DONE 0x5944524C

#define TOP_CTRL_REG_BASE_ADDR 0x2F0010000
#define PCIE_BOOT_MSK (0x03 << 7)
#define PCIE_BOOT_RC                    (0x00 << 7)
//#define PCIE_BOOT_RC (0x01 << 8)


static int do_nvt_pcie_copy_sram(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int eploader_size = EP_LOADER_SIZE;
	unsigned int *p_src = (unsigned int *)simple_strtoul(argv[1], NULL, 16);
	unsigned int *p_dst = (unsigned int *)(PCIE_SRAM_BASE);
	unsigned int reg;
	int i;

	reg = readl(EP_SRAM_CLK_BASE);
	reg |= CNN_CLK_EN;
	writel(reg, EP_SRAM_CLK_BASE);

	for (i = 0; i < eploader_size / sizeof(unsigned int); i++) {
		unsigned long a;
		unsigned long b;

		a = readl(p_src);
		writel(a, p_dst);
		b = readl(p_dst);

		if (a != b) {
			nvt_dbg(ERR, "source [0x%8x: 0x%8x], but dst [0x%8x: 0x%8x]\r\n",
					p_src, a, p_dst, b);
			return -1;
		}

		p_src++;
		p_dst++;
	}
	return 0;
}

U_BOOT_CMD(
	nvt_pcie_copy_sram, 2,    1,      do_nvt_pcie_copy_sram,
	"nvt_pcie_copy_sram [eploader addre]",
	""
);

static int do_nvt_pcie_boot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	writel(EP_BOOT_CPU_VALUE, EP_BOOT_CPU_BASE);
	return 0;
}

U_BOOT_CMD(
	nvt_pcie_boot, 1,    1,      do_nvt_pcie_boot,
	"trigger pcie CPU",
	""
);

static char epstatus_help_text[] =
	"nvt_pcie_epstatus get - get current ep status\n"
	"nvt_pcie_epstatus set idle - set communication register to idle\n"
	"nvt_pcie_epstatus set start [epfdt address] - set ep fdt address to communication register\n"
	"";

static int do_nvt_pcie_epstatus(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = -1;
	u32 value;
	u32 status;
	enum {
		IDLE = 0,
		DRAM_DONE,
	};

	if (argc == 2) {
		status = readl(CC_RC_EP_BOOT_COMM_REG);
		switch (status) {
		case PCIE_STATUS_IDLE :
			nvt_dbg(MSG, "EP Idle \n", __func__);
			ret = IDLE;
			break;
		case PCIE_STATUS_DRAM_DONE :
			nvt_dbg(MSG, "EP Dram config done \n", __func__);
			ret = DRAM_DONE;
			break;
		default:
			nvt_dbg(MSG, "EP on unknow status\n", __func__);
			ret = -1;
			break;
		}
	} else if (argc == 3) {
		if (strcmp(argv[2], "idle") == 0) {
			writel(PCIE_STATUS_IDLE, CC_RC_EP_BOOT_COMM_REG);
			ret = 0;
		}
	} else if (argc == 4) {
		if (strcmp(argv[2], "start") == 0) {
			value = (u32)simple_strtoul(argv[3], NULL, 16);
			nvt_dbg(MSG, "write start value 0x%lx\n", value);
			writel(value, CC_RC_EP_BOOT_COMM_REG);
			ret = 0;
		}
	}

	return ret;
}

U_BOOT_CMD(
	nvt_pcie_epstatus, 4,    1,      do_nvt_pcie_epstatus,
	"get or set epstatus on boot ep flow",
	epstatus_help_text
);



static int do_nvt_pcie_getid(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	enum {
		IS_EP = 0,
		IS_RC,
	};
	int ret = IS_EP;
	u32 value;
	value = readl(TOP_CTRL_REG_BASE_ADDR);
	nvt_dbg(MSG, "get id : 0x%lx\n", value);
	if ((value & (PCIE_BOOT_MSK)) == PCIE_BOOT_RC) {
		ret = IS_RC;
	}
	return ret;
}

U_BOOT_CMD(
	nvt_pcie_getid, 1,    1,      do_nvt_pcie_getid,
	"check pcie is RC or EP",
	""
);

static int do_nvt_pcie_copy(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned long *src = (unsigned long *)simple_strtoul(argv[1], NULL, 16);
	unsigned long *dest = (unsigned long *)(simple_strtoul(argv[2], NULL, 16) + PCIE_MAPPING_BASE);
	unsigned long size = (unsigned long)simple_strtoul(argv[3], NULL, 16);
	u32 status;


	status = readl(CC_RC_EP_BOOT_COMM_REG);
	switch (status) {
	case PCIE_STATUS_DRAM_DONE :
		nvt_dbg(MSG, "EP Dram config done \n", __func__);
		break;
	default:
		nvt_dbg(MSG, "EP Dram is not ready\n", __func__);
		return -1;
	}

	nvt_dbg(MSG, "src : 0x%lx, dest : 0x%lx, size : 0x%lx\n", src, dest, size);
	memcpy(dest, src, size);
	flush_cache(round_down((unsigned long)dest, CONFIG_SYS_CACHELINE_SIZE), round_up(size, CONFIG_SYS_CACHELINE_SIZE));
	return 0;
}

U_BOOT_CMD(
	nvt_pcie_copy, 4,    1,      do_nvt_pcie_copy,
	"nvt_pcie_copy [source data] [ep address] [size]",
	""
);


