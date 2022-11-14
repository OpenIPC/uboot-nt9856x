#ifndef __ASM_NVT_COMMON_NVT_IVOT_OPTEE_H
#define __ASM_NVT_COMMON_NVT_IVOT_OPTEE_H
#include <asm/nvt-common/nvt_types.h>
#include <asm/arch/crypto.h>

typedef enum _NVT_IVOT_OPTEE_RSA_TYPE {

	NVT_IVOT_OPTEE_RSA_1024 = 1024,
	NVT_IVOT_OPTEE_RSA_2048 = 2048

} NVT_IVOT_OPTEE_RSA_TYPE;



//////optee util cmd function

int nvt_ivot_l2_cache(int enable);

int nvt_ivot_actlr_smp_cfg(int enable);

int nvt_ivot_l2_aux_ctrl_cfg(unsigned long val);

#endif /* __ASM_NVT_COMMON_NVT_IVOT_OPTEE_H */
