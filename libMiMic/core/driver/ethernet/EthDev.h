/*
 * @file:    EthDev.h
 * @purpose: Ethernet Device Definitions
 * @version: V1.10
 * @date:    24. Feb. 2009
 *----------------------------------------------------------------------------
 *
 * Copyright (C) 2009 ARM Limited. All rights reserved.
 *
 * ARM Limited (ARM) is supplying this software for use with Cortex-M3
 * processor based microcontrollers.  This file can be freely distributed
 * within development tools that are supporting such ARM based processors.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 */
/*
 * Modified for MiMic by R.Iizuka. 2011.08.27
 * http://nyatla.jp/mimic
 */

#ifndef _ETHDEV__H
#define _ETHDEV__H
#include "NyLPC_stdlib.h"
#include "NyLPC_IEthernetDevice.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

const struct TiEthernetDevice* getEthernetDevicePnP(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

