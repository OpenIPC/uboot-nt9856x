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

#ifndef INCLUDE_EXT_H
#define INCLUDE_EXT_H

#include <common.h>
//#include <string.h>
//#include <stdint.h>
//#include <stdbool.h>
//#include <stdio.h>
//#include <unistd.h>
#define MEMORY_STATIC_ALLOC FALSE
#if (!MEMORY_STATIC_ALLOC)
#include <stdlib.h>
#endif


//debug include
//#include <time.h>
//#include <errno.h>
//#include <termios.h>
//#include <fcntl.h>

#define RTKBT_DEBUG 1

#define RTK_ERROR(fmt, args...)	    printf(fmt, ##args)
#define RTK_INFO(fmt, args...)      printf(fmt, ##args)

#if RTKBT_DEBUG
#define RTK_DEBUG(fmt, args...)	    printf(fmt, ##args)
#define RTK_WARN(fmt, args...)	    printf(fmt, ##args)
#define RTK_VERBOSE(fmt, args...)	printf(fmt, ##args)
#else
#define RTK_DEBUG(fmt, args...)
#define RTK_WARN(fmt, args...)
#define RTK_VERBOSE(fmt, args...)
#endif
#endif
