#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/io.h>
#include <stdlib.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/nvt_pcie.h>
#include <asm/arch/IOAddress.h>
#include <pci.h>

#if defined(CONFIG_TARGET_NA51090) || defined(CONFIG_TARGET_NA51090_A64)

#define DRV_VER	"0.03"

#define NVT_PCIE_RC_LOOPBACK	0
#define NVT_PCIE_RC		1
#define NVT_PCIE_EP		2
#define NVT_PCIE_RC_EP		3
#if defined (CONFIG_NVT_PCIE_TEST_MODE)
#define NVT_PCIE_MODE		(CONFIG_NVT_PCIE_TEST_MODE)
#else
#define NVT_PCIE_MODE		(NVT_PCIE_RC_LOOPBACK)
#endif

#if (NVT_PCIE_MODE == NVT_PCIE_RC) || (NVT_PCIE_MODE == NVT_PCIE_RC_EP)
#define ATU_NO_MAU1		0
#define ATU_NO_CFG		1	// share mem with ATU_NO_REMOTE_ATU
#define ATU_NO_REMOTE_ATU	2	// share mem with ATU_NO_CFG
#define ATU_NO_MAU2		3
#define ATU_NO_APB		4
#endif

struct nvt_pci_ctrl {
	phys_addr_t dbi_base;		// DBI base (virtual address)
	phys_addr_t atu_base;		// ATU base (virtual address)
	phys_addr_t cfg_base;		// CFG base (virtual address)
	struct pci_region regions[MAX_PCI_REGIONS];
	int region_count;
	int mode;	// NVT_PCIE_RC or NVT_PCIE_EP
	int last_atu;	// record last accessed ATU type (for EP multiplex)

	int (*read_dbi)(struct nvt_pci_ctrl*, int reg, u32 *);
	int (*write_dbi)(struct nvt_pci_ctrl*, int reg, u32);

	int (*read_cfg)(struct nvt_pci_ctrl*, int reg, u32 *);
	int (*write_cfg)(struct nvt_pci_ctrl*, int reg, u32);

	int (*read_atu)(struct nvt_pci_ctrl*, int reg, u32 *);
	int (*write_atu)(struct nvt_pci_ctrl*, int reg, u32);

	void (*config_oATU)(struct nvt_pci_ctrl*, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, int atu_type, bool en);
	void (*config_iATU)(struct nvt_pci_ctrl*, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, bool en);
};

#define pcie_ctrl_readl_ep    pcie_ctrl_readl_rc
#define pcie_ctrl_writel_ep   pcie_ctrl_writel_rc
#define ioread32        readl
#define iowrite32       writel
#define read_reg		readl
#define write_reg(base, val)	writel(val, base)

#if !defined(CONFIG_ARM64)
#define CPU_PCIE_MAP_VA		(0xD0000000)
#define CPU_PCIE_MAP_SIZE	(0x10000000)
#define PCIE_CFG_ATU_PA		(0x500000000ULL)

enum {
	MMU_LARGE_SECTION_SHIFT = 24, /* 16MB */
	MMU_LARGE_SECTION_SIZE  = 1 << MMU_LARGE_SECTION_SHIFT,
};

static void tlb_set_section_large(int section, u64 target_pa)
{
	u32 *page_table = (u32 *)gd->arch.tlb_addr;
	u32 value = 0x00040806;
	u64 entry = target_pa;

	/* Add the page offset */
	entry >>= (MMU_LARGE_SECTION_SHIFT-MMU_SECTION_SHIFT);
	entry = ((entry>>8)&0x0F)|((entry&0xFF)<<4);
	value |= ((u32)entry << MMU_SECTION_SHIFT);
//	printf("%s: input section %x, target pa 0x%llx = 0x%x\r\n", __func__, section, target_pa, value);

	/* Set PTE */
	page_table[section] = value;
}

static void tlb_add_pcie_mapping(phys_addr_t start, size_t size, u64 target_pa)
{
	int i;
	unsigned long startpt, stoppt;
	unsigned long upto, end;
	u32 *page_table = (u32 *)gd->arch.tlb_addr;

	end = ALIGN(start + size, MMU_SECTION_SIZE) >> MMU_SECTION_SHIFT;
	start = start >> MMU_SECTION_SHIFT;

	target_pa >>= MMU_SECTION_SHIFT;
	for (i=start; i<end; i++, target_pa++) {
		tlb_set_section_large(i, target_pa);
	}

	startpt = (unsigned long)&page_table[start];
	startpt &= ~(CONFIG_SYS_CACHELINE_SIZE - 1);
	stoppt = (unsigned long)&page_table[end];
	stoppt = ALIGN(stoppt, CONFIG_SYS_CACHELINE_SIZE);
	mmu_page_table_flush(startpt, stoppt);
}

static void ep_cfg_write(unsigned int val, phys_addr_t base)
{
	iowrite32(val, (void *)(CPU_PCIE_MAP_VA + base));
}

static u32 ep_cfg_read(phys_addr_t base)
{
	u32 data;

	data = ioread32((void *)(CPU_PCIE_MAP_VA + base));

	return data;
}

#else

#define CPU_CFG_MAP_VA		(0x600000000)
#define CPU_ATU_MAP_VA		(0x610000000)
#define CPU_PCIE_MAP_SIZE	(0x10000000)
#define PCIE_CFG_ATU_PA		(CPU_ATU_MAP_VA)

static void ep_cfg_write(unsigned int val, phys_addr_t base)
{
	iowrite32(val, (void *)(CPU_CFG_MAP_VA + base));
}

static u32 ep_cfg_read(phys_addr_t base)
{
	u32 data;

	data = ioread32((void *)(CPU_CFG_MAP_VA + base));

	return data;
}

#endif

/* local function declaration */
static void pcie_ctrl_readl_rc(phys_addr_t base, unsigned int *val)
{
	*val = ioread32((void *)base);
}

static void pcie_ctrl_writel_rc(unsigned int val, phys_addr_t base)
{
	iowrite32(val, (void *)base);
}

static u32 rc_apbm_src(phys_addr_t addr, u32 rdata_exp, u32 mask) {
	u32 data;

	pcie_ctrl_readl_rc(addr, &data);

	if ((data&mask) == (rdata_exp&mask)) {
		printf("%s:  compare pass, both 0x%x\r\n", __func__, rdata_exp);
		return 0;
	} else {
		printf("%s:  compare err, read 0x%x, expect 0x%x\r\n", __func__, data, rdata_exp);
		return 1;
	}
}


static void rc_apbm_srmw(phys_addr_t addr, u32 data, u32 start, u32 width) {
	const u32 MASK = (1<<width) - 1;
	u32 data_tmp;

	pcie_ctrl_readl_rc(addr, &data_tmp);
	data_tmp = (data_tmp & (~(MASK << start))) | ((data & MASK) << start);
	pcie_ctrl_writel_rc(data_tmp, addr);
}


static void rc_dbi_srmw(phys_addr_t reg, u32 data, u32 start, u32 width) {
	rc_apbm_srmw(PCIE_DBI_REG_BASE+reg, data, start, width);
}

static u32 rc_dbi_srp(phys_addr_t reg) {
	u32 data;

	pcie_ctrl_readl_rc((PCIE_DBI_REG_BASE+reg), &data);

	return data;
}


static void rc_dbi_sw(phys_addr_t reg, u32 data) {
	printf("%s: addr 0x%x, data 0x%x\r\n", __func__, PCIE_DBI_REG_BASE+reg, data);
	//COMMON_WriteReg(PCIE_DBI_REG_BASE+reg, data);
	pcie_ctrl_writel_rc(data, PCIE_DBI_REG_BASE+reg);
}

static u32 rc_apbm_sr(phys_addr_t addr) {
	u32 data;

	pcie_ctrl_readl_rc((addr), &data);

	return data;
}

static void rc_slv_sw(phys_addr_t addr, u32 data) {
	//COMMON_WriteReg(addr, data);
	pcie_ctrl_writel_rc(data, addr);
}

static u32 rc_slv_src(phys_addr_t addr, u32 rdata_exp, u32 mask) {
	return rc_apbm_src(addr, rdata_exp, mask);
}

static void config_oATU(struct nvt_pci_ctrl* ctrl, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, int atu_type, bool en) {
	uint32_t shift_mode = 0;

	if (ctrl == NULL) {
		printf("%s: NULL ctrl\r\n", __func__);
	}

	ctrl->write_atu(ctrl, IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0 + region_id*0x200,
		src_addr&0xFFFFFFFF);
	ctrl->write_atu(ctrl, IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0 + region_id*0x200,
		src_addr>>32);
	ctrl->write_atu(ctrl, IATU_LIMIT_ADDR_OFF_OUTBOUND_0 + region_id*0x200,
		src_upper_addr&0xFFFFFFFF);
	ctrl->write_atu(ctrl, IATU_UPPER_LIMIT_ADDR_OFF_OUTBOUND_0 + region_id*0x200,
		src_upper_addr>>32);
	ctrl->write_atu(ctrl, IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0 + region_id*0x200,
		target_addr&0xFFFFFFFF);
	ctrl->write_atu(ctrl, IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0 + region_id*0x200,
		target_addr>>32);
	ctrl->write_atu(ctrl, IATU_REGION_CTRL_1_OFF_OUTBOUND_0 + region_id*0x200,    atu_type); //[4:0]00100 CFG Type0

	if (atu_type == PCIE_ATU_TYPE_CFG0) {
		atu_type |= 1<<28;
	}
	ctrl->write_atu(ctrl, IATU_REGION_CTRL_2_OFF_OUTBOUND_0 + region_id*0x200,    (en<<31)); //[31]REGION_EN [28]CFG_SHIFT_MODE
}

static void config_iATU(struct nvt_pci_ctrl* ctrl, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, bool en) {
	if (ctrl == NULL) {
		printf("%s: NULL ctrl\r\n", __func__);
	}

	ctrl->write_atu(ctrl, IATU_LWR_BASE_ADDR_OFF_INBOUND_0 + 0x200*region_id,
		src_addr&0xFFFFFFFF);
	ctrl->write_atu(ctrl, IATU_UPPER_BASE_ADDR_OFF_INBOUND_0 + 0x200*region_id,
		src_addr>>32);
	ctrl->write_atu(ctrl, IATU_LIMIT_ADDR_OFF_INBOUND_0 + 0x200*region_id,
		src_upper_addr&0xFFFFFFFF);
	ctrl->write_atu(ctrl, IATU_UPPER_LIMIT_ADDR_OFF_INBOUND_0 + 0x200*region_id,
		src_upper_addr>>32);
	ctrl->write_atu(ctrl, IATU_LWR_TARGET_ADDR_OFF_INBOUND_0 + 0x200*region_id,
		target_addr&0xFFFFFFFF);
	ctrl->write_atu(ctrl, IATU_UPPER_TARGET_ADDR_OFF_INBOUND_0 + region_id*0x200,
		target_addr>>32);
	ctrl->write_atu(ctrl, IATU_REGION_CTRL_1_OFF_INBOUND_0 + region_id*0x200,
		0x00000000); //[4:0]00000 Memory
	ctrl->write_atu(ctrl, IATU_REGION_CTRL_2_OFF_INBOUND_0 + region_id*0x200,    (en<<31)); //[31]REGION_EN [28]CFG_SHIFT_MODE
}

static int rc_read_dbi(struct nvt_pci_ctrl* ctrl, int reg, u32 *val) {
	if (ctrl == NULL) return -1;
	if (val == NULL) return -1;

	pcie_ctrl_readl_rc((ctrl->dbi_base+reg), val);

	return 0;
}

static int rc_write_dbi(struct nvt_pci_ctrl* ctrl, int reg, u32 val) {
	if (ctrl == NULL) return -1;

	pcie_ctrl_writel_rc(val, (ctrl->dbi_base+reg));

	return 0;
}

static int rc_read_atu(struct nvt_pci_ctrl* ctrl, int reg, u32 *val) {
	if (ctrl == NULL) return -1;
	if (val == NULL) return -1;

	pcie_ctrl_readl_rc((ctrl->atu_base+reg), val);

	return 0;
}

static int rc_write_atu(struct nvt_pci_ctrl* ctrl, int reg, u32 val) {
	if (ctrl == NULL) return -1;

	pcie_ctrl_writel_rc(val, (ctrl->atu_base+reg));
	printf("%s: addr 0x%lx (base 0x%lx), value 0x%lx\r\n", __func__, (ctrl->atu_base+reg), ctrl->atu_base, val);

	return 0;
}

static struct nvt_pci_ctrl nvt_rc = {
	.dbi_base =	PCIE_DBI_REG_BASE,
	.atu_base =	PCIE_DBI_REG_BASE + PF0_ATU_CAP_DBIBaseAddress,
	.mode =		NVT_PCIE_RC,
	.last_atu =	-1,
	.read_dbi =	rc_read_dbi,
	.write_dbi =	rc_write_dbi,

	.read_cfg =	rc_read_dbi,	// DBI offset 0 maps to local CFG
	.write_cfg =	rc_write_dbi,	// DBI offset 0 maps to local CFG

	.read_atu =	rc_read_atu,
	.write_atu =	rc_write_atu,

	.config_oATU =	config_oATU,
	.config_iATU =	config_iATU,
};

#if (NVT_PCIE_MODE == NVT_PCIE_RC) || (NVT_PCIE_MODE == NVT_PCIE_RC_EP)

static int ep_read_dbi(struct nvt_pci_ctrl* ctrl, int reg, u32 *val) {
	if (ctrl == NULL) return -1;
	if (val == NULL) return -1;
printf("%s: UNDER CONSTRUCTION\r\n", __func__);
	if (ctrl->last_atu != ATU_NO_APB) {
	}

	pcie_ctrl_readl_rc((ctrl->dbi_base+reg), val);

	ctrl->last_atu = ATU_NO_APB;

	return 0;
}

static int ep_write_dbi(struct nvt_pci_ctrl* ctrl, int reg, u32 val) {
	if (ctrl == NULL) return -1;
printf("%s: UNDER CONSTRUCTION\r\n", __func__);
	if (ctrl->last_atu != ATU_NO_APB) {
	}

	pcie_ctrl_writel_rc(val, (ctrl->dbi_base+reg));

	ctrl->last_atu = ATU_NO_APB;

	return 0;
}

static int ep_read_cfg(struct nvt_pci_ctrl* ctrl, int reg, u32 *val) {
	if (ctrl == NULL) return -1;
	if (val == NULL) return -1;

#if !defined(CONFIG_ARM64)
	if (ctrl->last_atu != ATU_NO_CFG) {
		// oATU shadow to CFG
		nvt_rc.config_oATU(&nvt_rc, PCIE_CFG_ATU_PA, PCIE_CFG_ATU_PA+CPU_PCIE_MAP_SIZE-1, PCIE_CFG_ATU_PA, ATU_NO_CFG, PCIE_ATU_TYPE_CFG0, true);
	}
#endif

	pcie_ctrl_readl_rc((ctrl->cfg_base+reg), val);

	ctrl->last_atu = ATU_NO_CFG;

	return 0;
}

static int ep_write_cfg(struct nvt_pci_ctrl* ctrl, int reg, u32 val) {
	if (ctrl == NULL) return -1;

#if !defined(CONFIG_ARM64)
	if (ctrl->last_atu != ATU_NO_CFG) {
		// oATU shadow to CFG
		nvt_rc.config_oATU(&nvt_rc, PCIE_CFG_ATU_PA, PCIE_CFG_ATU_PA+CPU_PCIE_MAP_SIZE-1, PCIE_CFG_ATU_PA, ATU_NO_CFG, PCIE_ATU_TYPE_CFG0, true);
	}
#endif

	pcie_ctrl_writel_rc(val, (ctrl->cfg_base+reg));

	ctrl->last_atu = ATU_NO_CFG;

	return 0;
}

static int ep_read_atu(struct nvt_pci_ctrl* ctrl, int reg, u32 *val) {
	if (ctrl == NULL) return -1;
	if (val == NULL) return -1;

#if !defined(CONFIG_ARM64)
	if (ctrl->last_atu != ATU_NO_REMOTE_ATU) {
		// oATU shadow to ATU
		nvt_rc.config_oATU(&nvt_rc, PCIE_CFG_ATU_PA, PCIE_CFG_ATU_PA+CPU_PCIE_MAP_SIZE-1, PCIE_CFG_ATU_PA, ATU_NO_CFG, PCIE_ATU_TYPE_MEM, true);
	}
#endif

	pcie_ctrl_readl_rc((ctrl->atu_base+reg), val);
printf("%s: addr 0x%lx, val 0x%lx\r\n", __func__, (ctrl->atu_base+reg), val);
	ctrl->last_atu = ATU_NO_REMOTE_ATU;

	return 0;
}

static int ep_write_atu(struct nvt_pci_ctrl* ctrl, int reg, u32 val) {
	if (ctrl == NULL) return -1;

#if !defined(CONFIG_ARM64)
	if (ctrl->last_atu != ATU_NO_REMOTE_ATU) {
		// oATU shadow to ATU
		nvt_rc.config_oATU(&nvt_rc, PCIE_CFG_ATU_PA, PCIE_CFG_ATU_PA+CPU_PCIE_MAP_SIZE-1, PCIE_CFG_ATU_PA, ATU_NO_CFG, PCIE_ATU_TYPE_MEM, true);
	}
#endif
printf("%s: addr 0x%lx (base 0x%lx), value 0x%lx\r\n", __func__, (ctrl->atu_base+reg), ctrl->atu_base, val);
	pcie_ctrl_writel_rc(val, (ctrl->atu_base+reg));

	ctrl->last_atu = ATU_NO_REMOTE_ATU;

	return 0;
}

static struct nvt_pci_ctrl nvt_ep = {
	.dbi_base =	(PCIE_SLV_oATU0_BASE + 0x0D000000),
	.cfg_base =	CPU_CFG_MAP_VA,
	.atu_base =	CPU_ATU_MAP_VA,
//	.dbi_base =	CPU_PCIE_MAP_VA,
//	.cfg_base =	CPU_PCIE_MAP_VA,
//	.atu_base =	CPU_PCIE_MAP_VA,
	.mode =		NVT_PCIE_EP,
	.last_atu =	-1,
	.read_dbi =	ep_read_dbi,
	.write_dbi =	ep_write_dbi,

	.read_cfg =	ep_read_cfg,
	.write_cfg =	ep_write_cfg,

	.read_atu =	ep_read_atu,
	.write_atu =	ep_write_atu,

	.config_oATU =	config_oATU,
	.config_iATU =	config_iATU,
};
#endif


static void pcie_ini(u32 target_speed, u32 eq_enable) {
	u32 reg;

#if (0)
//#if  (CFG_FPGA_ENVIRONMENT_EN==1)
	printf("%s: FPGA keep gen 1\r\n", __func__);
	rc_dbi_srmw(LINK_CONTROL2_LINK_STATUS2_REG, 1, LINK_CONTROL2_LINK_STATUS2_REG_PCIE_CAP_TARGET_LINK_SPEED_BitAddressOffset, 4); //adr, dat, start, width //set target gen1
#else
	printf("%s: LINK STS2 reg 0x%x\r\n", __func__, rc_dbi_srp(LINK_CONTROL2_LINK_STATUS2_REG));
	 //i_rc.link_train_en = 1'b1; //default link_train_en RC = 0, EP = 1
	rc_apbm_srmw(PCIE_TOP_REG_BASE+0x304, 0x01, 2, 1);	//[2]link_train_en
#endif

	reg = rc_apbm_sr(PCIE_TOP_REG_BASE+0x08);
	printf("%s: phy link up/down %d, data link up/down %d, LTSSM 0x%x\r\n",
		__func__, (reg>>11)&0x01, (reg>>0)&0x01, (reg>>4)&0x3F);
}

static void pcie_atu_rc_ini(void) {
#if defined(CONFIG_TARGET_NA51090)
	printf("RC iATU outbound read/write (A32), should set before EP BAR setting ...\r\n");
#if (NVT_PCIE_MODE == NVT_PCIE_RC_LOOPBACK)
	nvt_rc.config_oATU(&nvt_rc, 0x400000000ULL, 0x41FFFFFFFULL, 0x400000000ULL, 0, PCIE_ATU_TYPE_MEM, true);
	nvt_rc.config_oATU(&nvt_rc, 0xA00000000ULL, 0xA0FFFFFFFULL, 0xA00000000ULL, 1, PCIE_ATU_TYPE_MEM, true);
	nvt_rc.config_oATU(&nvt_rc, 0xFF0000000ULL, 0xFFFFFFFFFULL, 0xFF0000000ULL, 2, PCIE_ATU_TYPE_MEM, true);
#else
	nvt_rc.config_oATU(&nvt_rc, 0x400000000ULL, 0x41FFFFFFFULL, 0x400000000ULL, ATU_NO_MAU1, PCIE_ATU_TYPE_MEM, true);
	nvt_rc.config_oATU(&nvt_rc, 0x500000000ULL, 0x50FFFFFFFULL, 0x500000000ULL, ATU_NO_CFG, PCIE_ATU_TYPE_CFG0, true);
//	nvt_rc.config_oATU(&nvt_rc, 0x500000000ULL, 0x50FFFFFFFULL, 0x500000000ULL, ATU_NO_REMOTE_ATU, PCIE_ATU_TYPE_MEM, false);
//	nvt_rc.config_oATU(&nvt_rc, 0x500000000ULL, 0x50FFFFFFFULL, 0x500000000ULL, ATU_NO_MAU2, PCIE_ATU_TYPE_MEM, false);
	nvt_rc.config_oATU(&nvt_rc, 0xFF0000000ULL, 0xFFFFFFFFFULL, 0xFF0000000ULL, ATU_NO_APB, PCIE_ATU_TYPE_MEM, true);
#endif

#else
	printf("RC iATU outbound read/write (A64), should set before EP BAR setting ...\r\n");
#if (NVT_PCIE_MODE == NVT_PCIE_RC_LOOPBACK)
	nvt_rc.config_oATU(&nvt_rc, PCIE_SLV_oATU0_BASE, PCIE_SLV_oATU0_LIM, PCIE_SLV_oATU0_BASE, 0, PCIE_ATU_TYPE_MEM, true);
	nvt_rc.config_oATU(&nvt_rc, PCIE_SLV_oATU1_BASE, PCIE_SLV_oATU1_LIM, PCIE_SLV_oATU1_BASE, 1, PCIE_ATU_TYPE_MEM, true);
	nvt_rc.config_oATU(&nvt_rc, 0xA00000000, 0xA1fffffff, 0xA00000000, 2, PCIE_ATU_TYPE_MEM, true);
#else
	nvt_rc.config_oATU(&nvt_rc, PCIE_SLV_oATU0_BASE, PCIE_SLV_oATU0_LIM, PCIE_SLV_oATU0_BASE, 0, PCIE_ATU_TYPE_MEM, true);
	nvt_rc.config_oATU(&nvt_rc, PCIE_SLV_oATU1_BASE, PCIE_SLV_oATU1_LIM, PCIE_SLV_oATU1_BASE, 1, PCIE_ATU_TYPE_MEM, true);
	nvt_rc.config_oATU(&nvt_rc, PCIE_SLV_oATU2_BASE, PCIE_SLV_oATU2_LIM, PCIE_SLV_oATU2_BASE, 2, PCIE_ATU_TYPE_MEM, true);
	nvt_rc.config_oATU(&nvt_rc, 0x600000000ULL, 0x60FFFFFFFULL, 0x600000000ULL, 3, PCIE_ATU_TYPE_CFG0, true);
#endif

#endif
#if 0
	rc_dbi_sw(IATU_LWR_BASE_ADDR_DBIOFF_OUTBOUND_0,    PCIE_SLV_oATU0_BASE&0xFFFFFFFF);	//source base
	rc_dbi_sw(IATU_UPPER_BASE_ADDR_DBIOFF_OUTBOUND_0,  PCIE_SLV_oATU0_BASE>>32);		//source base
	rc_dbi_sw(IATU_LIMIT_ADDR_DBIOFF_OUTBOUND_0,       PCIE_SLV_oATU0_LIM&0xFFFFFFFF);	//source limit //0825
	rc_dbi_sw(IATU_UPPER_LIMIT_ADDR_DBIOFF_OUTBOUND_0, PCIE_SLV_oATU0_LIM>>32);
	rc_dbi_sw(IATU_LWR_TARGET_ADDR_DBIOFF_OUTBOUND_0,  PCIE_SLV_oATU0_BASE&0xFFFFFFFF);	//target base
	rc_dbi_sw(IATU_UPPER_TARGET_ADDR_DBIOFF_OUTBOUND_0,PCIE_SLV_oATU0_BASE>>32);		//target base
	rc_dbi_sw(IATU_REGION_CTRL_1_DBIOFF_OUTBOUND_0,    0x00000000); //[4:0]00100 CFG Type0
	rc_dbi_sw(IATU_REGION_CTRL_2_DBIOFF_OUTBOUND_0,    0x80000000); //[31]REGION_EN [28]CFG_SHIFT_MODE
//	rc_dbi_sw(IATU_UPPER_TARGET_ADDR_DBIOFF_OUTBOUND_0,0x00000000);
//	rc_dbi_sw(IATU_UPPER_LIMIT_ADDR_DBIOFF_OUTBOUND_0, 0x00000000);

	rc_dbi_sw(IATU_LWR_BASE_ADDR_DBIOFF_OUTBOUND_1,    PCIE_SLV_oATU1_BASE&0xFFFFFFFF);	//source base
	rc_dbi_sw(IATU_UPPER_BASE_ADDR_DBIOFF_OUTBOUND_1,  PCIE_SLV_oATU1_BASE>>32); 		//source base
	rc_dbi_sw(IATU_LIMIT_ADDR_DBIOFF_OUTBOUND_1,       PCIE_SLV_oATU1_LIM&0xFFFFFFFF);			//source limit
	rc_dbi_sw(IATU_UPPER_LIMIT_ADDR_DBIOFF_OUTBOUND_1, PCIE_SLV_oATU1_LIM>>32);
	rc_dbi_sw(IATU_LWR_TARGET_ADDR_DBIOFF_OUTBOUND_1,  PCIE_SLV_oATU1_BASE&0xFFFFFFFF);	//target base
	rc_dbi_sw(IATU_UPPER_TARGET_ADDR_DBIOFF_OUTBOUND_1,PCIE_SLV_oATU1_BASE>>32);		//target base
	rc_dbi_sw(IATU_REGION_CTRL_1_DBIOFF_OUTBOUND_1,    0x00000000); //[4:0]00000 Memory
	rc_dbi_sw(IATU_REGION_CTRL_2_DBIOFF_OUTBOUND_1,    0x80000000); //[31]REGION_EN [28]CFG_SHIFT_MODE
//	rc_dbi_sw(IATU_UPPER_LIMIT_ADDR_DBIOFF_OUTBOUND_1, 0x00000000);
#endif
}

#if (NVT_PCIE_MODE == NVT_PCIE_RC) || (NVT_PCIE_MODE == NVT_PCIE_RC_EP)

static void pcie_atu_ep_ini(void) {
#if defined(CONFIG_TARGET_NA51090)
	// EP iATU
	nvt_ep.config_iATU(&nvt_ep, 0x400000000ULL, 0x41FFFFFFFULL, 0x000000000ULL, 0, true);
	nvt_ep.config_iATU(&nvt_ep, 0xFF0000000ULL, 0xFFFFFFFFFULL, 0x2F0000000ULL, 1, true);

	// EP oATU
	nvt_ep.config_oATU(&nvt_ep, 0x400000000ULL, 0x41FFFFFFFULL, 0x400000000ULL, 0, PCIE_ATU_TYPE_MEM, true);
	nvt_ep.config_oATU(&nvt_ep, 0x500000000ULL, 0x50FFFFFFFULL, 0x500000000ULL, 1, PCIE_ATU_TYPE_MEM, true);
	nvt_ep.config_oATU(&nvt_ep, 0xFF0000000ULL, 0xFFFFFFFFFULL, 0xFF0000000ULL, 2, PCIE_ATU_TYPE_MEM, true);
#else
	// EP iATU
	nvt_ep.config_iATU(&nvt_ep, PCIE_EP0_MEM_BASE, PCIE_SLV_oATU1_LIM, 0x000000000ULL, 0, true);
	nvt_ep.config_iATU(&nvt_ep, PCIE_EP0_MEM2_BASE, PCIE_SLV_oATU2_LIM, PCIE_EP0_REAL_MEM2_BASE, 1, true);
	nvt_ep.config_iATU(&nvt_ep, PCIE_EP0_REG_BASE, PCIE_SLV_oATU0_LIM, PCIE_EP0_REAL_REG_BASE, 2, true);

	// EP oATU
	nvt_ep.config_oATU(&nvt_ep, 0xC00000000ULL, 0xC2FFFFFFFULL, 0xC00000000ULL, 0, PCIE_ATU_TYPE_MEM, true);
	nvt_ep.config_oATU(&nvt_ep, 0xE00000000ULL, 0xE0FFFFFFFULL, 0xE00000000ULL, 1, PCIE_ATU_TYPE_MEM, true);
#endif
}

/* bar_sz: 0:1M, 1:2M, 2:4M, 3:8M ... */
static void ep_set_resizable_barsz(int bar_idx, u32 bar_sz)
{
	u32 val, tmp, order = 0, resize_idx;

	tmp = (bar_sz >> 20) - 1;
	while (tmp) {
		order ++;
		tmp >>= 1;
	}

	switch (bar_idx) {
	case 0:
		resize_idx = 0;
		break;
	default:
		panic("error bar_idx: %d \n", bar_idx);
		break;
	}

	nvt_ep.read_cfg(&nvt_ep, 0x2B4 + (resize_idx*8) + 0x8, &val);
	val &= ~(0x1F << 8);
	val |= ((order & 0x1F) << 8);
	nvt_ep.write_cfg(&nvt_ep, 0x2B4 + (resize_idx*8) + 0x8, val);
}

static void setup_ep_bar(int bar_idx, u64 bar_base, u64 bar_sz) {
	u32 reg;

	printf("%s: bar %d, base 0x%llx, size 0x%llx\r\n", __func__, bar_idx, bar_base, bar_sz);
	if (bar_idx == 0) {
		// BAR1
		ep_set_resizable_barsz(0, bar_sz);
	}

	nvt_ep.write_cfg(&nvt_ep, 0x10 + 0x4*bar_idx, bar_base&0xFFFFFFFF);
	nvt_ep.write_cfg(&nvt_ep, 0x10 + 0x4*bar_idx + 0x4, bar_base>>32);
}

static void pcie_ep_bar_ini(void) {
	setup_ep_bar(0, PCIE_SLV_oATU1_BASE, 0x10000000ULL);	// BAR0: MAU

	setup_ep_bar(2, PCIE_SLV_oATU0_BASE, 0x10000000ULL);	// BAR2: APB

	setup_ep_bar(4, PCIE_CFG_ATU_PA, CPU_PCIE_MAP_SIZE);	// BAR4: ATU
}

#endif

static void pcie_atu_rc_inbound_loopback_ini(void) {
	printf("RC iATU inbound ...\r\n");

#if defined(CONFIG_TARGET_NA51090)
	nvt_rc.config_iATU(&nvt_rc, 0x400000000ULL, 0x41FFFFFFFULL, 0x000000000ULL, 0, true);
	nvt_rc.config_iATU(&nvt_rc, 0xA00000000ULL, 0xA0FFFFFFFULL, 0x100000000ULL, 1, true);
	nvt_rc.config_iATU(&nvt_rc, 0xFF0000000ULL, 0xFFFFFFFFFULL, 0x2F0000000ULL, 2, true);
#else

#if (NVT_PCIE_MODE == NVT_PCIE_RC_LOOPBACK)
	nvt_rc.config_iATU(&nvt_rc, PCIE_EP0_MEM_BASE, PCIE_SLV_oATU1_LIM, PCIE_EP0_REAL_MEM_BASE, 0, true);
	nvt_rc.config_iATU(&nvt_rc, 0xA00000000, 0xA1fffffff, PCIE_EP0_REAL_MEM2_BASE, 1, true);
	nvt_rc.config_iATU(&nvt_rc, PCIE_EP0_REG_BASE, PCIE_SLV_oATU0_LIM, PCIE_EP0_REAL_REG_BASE, 2, true);
#else
	nvt_rc.config_iATU(&nvt_rc, 0xC00000000, 0xD1FFFFFFF, 0xC00000000, 0, true);
	nvt_rc.config_iATU(&nvt_rc, 0xE00000000, 0xE0FFFFFFF, PCIE_EP0_REAL_REG_BASE, 1, true);
#endif

#endif

#if 0
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_LWR_BASE_ADDR_OFF_INBOUND_0,    PCIE_EP0_MEM_BASE&0xFFFFFFFF);//source base
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_UPPER_BASE_ADDR_OFF_INBOUND_0,  PCIE_EP0_MEM_BASE>>32);	//source base
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_LIMIT_ADDR_OFF_INBOUND_0,       PCIE_SLV_oATU1_LIM&0xFFFFFFFF); //source limit
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_UPPER_LIMIT_ADDR_OFF_INBOUND_0, PCIE_SLV_oATU1_LIM>>32);
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_LWR_TARGET_ADDR_OFF_INBOUND_0,  PCIE_EP0_REAL_MEM_BASE&0xFFFFFFFF); //target base
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_UPPER_TARGET_ADDR_OFF_INBOUND_0,PCIE_EP0_REAL_MEM_BASE>>32);
//	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_REGION_CTRL_1_OFF_INBOUND_0,    0x10000000); //[4:0]00000 Memory
//	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_REGION_CTRL_1_OFF_INBOUND_0,    0x20000000); //[4:0]00000 Memory
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_REGION_CTRL_1_OFF_INBOUND_0,    0x00000000); //[4:0]00000 Memory
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_REGION_CTRL_2_OFF_INBOUND_0,    0x80000000); //[31]REGION_EN [28]CFG_SHIFT_MODE
//	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_UPPER_TARGET_ADDR_OFF_INBOUND_0,0x00000000);
//	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_UPPER_LIMIT_ADDR_OFF_INBOUND_0, 0x00000000);
#endif

#if 0
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_LWR_BASE_ADDR_OFF_INBOUND_1,    PCIE_EP0_REG_BASE&0xFFFFFFFF);//source base
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_UPPER_BASE_ADDR_OFF_INBOUND_1,  PCIE_EP0_REG_BASE>>32);	//source base
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_LIMIT_ADDR_OFF_INBOUND_1,       PCIE_SLV_oATU0_LIM&0xFFFFFFFF);	//source limit
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_UPPER_LIMIT_ADDR_OFF_INBOUND_1, PCIE_SLV_oATU0_LIM>>32);	//source limit
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_LWR_TARGET_ADDR_OFF_INBOUND_1,  PCIE_EP0_REAL_REG_BASE&0xFFFFFFFF);	//target base
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_UPPER_TARGET_ADDR_OFF_INBOUND_1,PCIE_EP0_REAL_REG_BASE>>32);	//target base
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_REGION_CTRL_1_OFF_INBOUND_1,    0x00000000); //[4:0]00000 Memory
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_REGION_CTRL_2_OFF_INBOUND_1,    0x80000000); //[31]REGION_EN [28]CFG_SHIFT_MODE
//	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_UPPER_TARGET_ADDR_OFF_INBOUND_1,0x00000000);
//	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_UPPER_LIMIT_ADDR_OFF_INBOUND_1, 0x00000000);
#endif

#if 0
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_LWR_BASE_ADDR_OFF_INBOUND_2,    PCIE_EP0_EC_BASE); //source base
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_LIMIT_ADDR_OFF_INBOUND_2,       PCIE_EP0_EC_BASE+0x00ffffff); //source limit
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_LWR_TARGET_ADDR_OFF_INBOUND_2,  PCIE_EP0_REAL_EC_BASE); //target base
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_REGION_CTRL_1_OFF_INBOUND_2,    0x00000000); //[4:0]00000 Memory
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_REGION_CTRL_2_OFF_INBOUND_2,    0x80000000); //[31]REGION_EN [28]CFG_SHIFT_MODE
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_UPPER_TARGET_ADDR_OFF_INBOUND_2,0x00000000);
	rc_dbi_sw(PF0_ATU_CAP_DBIBaseAddress+IATU_UPPER_LIMIT_ADDR_OFF_INBOUND_2, 0x00000000);
#endif
}


static u32 pcie_ini0(u32 opt) {
	u32 opt_L0offL1on = 0;
	u32 opt_fake_phy_en = 0;

	if ((opt&0x03) == 0x01) {
		opt_L0offL1on = 1;
	}

	opt_fake_phy_en = (opt>>2) & 0x01;

	rc_apbm_srmw(PCIE_TOP_REG_BASE+0x25C, opt_fake_phy_en, 23, 1);	//[23]fake_phy_en

	rc_apbm_srmw(PCIE_TOP_REG_BASE+0x300, 0x07, 0, 3);	//[2:0]pipe_rst_n, power_up_rst_n, perst_n

	if (opt_L0offL1on) {
		rc_apbm_srmw(PCIE_TOP_REG_BASE+0x308, 0x01, 24, 2);	//[25:24]00:L0 only, 01:L1 only, 10: L0,L1 on
	}

	return 3;
}


#if 0
static u32 pcie_heavy_load_test(u32 addr, u32 size) {
	u32 ret = 0;

	rc_slv_sw(PCIE_HVYLD_BASE+0x8090, 0xaaaaaaaa);	//TEST_PATTERN_0
	rc_slv_sw(PCIE_HVYLD_BASE+0x8094, 0x55555555);	//TEST_PATTERN_1
	rc_slv_sw(PCIE_HVYLD_BASE+0x8098, 0xaaaaaaaa);	//TEST_PATTERN_2
	rc_slv_sw(PCIE_HVYLD_BASE+0x809c, 0x55555555);	//TEST_PATTERN_3
	rc_slv_sw(PCIE_HVYLD_BASE+0x8064, addr);	// heavy load channel 0 start address
	rc_slv_sw(PCIE_HVYLD_BASE+0x8068, size);	// heavy load channel 0 pattern size
	rc_slv_sw(PCIE_HVYLD_BASE+0x80a0, 0x00000003);	// [0]done_status [1]err_status clear status

	rc_slv_sw(PCIE_HVYLD_BASE+0x8060, (1<<0)|(0<<4)|(3<<6)|(0xf<<8)|(0<<16));	// ch0  0 outstanding (bit-31:16--> test repeat times) (bit-15:8 --> burst length)

	rc_slv_sw(PCIE_HVYLD_BASE+0x805c, 0x00000001);	// channel 0/1 start (bit-0,1)

	while (1) {
		u32 reg;

		reg = rc_apbm_sr(PCIE_HVYLD_BASE+0x80a0);
		if (reg & 0x02) {
			ret = 1;
			printf("%s: heavy load error 0x%x\r\n", __func__, reg);
			break;
		}
		if (reg & 0x01) break;
	}

	rc_slv_sw(PCIE_HVYLD_BASE+0x80a0, 0x00000003);	// [0]done_status [1]err_status clear status

	return ret;
}
#endif

static void pcie_ep_mem_rw(void) {
	printf("========================================\r\n");
	printf("EP memory read/write ...\r\n");
	printf("========================================\r\n");

	// memory
	rc_slv_sw(0x40000000, 0xaaaaaaaa);
	rc_slv_sw(0x40000004, 0x55555555);
	rc_slv_sw(0x4ffffff8, 0x12345678);
	rc_slv_sw(0x4ffffffc, 0x87654321);

	rc_slv_src(0x40000000, 0xaaaaaaaa, 0xffffffff);
	rc_slv_src(0x40000004, 0x55555555, 0xffffffff);
	rc_slv_src(0x4ffffff8, 0x12345678, 0xffffffff);
	rc_slv_src(0x4ffffffc, 0x87654321, 0xffffffff);

	//if (pcie_heavy_load_test(0x40000000, 0x1004) == 0) {
	//	printf("heavy load R/W ok\r\n");
	//} else {
	//	printf("heavy load R/W err\r\n");
	//}
}

static void pcie_init(void) {
	u32 target_speed;
	u32 u32_val;

#if !defined(CONFIG_ARM64)
	tlb_add_pcie_mapping(0xC0000000, CPU_PCIE_MAP_SIZE, 0x400000000ULL);
	tlb_add_pcie_mapping(CPU_PCIE_MAP_VA, CPU_PCIE_MAP_SIZE, PCIE_CFG_ATU_PA);
	tlb_add_pcie_mapping(0xE0000000, CPU_PCIE_MAP_SIZE, 0xFF0000000ULL);
#endif

#if defined(CONFIG_NVT_PCIE_TEST_MODE)
printf("%s: CONFIG_NVT_PCIE_TEST_MODE: %d\r\n", __func__, CONFIG_NVT_PCIE_TEST_MODE);
#else
printf("%s: no CONFIG_NVT_PCIE_TEST_MODE\r\n", __func__);
#endif

	// -------------------------- //
	//  PCIE TOP APB 1st test
	// -------------------------- //
	rc_apbm_src(PCIE_TOP_REG_BASE+0x000,0x061b0000, 0xffffffff);
	rc_apbm_src(PCIE_TOP_REG_BASE+0x304,0x00000030, 0xffffffff);

	// -------------------------- //
	//  Bootstrap
	// -------------------------- //
	pcie_ctrl_writel_rc( (1<<0)|(1<<1)|(0<<2)|(1<<3)|(0<<4)|(1<<6) , (PCIE_TOP_REG_BASE+0x308));
	rc_apbm_src((PCIE_TOP_REG_BASE+0x308), (1<<0)|(1<<1)|(0<<2)|(1<<3)|(0<<4)|(1<<6), 0xffffffff);

	// -------------------------- //
	//  PHY APB
	// -------------------------- //
	//reset phy apb
	rc_apbm_srmw(PCIE_TOP_REG_BASE+0x300, 0x00, 3, 1);	//[2:0]pipe_rst_n, power_up_rst_n, perst_n [3]phy_apb_rst_n
	rc_apbm_srmw(PCIE_TOP_REG_BASE+0x300, 0x01, 3, 1);	//[2:0]pipe_rst_n, power_up_rst_n, perst_n [3]phy_apb_rst_n

	// -------------------------- //
	//  initialization
	// -------------------------- //
	// [2]fake_phy_en, [1:0]00:L0onL1off 01:L0offL1on
	target_speed = pcie_ini0((1<<2)|(0<<0));	//opt, target_speed_o

	// -------------------------- //
	//  DBI
	// -------------------------- //
#if (NVT_PCIE_MODE == NVT_PCIE_RC_LOOPBACK)
	rc_dbi_srmw(PIPE_LOOPBACK_CONTROL_OFF, 1, PIPE_LOOPBACK_CONTROL_OFF_PIPE_LOOPBACK_BitAddressOffset, 1); //
	rc_dbi_srmw(PORT_LINK_CTRL_OFF, 1, PORT_LINK_CTRL_OFF_LOOPBACK_ENABLE_BitAddressOffset, 1); //
#else
	// release PRST
	u32_val = ioread32(IOADDR_TOP_REG_BASE + 0x28);
	u32_val |= 1<<4;	// release preset
	iowrite32(u32_val, IOADDR_TOP_REG_BASE + 0x28);
#endif

	u32_val = rc_dbi_srp(PORT_LINK_CTRL_OFF); //
	printf("%s: PORT_LINK_CTRL_OFF = 0x%x\r\n", __func__, u32_val);

	pcie_ini(target_speed,0);   //target_speed, eq_enable:0:eq disable

	pcie_atu_rc_ini(); //rc atu should set first to let EP bar can access

#if (NVT_PCIE_MODE == NVT_PCIE_RC) || (NVT_PCIE_MODE == NVT_PCIE_RC_EP)
	pcie_ep_bar_ini();
#endif

	pcie_atu_rc_inbound_loopback_ini();

	rc_dbi_srmw(STATUS_COMMAND_REG, 1, STATUS_COMMAND_REG_PCI_TYPE0_IO_EN_BitAddressOffset, STATUS_COMMAND_REG_PCI_TYPE0_IO_EN_RegisterSize);
	rc_dbi_srmw(STATUS_COMMAND_REG, 1, STATUS_COMMAND_REG_PCI_TYPE0_MEM_SPACE_EN_BitAddressOffset, STATUS_COMMAND_REG_PCI_TYPE0_MEM_SPACE_EN_RegisterSize);
	rc_dbi_srmw(STATUS_COMMAND_REG, 1, STATUS_COMMAND_REG_PCI_TYPE0_BUS_MASTER_EN_BitAddressOffset, STATUS_COMMAND_REG_PCI_TYPE0_BUS_MASTER_EN_RegisterSize);

#if (NVT_PCIE_MODE == NVT_PCIE_RC) || (NVT_PCIE_MODE == NVT_PCIE_RC_EP)
	// Enable EP
	nvt_ep.write_cfg(&nvt_ep, PCI_COMMAND, PCI_COMMAND_MASTER|PCI_COMMAND_MEMORY);

	// Config EP ATU after EP is enabled
	pcie_atu_ep_ini();

	// switch D000_0000 to ATU
	nvt_ep.read_atu(&nvt_ep, IATU_REGION_CTRL_1_OFF_OUTBOUND_0, &u32_val);
#endif

#if (CONFIG_SYS_PCI_64BIT == 1)
printf("%s: CONFIG_SYS_PCI_64BIT\r\n", __func__);
#else
printf("%s: NO CONFIG_SYS_PCI_64BIT\r\n", __func__);
#endif
	//pcie_ep_mem_rw();
}



/* this function is portable for the customer */
int do_pcie (cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	printk("-------------- PCIe Version: %s -------------- \n", DRV_VER);
	pcie_init();

	return 0;
}


U_BOOT_CMD(
	pcie,	2,	1,	do_pcie,
	"create pcie-mapping",
	"command: pcie [1/2/3]"
);

#endif
