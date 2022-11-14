/******************************************************************************
 *
 *  Copyright (C) 2014 Google, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#ifndef RTK_COMMON_INT_H__
#define RTK_COMMON_INT_H__

/***********************************************
****    internal include*****************************/
#include "osi_data_types.h"

/************************************************/

#define  HCI_H4     1
#define  HCI_H5     2

/* Message event mask across Host/Controller lib and stack */
#define MSG_EVT_MASK                    0xFF00 /* eq. BT_EVT_MASK */
#define MSG_SUB_EVT_MASK                0x00FF /* eq. BT_SUB_EVT_MASK */

/* Message event ID passed from Host/Controller lib to stack */
#define MSG_HC_TO_STACK_HCI_ERR        0x1300 /* eq. BT_EVT_TO_BTU_HCIT_ERR */
#define MSG_HC_TO_STACK_HCI_ACL        0x1100 /* eq. BT_EVT_TO_BTU_HCI_ACL */
#define MSG_HC_TO_STACK_HCI_SCO        0x1200 /* eq. BT_EVT_TO_BTU_HCI_SCO */
#define MSG_HC_TO_STACK_HCI_EVT        0x1000 /* eq. BT_EVT_TO_BTU_HCI_EVT */
#define MSG_HC_TO_STACK_L2C_SEG_XMIT   0x1900 /* eq. BT_EVT_TO_BTU_L2C_SEG_XMIT */

/* Message event ID passed from stack to vendor lib */
#define MSG_STACK_TO_HC_HCI_ACL        0x2100 /* eq. BT_EVT_TO_LM_HCI_ACL */
#define MSG_STACK_TO_HC_HCI_SCO        0x2200 /* eq. BT_EVT_TO_LM_HCI_SCO */
#define MSG_STACK_TO_HC_HCI_CMD        0x2000 /* eq. BT_EVT_TO_LM_HCI_CMD */

typedef enum {
    DATA_TYPE_UNKNOWN = 0,
    DATA_TYPE_COMMAND = 1,
    DATA_TYPE_ACL = 2,
    DATA_TYPE_SCO = 3,
    DATA_TYPE_EVENT = 4
} serial_data_type_t;

enum {
    RTKBT_PACKET_IDLE,
    RTKBT_PACKET_TYPE,
    RTKBT_PACKET_HEADER,
    RTKBT_PACKET_CONTENT,
    RTKBT_PACKET_END
};

/******************************************************************************
**  Type definitions
******************************************************************************/
    // 2 bytes for opcode, 1 byte for parameter length (Volume 2, Part E, 5.4.1)
#define COMMAND_PREAMBLE_SIZE 3
    // 2 bytes for handle, 2 bytes for data length (Volume 2, Part E, 5.4.2)
#define ACL_PREAMBLE_SIZE 4
    // 2 bytes for handle, 1 byte for data length (Volume 2, Part E, 5.4.3)
#define SCO_PREAMBLE_SIZE 3
    // 1 byte for event code, 1 byte for parameter length (Volume 2, Part E, 5.4.4)
#define EVENT_PREAMBLE_SIZE 2

#define HCI_PACKET_TYPE_TO_INDEX(type) ((type) - 1)

#define COMMON_DATA_LENGTH_INDEX 3

#define EVENT_DATA_LENGTH_INDEX 2

#define WAIT_CT_BAUDRATE_READY_TIMEOUT_VALUE    250

/* Maximum numbers of allowed internal
** outstanding command packets at any time
*/
#define INT_CMD_PKT_MAX_COUNT       8
#define INT_CMD_PKT_IDX_MASK        0x07


typedef struct
{
    u16         total_num;
    u16         timer_event;
} rtk_timer_t;


typedef struct
{
    u16          event;
    u16          len;
    u16          offset;
    u16          layer_specific;
    u8           data[];
} HC_BT_HDR;

#define BT_HC_HDR_SIZE (sizeof(HC_BT_HDR))


/* Callback function for the returned event of internal issued command */
typedef void (*tINT_CMD_CBACK)(void *p_mem);

/* Initialize transport's control block */
typedef void (*tHCI_INIT)(void);

/* Do transport's control block clean-up */
typedef void (*tHCI_CLEANUP)(void);

/* Handler for HCI upstream path */
typedef bool (*tHCI_RCV_AND_CHK_TIMER)(void);

/* Handler for sending HCI command from the local module */
typedef u8 (*tHCI_SEND_INT)(u16 opcode, HC_BT_HDR *p_buf, tINT_CMD_CBACK p_cback);

typedef void* (*tHCI_ALLOC)(int size);

/* datapath buffer deallocation callback (callout) */
typedef void (*tHCI_DEALLOC)(void *p_buf);

typedef struct
{
    u16 opcode;        /* OPCODE of outstanding internal commands */
    tINT_CMD_CBACK cback;   /* Callback function when return of internal * command is received */
} tINT_CMD_Q;

/******************************************************************************
**  Extern variables and functions
******************************************************************************/
typedef struct {
    tHCI_INIT               init;
    tHCI_CLEANUP            cleanup;
    tHCI_SEND_INT           send_int_cmd;
    tHCI_RCV_AND_CHK_TIMER  rcv_and_chk_timer;
    tHCI_ALLOC              alloc;
    tHCI_DEALLOC            dealloc;
} tHCI_IF;

#define HCI_VERSION_MASK_10     (1<<0)     //Bluetooth Core Spec 1.0b
#define HCI_VERSION_MASK_11     (1<<1)     //Bluetooth Core Spec 1.1
#define HCI_VERSION_MASK_12     (1<<2)     //Bluetooth Core Spec 1.2
#define HCI_VERSION_MASK_20     (1<<3)     //Bluetooth Core Spec 2.0+EDR
#define HCI_VERSION_MASK_21     (1<<4)     //Bluetooth Core Spec 2.1+EDR
#define HCI_VERSION_MASK_30     (1<<5)     //Bluetooth Core Spec 3.0+HS
#define HCI_VERSION_MASK_40     (1<<6)     //Bluetooth Core Spec 4.0
#define HCI_VERSION_MASK_41     (1<<7)     //Bluetooth Core Spec 4.1
#define HCI_VERSION_MASK_42     (1<<8)     //Bluetooth Core Spec 4.2
#define HCI_VERSION_MASK_ALL    (0xFFFFFFFF)

#define HCI_REVISION_MASK_ALL   (0xFFFFFFFF)

#define LMP_SUBVERSION_NONE     (0x0)

#define CHIPTYPE_NONE           (0x1F)      //Chip Type's range: 0x0 ~ 0xF
#define CHIP_TYPE_MASK_ALL      (0xFFFFFFFF)

#define PROJECT_ID_MASK_ALL     (0xFFFFFFFF)    // temp used for unknow project id for a new chip

#define PATCH_OPTIONAL_MATCH_FLAG_CHIPTYPE   (0x1)

#define CONFIG_MAC_OFFSET_GEN_1_2       (0x3C)      //MAC's OFFSET in config/efuse for realtek generation 1~2 bluetooth chip
#define CONFIG_MAC_OFFSET_GEN_3PLUS     (0x44)      //MAC's OFFSET in config/efuse for rtk generation 3+ bluetooth chip

#endif
