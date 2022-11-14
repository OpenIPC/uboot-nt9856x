/******************************************************************************
 *
 *  Copyright (C) 2009-2012 Realtek Corporation
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

/******************************************************************************
 *
 *  Filename:      userial_vendor.c
 *
 *  Description:   Contains vendor-specific userial functions
 *
 ******************************************************************************/
#include "osi_include_int.h"
#include "osi_include_ext.h"


/******************************************************************************
**  Constants & Macros
******************************************************************************/
#define VND_PORT_NAME_MAXLEN    256

/******************************************************************************
**  Local type definitions
******************************************************************************/


/******************************************************************************
**  Static variables
******************************************************************************/


/*****************************************************************************
**   Helper Functions
*****************************************************************************/

/*******************************************************************************
**
** Function        userial_to_tcio_baud
**
** Description     helper function converts USERIAL baud rates into TCIO
**                  conforming baud rates
**
** Returns         TRUE/FALSE
**
*******************************************************************************/
u8 userial_to_tcio_baud(u8 cfg_baud, u32 *baud)
{
    UNUSED(cfg_baud);
    UNUSED(baud);
    return 0;
}


/*****************************************************************************
**   Userial Vendor API Functions
*****************************************************************************/

/*******************************************************************************
**
** Function        userial_vendor_init
**
** Description     Initialize userial vendor-specific control block
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_init()
{
    return;
}


/*******************************************************************************
**
** Function        userial_vendor_open
**
** Description     Open the serial port with the given configuration
**
** Returns         device fd
**
*******************************************************************************/
int userial_vendor_open(tUSERIAL_CFG *p_cfg)
{
    UNUSED(p_cfg);
   return 0;
}

/*******************************************************************************
**
** Function        userial_vendor_close
**
** Description     Conduct vendor-specific close work
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_close(void)
{
    return;
}

/*******************************************************************************
**
** Function        userial_vendor_set_baud
**
** Description     Set new baud rate
**
** Returns         None
**
*******************************************************************************/
BOOLEAN userial_vendor_set_baud(u8 userial_baud)
{
    UNUSED(userial_baud);
    return TRUE;
}

/*******************************************************************************
**
** Function        userial_vendor_ioctl
**
** Description     ioctl inteface
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_ioctl(userial_vendor_ioctl_op_t op, void *p_data)
{
    UNUSED(op);
    UNUSED(p_data);
    return;
}

/*******************************************************************************
**
** Function        userial_set_port
**
** Description     Configure UART port name
**
** Returns         0 : Success
**                 Otherwise : Fail
**
*******************************************************************************/
int userial_set_port(char *p_conf_name, char *p_conf_value, int param)
{
    UNUSED(p_conf_name);
    UNUSED(p_conf_value);

    UNUSED(param);
    return 0;
}

/*******************************************************************************
**
** Function        userial_vendor_set_hw_fctrl
**
** Description     Conduct vendor-specific close work
**
** Returns         None
**
*******************************************************************************/
BOOLEAN userial_vendor_set_hw_fctrl(u8 hw_fctrl)
{
    UNUSED(hw_fctrl);
    return TRUE;
}

u32 userial_vendor_send_data(u8* data, u32 len)
{
    UNUSED(data);
    UNUSED(len);
    return 0;
}

u32 userial_vendor_receive_data(u8* data)
{
    UNUSED(data);
    return 0;
}


