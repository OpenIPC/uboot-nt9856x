#ifndef __ARCH_EFUSE_REG_PROTECTED_H
#define __ARCH_EFUSE_REG_PROTECTED_H

#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/rcw_macro.h>

#define EFUSE_LOT_ID1	0x1E
#define EFUSE_LOT_ID2	0x1F

REGDEF_BEGIN(EFUSE_LOT_ID1_REG)
REGDEF_BIT(lot_id_no_1_5_0, 6)
REGDEF_BIT(lot_id_no_2_5_0, 6)
REGDEF_BIT(lot_id_no_3_5_0, 6)
REGDEF_BIT(lot_id_no_4_5_0, 6)
REGDEF_BIT(lot_id_no_5_5_0, 6)
REGDEF_BIT(lot_id_no_6_1_0, 2)
REGDEF_END(EFUSE_LOT_ID1_REG)

REGDEF_BEGIN(EFUSE_LOT_ID2_REG)
REGDEF_BIT(lot_id_no_6_5_2, 4)
REGDEF_BIT(wafer_id, 5)
REGDEF_BIT(x_coordinate, 7)
REGDEF_BIT(y_coordinate, 7)
REGDEF_BIT(, 1)
REGDEF_END(EFUSE_LOT_ID2_REG)
#endif