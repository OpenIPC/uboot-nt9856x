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
#ifndef RTK_H5_INT_H__
#define RTK_H5_INT_H__

/***********************************************
****    internal include*****************************/
#include "osi_data_types.h"
#include "hci_common_int.h"

/************************************************/
#define TIMER_H5_TOTAL_NUM_MAX             16

#define TIMER_H5_HW_INIT_READY              1
#define TIMER_H5_SYNC_RETRANS               2
#define TIMER_H5_CONF_RETRANS               4
#define TIMER_H5_DATA_RETRANS               8
#define TIMER_H5_WAIT_CT_BAUDRATE_READY     16


#define DATA_RETRANS_COUNT  40  //40*100 = 4000ms(4s)
#define SYNC_RETRANS_COUNT  20  //20*250 = 5000ms(5s)
#define CONF_RETRANS_COUNT  20


#define DATA_RETRANS_TIMEOUT_VALUE              100 //ms
#define SYNC_RETRANS_TIMEOUT_VALUE              250
#define CONF_RETRANS_TIMEOUT_VALUE              250
#define H5_HW_INIT_READY_TIMEOUT_VALUE          10000//4

const tHCI_IF *hci_get_h5_interface(void);

#endif
