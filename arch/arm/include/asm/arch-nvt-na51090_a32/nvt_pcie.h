#ifndef _ARCH_PCIE_H
#define _ARCH_PCIE_H

#define	PCIE_HVYLD_BASE				0xec030000
#define PCIE_TOP_REG_BASE				0xf04f0000
#define PCIE_DBI_REG_BASE				0xfd000000

#define	PCIE_SLV_oATU0_BASE			0xFF0000000 //RC oATU memory, (map to register)
#define	PCIE_SLV_oATU0_LIM			0xFFfffffff

#define	PCIE_SLV_oATU1_BASE			0x400000000 //RC oATU memory, (map to MAU)
#define	PCIE_SLV_oATU1_LIM			0x41fffffff
//#define	PCIE_SLV_oATU1_LIM			0x1fffffff
#define	PCIE_SLV_oATU2_BASE			0x500000000 //RC oATU memory, (map to MAU2)
#define	PCIE_SLV_oATU2_LIM			0x51fffffff
#define	PCIE_EP0CFG_BASE			0x70000000
#define	PCIE_EP0CFG_LIM				0x7000ffff

#define	PCIE_EP0_MEM_BASE			0x400000000
#define	PCIE_EP0_MEM2_BASE			0x500000000
#define	PCIE_EP0_REG_BASE			0xFF0000000
#define	PCIE_EP0_C1_BASE			0x51000000
#define	PCIE_EP0_EC_BASE			0x5c000000
//#define	PCIE_EP0_REAL_MEM_BASE			0x2F0000000
#define	PCIE_EP0_REAL_MEM_BASE			0x20000000
#define	PCIE_EP0_REAL_MEM2_BASE			0x120000000
#define	PCIE_EP0_REAL_C1_BASE			0xc1000000
#define	PCIE_EP0_REAL_REG_BASE			0x2F0000000
#define	PCIE_EP0_REAL_EC_BASE			0xec000000

#define PCIE_ATU_TYPE_MEM			(0x0 << 0)
#define PCIE_ATU_TYPE_IO			(0x2 << 0)
#define PCIE_ATU_TYPE_CFG0			(0x4 << 0)
#define PCIE_ATU_TYPE_CFG1			(0x5 << 0)

#define	PF0_TYPE0_HDR_BaseAddress	0x0
#define	PF0_PCIE_CAP_BaseAddress	0x70
#define	PF0_PORT_LOGIC_BaseAddress	0x700

#define PF0_ATU_CAP_BaseAddress		0x00000000
#define PF0_ATU_CAP_DBIBaseAddress	0x00300000

/* Register STATUS_COMMAND_REG */
/* Status and Command Register. */
#define	STATUS_COMMAND_REG			(PF0_TYPE0_HDR_BaseAddress + 0x4)

/* Register STATUS_COMMAND_REG field PCI_TYPE0_IO_EN */
/* IO Space Enable.
Controls a Function's response to I/O Space accesses.
 - When this bit is set, the Function is enabled to decode the address and further process I/O Space accesses.
 - When this bit is clear, all received I/O accesses are caused to be handled as Unsupported Requests.
For a Function that does not support I/O Space accesses, the controller hardwires this bit to 0b.

Note: The access attributes of this field are as follows:
 - Dbi: !has_io_bar ? RO : RW  */
#define	STATUS_COMMAND_REG_PCI_TYPE0_IO_EN_BitAddressOffset	0
#define	STATUS_COMMAND_REG_PCI_TYPE0_IO_EN_RegisterSize 	1

/* Register STATUS_COMMAND_REG field PCI_TYPE0_MEM_SPACE_EN */
/* Memory Space Enable.
Controls a Function's response to Memory Space accesses.
 - When this bit is set, the Function is enabled to decode the address and further process Memory Space accesses.
 - When this bit is clear, all received Memory Space accesses are caused to be handled as Unsupported Requests.
For a Function does not support Memory Space accesses, the controller hardwires this bit to 0b.

Note: The access attributes of this field are as follows:
 - Dbi: !has_mem_bar ? RO : RW  */
#define	STATUS_COMMAND_REG_PCI_TYPE0_MEM_SPACE_EN_BitAddressOffset	1
#define	STATUS_COMMAND_REG_PCI_TYPE0_MEM_SPACE_EN_RegisterSize		1

/* Register STATUS_COMMAND_REG field PCI_TYPE0_BUS_MASTER_EN */
/* Bus Master Enable.
Controls the ability of a Function to issue Memory and I/O Read/Write requests.
 - When this bit is set, the Function is allowed to issue Memory or I/O Requests.
 - When this bit is clear, the Function is not allowed to issue any Memory or I/O Requests.
Requests other than Memory or I/O Requests are not controlled by this bit.

Note: MSI/MSI-X interrupt Messages are in-band memory writes, setting the Bus Master Enable bit to 0b disables MSI/MSI-X interrupt Messages as well.

Note: The access attributes of this field are as follows:
 - Dbi: R/W  */
#define	STATUS_COMMAND_REG_PCI_TYPE0_BUS_MASTER_EN_BitAddressOffset	2
#define	STATUS_COMMAND_REG_PCI_TYPE0_BUS_MASTER_EN_RegisterSize		1

/* Register LINK_CONTROL2_LINK_STATUS2_REG */
/* Link Control 2 and Status 2 Register.For a description of this standard PCIe register, see the PCI Express Specification. */
#define	LINK_CONTROL2_LINK_STATUS2_REG	(PF0_PCIE_CAP_BaseAddress + 0x30)

/* Register LINK_CONTROL2_LINK_STATUS2_REG field PCIE_CAP_TARGET_LINK_SPEED */
/* Target Link Speed.
For a description of this standard PCIe register field, see the PCI Express Specification.

 In M-PCIe mode, the contents of this field are derived from other registers.

Note: This register field is sticky. */
#define LINK_CONTROL2_LINK_STATUS2_REG_PCIE_CAP_TARGET_LINK_SPEED_BitAddressOffset	0
#define LINK_CONTROL2_LINK_STATUS2_REG_PCIE_CAP_TARGET_LINK_SPEED_RegisterSize		4


/* Register PORT_LINK_CTRL_OFF *//* Port Link Control Register. */
#define	PORT_LINK_CTRL_OFF	(PF0_PORT_LOGIC_BaseAddress + 0x10)

/* Register PORT_LINK_CTRL_OFF field LOOPBACK_ENABLE */
/* Loopback Enable. Turns on loopback. For more details, see "Loopback".
For M-PCIe, to force the master to enter Digital Loopback mode, you must set this field to "1" during Configuration.start state(initial discovery/configuration).
M-PCIe doesn't support loopback mode from L0 state - only from Configuration.start.
Note: This register field is sticky. */
#define	PORT_LINK_CTRL_OFF_LOOPBACK_ENABLE_BitAddressOffset	2
#define	PORT_LINK_CTRL_OFF_LOOPBACK_ENABLE_RegisterSize		1

/* Register PIPE_LOOPBACK_CONTROL_OFF */
/* PIPE Loopback Control Register. */
#define	PIPE_LOOPBACK_CONTROL_OFF		(PF0_PORT_LOGIC_BaseAddress + 0x1b8)

/* Register PIPE_LOOPBACK_CONTROL_OFF field PIPE_LOOPBACK */
/* PIPE Loopback Enable. Indicates RMMI Loopback if M-PCIe.Note: This register field is sticky. */
#define	PIPE_LOOPBACK_CONTROL_OFF_PIPE_LOOPBACK_BitAddressOffset	31
#define	PIPE_LOOPBACK_CONTROL_OFF_PIPE_LOOPBACK_RegisterSize		1


/* Register IATU_REGION_CTRL_1_OFF_OUTBOUND_0 */
/* iATU Region Control 1 Register. */
#define	IATU_REGION_CTRL_1_OFF_OUTBOUND_0	(PF0_ATU_CAP_BaseAddress + 0x0)
#define	IATU_REGION_CTRL_1_DBIOFF_OUTBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x0)

/* Register IATU_REGION_CTRL_2_OFF_OUTBOUND_0 */
/* iATU Region Control 2 Register. */
#define	IATU_REGION_CTRL_2_OFF_OUTBOUND_0	(PF0_ATU_CAP_BaseAddress + 0x4)
#define	IATU_REGION_CTRL_2_DBIOFF_OUTBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x4)

/* Register IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0 */
/* iATU Lower Base Address Register. The CX_ATU_MIN_REGION_SIZE configuration parameter (Value Range: 4 kB, 8 kB, 16 kB, 32 kB, 64 kB defaults to 64 kB) specifies the minimum size of an address translation region. For example, if set to 64 kB; the lower 16 bits of the Base, Limit and Target registers are zero and all address regions are aligned on 64 kB boundaries. More precisely, the lower log2(CX_ATU_MIN_REGION_SIZE) bits are zero. */
#define	IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0	(PF0_ATU_CAP_BaseAddress + 0x8)
#define	IATU_LWR_BASE_ADDR_DBIOFF_OUTBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x8)

/* Register IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0 */
/* iATU Upper Base Address Register. */
#define	IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0	(PF0_ATU_CAP_BaseAddress + 0xc)
#define	IATU_UPPER_BASE_ADDR_DBIOFF_OUTBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0xc)

/* Register IATU_LIMIT_ADDR_OFF_OUTBOUND_0 */
/* iATU Limit Address Register. */
#define	IATU_LIMIT_ADDR_OFF_OUTBOUND_0		(PF0_ATU_CAP_BaseAddress + 0x10)
#define	IATU_LIMIT_ADDR_DBIOFF_OUTBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x10)

/* Register IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0 */
/* iATU Lower Target Address Register. */
#define	IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0	(PF0_ATU_CAP_BaseAddress + 0x14)
#define	IATU_LWR_TARGET_ADDR_DBIOFF_OUTBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x14)

/* Register IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0 */
/* iATU Upper Target Address Register. */
#define	IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0	(PF0_ATU_CAP_BaseAddress + 0x18)
#define	IATU_UPPER_TARGET_ADDR_DBIOFF_OUTBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x18)
#define	IATU_UPPER_LIMIT_ADDR_OFF_OUTBOUND_0	(PF0_ATU_CAP_BaseAddress + 0x20)
#define	IATU_UPPER_LIMIT_ADDR_DBIOFF_OUTBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x20)

/* Register IATU_REGION_CTRL_1_OFF_INBOUND_0 */
/* iATU Region Control 1 Register. */
#define IATU_REGION_CTRL_1_OFF_INBOUND_0	(PF0_ATU_CAP_BaseAddress + 0x100)
#define	IATU_REGION_CTRL_1_DBIOFF_INBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x100)

/* Register IATU_REGION_CTRL_2_OFF_INBOUND_0 */
/* iATU Region Control 2 Register. */
#define	IATU_REGION_CTRL_2_OFF_INBOUND_0	(PF0_ATU_CAP_BaseAddress + 0x104)
#define	IATU_REGION_CTRL_2_DBIOFF_INBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x104)

/* Register IATU_LWR_BASE_ADDR_OFF_INBOUND_0 */
/* iATU Lower Base Address Register. The CX_ATU_MIN_REGION_SIZE configuration parameter (Value Range: 4 kB, 8 kB, 16 kB, 32 kB, 64 kB defaults to 64 kB) specifies the minimum size of an address translation region. For example, if set to 64 kB; the lower 16 bits of the Base, Limit and Target registers are zero and all address regions are aligned on 64 kB boundaries. More precisely, the lower log2(CX_ATU_MIN_REGION_SIZE) bits are zero. */
#define	IATU_LWR_BASE_ADDR_OFF_INBOUND_0	(PF0_ATU_CAP_BaseAddress + 0x108)
#define	IATU_LWR_BASE_ADDR_DBIOFF_INBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x108)

/* Register IATU_UPPER_BASE_ADDR_OFF_INBOUND_0 */
/* iATU Upper Base Address Register. */
#define	IATU_UPPER_BASE_ADDR_OFF_INBOUND_0	(PF0_ATU_CAP_BaseAddress + 0x10c)
#define	IATU_UPPER_BASE_ADDR_DBIOFF_INBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x10c)

/* Register IATU_LIMIT_ADDR_OFF_INBOUND_0 */
/* iATU Limit Address Register. */
#define	IATU_LIMIT_ADDR_OFF_INBOUND_0		(PF0_ATU_CAP_BaseAddress + 0x110)
#define	IATU_LIMIT_ADDR_DBIOFF_INBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x110)

/* Register IATU_LWR_TARGET_ADDR_OFF_INBOUND_0 */
/* iATU Lower Target Address Register. */
#define	IATU_LWR_TARGET_ADDR_OFF_INBOUND_0	(PF0_ATU_CAP_BaseAddress + 0x114)
#define	IATU_LWR_TARGET_ADDR_DBIOFF_INBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x114)
#define	IATU_UPPER_TARGET_ADDR_OFF_INBOUND_0	(PF0_ATU_CAP_BaseAddress + 0x118)
#define	IATU_UPPER_TARGET_ADDR_DBIOFF_INBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x118)
#define	IATU_UPPER_LIMIT_ADDR_OFF_INBOUND_0	(PF0_ATU_CAP_BaseAddress + 0x120)
#define	IATU_UPPER_LIMIT_ADDR_DBIOFF_INBOUND_0	(PF0_ATU_CAP_DBIBaseAddress + 0x120)

/* Register IATU_REGION_CTRL_1_OFF_OUTBOUND_1 */
/* iATU Region Control 1 Register. */
#define	IATU_REGION_CTRL_1_OFF_OUTBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x200)
#define	IATU_REGION_CTRL_1_DBIOFF_OUTBOUND_1	(PF0_ATU_CAP_DBIBaseAddress + 0x200)

/* Register IATU_REGION_CTRL_2_OFF_OUTBOUND_1 */
/* iATU Region Control 2 Register. */
#define	IATU_REGION_CTRL_2_OFF_OUTBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x204)
#define	IATU_REGION_CTRL_2_DBIOFF_OUTBOUND_1	(PF0_ATU_CAP_DBIBaseAddress + 0x204)

/* Register IATU_LWR_BASE_ADDR_OFF_OUTBOUND_1 */
/* iATU Lower Base Address Register. The CX_ATU_MIN_REGION_SIZE configuration parameter (Value Range: 4 kB, 8 kB, 16 kB, 32 kB, 64 kB defaults to 64 kB) specifies the minimum size of an address translation region. For example, if set to 64 kB; the lower 16 bits of the Base, Limit and Target registers are zero and all address regions are aligned on 64 kB boundaries. More precisely, the lower log2(CX_ATU_MIN_REGION_SIZE) bits are zero. */
#define	IATU_LWR_BASE_ADDR_OFF_OUTBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x208)
#define	IATU_LWR_BASE_ADDR_DBIOFF_OUTBOUND_1	(PF0_ATU_CAP_DBIBaseAddress + 0x208)

/* Register IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_1 */
/* iATU Upper Base Address Register. */
#define	IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x20c)
#define	IATU_UPPER_BASE_ADDR_DBIOFF_OUTBOUND_1	(PF0_ATU_CAP_DBIBaseAddress + 0x20c)

/* Register IATU_LIMIT_ADDR_OFF_OUTBOUND_1 */
/* iATU Limit Address Register. */
#define	IATU_LIMIT_ADDR_OFF_OUTBOUND_1		(PF0_ATU_CAP_BaseAddress + 0x210)
#define	IATU_LIMIT_ADDR_DBIOFF_OUTBOUND_1	(PF0_ATU_CAP_DBIBaseAddress + 0x210)

/* Register IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_1 */
/* iATU Lower Target Address Register. */
#define	IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x214)
#define	IATU_LWR_TARGET_ADDR_DBIOFF_OUTBOUND_1	(PF0_ATU_CAP_DBIBaseAddress + 0x214)

/* Register IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_1 */
/* iATU Upper Target Address Register. */
#define	IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x218)
#define	IATU_UPPER_TARGET_ADDR_DBIOFF_OUTBOUND_1	(PF0_ATU_CAP_DBIBaseAddress + 0x218)
#define	IATU_UPPER_LIMIT_ADDR_OFF_OUTBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x220)
#define	IATU_UPPER_LIMIT_ADDR_DBIOFF_OUTBOUND_1	(PF0_ATU_CAP_DBIBaseAddress + 0x220)

/* Register IATU_REGION_CTRL_1_OFF_INBOUND_1 */
/* iATU Region Control 1 Register. */
#define	IATU_REGION_CTRL_1_OFF_INBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x300)

/* Register IATU_REGION_CTRL_2_OFF_INBOUND_1 */
/* iATU Region Control 2 Register. */
#define	IATU_REGION_CTRL_2_OFF_INBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x304)

/* Register IATU_LWR_BASE_ADDR_OFF_INBOUND_1 */
/* iATU Lower Base Address Register. The CX_ATU_MIN_REGION_SIZE configuration parameter (Value Range: 4 kB, 8 kB, 16 kB, 32 kB, 64 kB defaults to 64 kB) specifies the minimum size of an address translation region. For example, if set to 64 kB; the lower 16 bits of the Base, Limit and Target registers are zero and all address regions are aligned on 64 kB boundaries. More precisely, the lower log2(CX_ATU_MIN_REGION_SIZE) bits are zero. */
#define	IATU_LWR_BASE_ADDR_OFF_INBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x308)

/* Register IATU_UPPER_BASE_ADDR_OFF_INBOUND_1 */
/* iATU Upper Base Address Register. */
#define	IATU_UPPER_BASE_ADDR_OFF_INBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x30c)

/* Register IATU_LIMIT_ADDR_OFF_INBOUND_1 */
/* iATU Limit Address Register. */
#define	IATU_LIMIT_ADDR_OFF_INBOUND_1		(PF0_ATU_CAP_BaseAddress + 0x310)

/* Register IATU_LWR_TARGET_ADDR_OFF_INBOUND_1 */
/* iATU Lower Target Address Register. */
#define	IATU_LWR_TARGET_ADDR_OFF_INBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x314)
#define	IATU_LWR_TARGET_ADDR_DBIOFF_INBOUND_1	(PF0_ATU_CAP_DBIBaseAddress + 0x314)
#define	IATU_UPPER_TARGET_ADDR_OFF_INBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x318)
#define	IATU_UPPER_TARGET_ADDR_DBIOFF_INBOUND_1	(PF0_ATU_CAP_DBIBaseAddress + 0x318)
#define	IATU_UPPER_LIMIT_ADDR_OFF_INBOUND_1	(PF0_ATU_CAP_BaseAddress + 0x320)
#define	IATU_UPPER_LIMIT_ADDR_DBIOFF_INBOUND_1	(PF0_ATU_CAP_DBIBaseAddress + 0x320)

/* Register IATU_REGION_CTRL_1_OFF_INBOUND_2 */
/* iATU Region Control 1 Register. */
#define	IATU_REGION_CTRL_1_OFF_INBOUND_2	(PF0_ATU_CAP_BaseAddress + 0x500)
#define	IATU_REGION_CTRL_1_DBIOFF_INBOUND_2	(PF0_ATU_CAP_DBIBaseAddress + 0x500)

/* Register IATU_REGION_CTRL_2_OFF_INBOUND_2 */
/* iATU Region Control 2 Register. */
#define	IATU_REGION_CTRL_2_OFF_INBOUND_2	(PF0_ATU_CAP_BaseAddress + 0x504)
#define	IATU_REGION_CTRL_2_DBIOFF_INBOUND_2	(PF0_ATU_CAP_DBIBaseAddress + 0x504)

/* Register IATU_LWR_BASE_ADDR_OFF_INBOUND_2 */
/* iATU Lower Base Address Register. The CX_ATU_MIN_REGION_SIZE configuration parameter (Value Range: 4 kB, 8 kB, 16 kB, 32 kB, 64 kB defaults to 64 kB) specifies the minimum size of an address translation region. For example, if set to 64 kB; the lower 16 bits of the Base, Limit and Target registers are zero and all address regions are aligned on 64 kB boundaries. More precisely, the lower log2(CX_ATU_MIN_REGION_SIZE) bits are zero. */
#define	IATU_LWR_BASE_ADDR_OFF_INBOUND_2	(PF0_ATU_CAP_BaseAddress + 0x508)
#define	IATU_LWR_BASE_ADDR_DBIOFF_INBOUND_2	(PF0_ATU_CAP_DBIBaseAddress + 0x508)

/* Register IATU_LIMIT_ADDR_OFF_INBOUND_2 */
/* iATU Limit Address Register. */
#define	IATU_LIMIT_ADDR_OFF_INBOUND_2		(PF0_ATU_CAP_BaseAddress + 0x510)
#define	IATU_LIMIT_ADDR_DBIOFF_INBOUND_2	(PF0_ATU_CAP_DBIBaseAddress + 0x510)

/* Register IATU_LWR_TARGET_ADDR_OFF_INBOUND_2 */
/* iATU Lower Target Address Register. */
#define	IATU_LWR_TARGET_ADDR_OFF_INBOUND_2	(PF0_ATU_CAP_BaseAddress + 0x514)
#define	IATU_LWR_TARGET_ADDR_DBIOFF_INBOUND_2	(PF0_ATU_CAP_DBIBaseAddress + 0x514)
#define	IATU_UPPER_TARGET_ADDR_OFF_INBOUND_2	(PF0_ATU_CAP_BaseAddress + 0x518)
#define	IATU_UPPER_TARGET_ADDR_DBIOFF_INBOUND_2	(PF0_ATU_CAP_DBIBaseAddress + 0x518)
#define	IATU_UPPER_LIMIT_ADDR_OFF_INBOUND_2	(PF0_ATU_CAP_BaseAddress + 0x520)
#define	IATU_UPPER_LIMIT_ADDR_DBIOFF_INBOUND_2	(PF0_ATU_CAP_DBIBaseAddress + 0x520)


extern	void dbg_pcie_Function(u32 para_1, u32 para_2, u32 para_3);

#endif
