/**
    NVT OPTees utilities for command customization

    @file       nvt_optee_utils.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __NVT_OPTEE_UTILS_H__
#define __NVT_OPTEE_UTILS_H__


#define NSEC_SHM	0x02800000
#define NSEC_SHM_SIZE	0x00400000

/******define from optee os original*******/


#define OPTEE_SMC_32                    0
#define OPTEE_SMC_64                    0x40000000
#define OPTEE_SMC_FAST_CALL             0x80000000
#define OPTEE_SMC_STD_CALL              0


#define OPTEE_SMC_OWNER_MASK            0x3F
#define OPTEE_SMC_OWNER_SHIFT           24

#define OPTEE_SMC_FUNC_MASK             0xFFFF

#define OPTEE_SMC_OWNER_ARCH            0
#define OPTEE_SMC_OWNER_CPU             1
#define OPTEE_SMC_OWNER_SIP             2
#define OPTEE_SMC_OWNER_OEM             3
#define OPTEE_SMC_OWNER_STANDARD        4
#define OPTEE_SMC_OWNER_TRUSTED_APP     48
#define OPTEE_SMC_OWNER_TRUSTED_OS      50

#define OPTEE_SMC_OWNER_TRUSTED_OS_OPTEED 62
#define OPTEE_SMC_OWNER_TRUSTED_OS_API  63

#define OPTEE_MSG_CMD_OPEN_SESSION      0
#define OPTEE_MSG_CMD_INVOKE_COMMAND    1
#define OPTEE_MSG_CMD_CLOSE_SESSION     2
#define OPTEE_MSG_CMD_CANCEL            3
#define OPTEE_MSG_CMD_REGISTER_SHM      4
#define OPTEE_MSG_CMD_UNREGISTER_SHM    5


#define OPTEE_SMC_RETURN_OK             0x0
#define OPTEE_SMC_RETURN_ETHREAD_LIMIT  0x1
#define OPTEE_SMC_RETURN_EBUSY          0x2
#define OPTEE_SMC_RETURN_ERESUME        0x3
#define OPTEE_SMC_RETURN_EBADADDR       0x4
#define OPTEE_SMC_RETURN_EBADCMD        0x5
#define OPTEE_SMC_RETURN_ENOMEM         0x6
#define OPTEE_SMC_RETURN_ENOTAVAIL      0x7



#define OPTEE_SMC_CALL_VAL(type, calling_convention, owner, func_num) \
                        ((type) | (calling_convention) | \
                        (((owner) & OPTEE_SMC_OWNER_MASK) << \
                                OPTEE_SMC_OWNER_SHIFT) |\
                        ((func_num) & OPTEE_SMC_FUNC_MASK))


/*******define from optee os smc_id_def.h************/

#define SMC_FUNC_ID_VERSION     0x0001

#define OPTEE_SMC_STD_CALL_VAL(func_num) \
        OPTEE_SMC_CALL_VAL(OPTEE_SMC_32, OPTEE_SMC_STD_CALL, \
                           OPTEE_SMC_OWNER_TRUSTED_OS, (func_num))


#define FUNCTION_NUM_GEN(type, module_id, func_num) \
                (((type & TYPE_MASK) << TYPE_SHIFT) | ((module_id & MODULE_MASK) << MODULE_SHIFT) | ((func_num & FUNC_MASK) << FUNC_SHIFT))

#define OEM_FAST_FUNC_GEN(type, module_id, func_num) \
                OPTEE_SMC_CALL_VAL(OPTEE_SMC_32, OPTEE_SMC_FAST_CALL, OPTEE_SMC_OWNER_OEM, FUNCTION_NUM_GEN(type, module_id, func_num))

#define OEM_STD_FUNC_GEN(type, module_id, func_num) \
                OPTEE_SMC_CALL_VAL(OPTEE_SMC_32, OPTEE_SMC_STD_CALL, OPTEE_SMC_OWNER_OEM, FUNCTION_NUM_GEN(type, module_id, func_num))


/* Function type */
#define TYPE_SHIFT                  14
#define TYPE_MASK                    3
#define SOFTWARE_TYPE                0
#define HW_TYPE                      1
#define CUSTOM_TYPE                  2

/* Module ID define */
#define MODULE_SHIFT                 8
#define MODULE_MASK               0x3F


/* Function number for each module */
#define FUNC_MASK                 0xFF
#define FUNC_SHIFT                   0

typedef enum {
	ID_0  =  0, ID_1 , ID_2 , ID_3 , ID_4 , ID_5 ,  ID_6 , ID_7 , ID_8 , ID_9 ,
	ID_10 = 10, ID_11, ID_12, ID_13, ID_14, ID_15,  ID_16, ID_17, ID_18, ID_19,
	ID_20 = 20, ID_21, ID_22, ID_23, ID_24, ID_25,  ID_26, ID_27, ID_28, ID_29,
	ID_30 = 30, ID_31, ID_32, ID_33, ID_34, ID_35,  ID_36, ID_37, ID_38, ID_39,
	ID_40 = 40, ID_41, ID_42, ID_43, ID_44, ID_45,  ID_46, ID_47, ID_48, ID_49,
	ID_50 = 50, ID_51, ID_52, ID_53, ID_54, ID_55,  ID_56, ID_57, ID_58, ID_59,
	ID_60 = 60, ID_61, ID_62, ID_63, ID_INVALID
} FUNCTION_ID;


#endif /* __NVT_OPTEE_UTILS_H__ */
