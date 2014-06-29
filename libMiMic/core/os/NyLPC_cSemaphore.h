/*********************************************************************************
 * PROJECT: MiMic
 * --------------------------------------------------------------------------------
 *
 * This file is part of MiMic
 * Copyright (C)2011 Ryo Iizuka
 *
 * MiMic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by　the Free Software Foundation, either version 3 of the　License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information please contact.
 *  http://nyatla.jp/
 *  <airmail(at)ebony.plala.or.jp> or <nyatla(at)nyatla.jp>
 *
 *********************************************************************************/
#ifndef NYLPC_CSEMAPHORE_H_
#define NYLPC_CSEMAPHORE_H_
////////////////////////////////////////////////////////////////////////////////
// Include
////////////////////////////////////////////////////////////////////////////////
#include "NyLPC_stdlib.h"
#if NyLPC_ARCH==NyLPC_ARCH_FREERTOS
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#elif NyLPC_ARCH==NyLPC_ARCH_MBEDRTOS
#elif NyLPC_ARCH==NyLPC_ARCH_WIN32
    //not work!
#else
#error Bad NyLPC_ARCH!
#endif


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************
 *
 * NyLPC_cSemaphore class
 *
 **********************************************************************/
typedef struct NyLPC_TcSemaphore NyLPC_TcSemaphore_t;

#if NyLPC_ARCH==NyLPC_ARCH_FREERTOS

struct NyLPC_TcSemaphore
{
    xSemaphoreHandle _handle;
};
#elif NyLPC_ARCH==NyLPC_ARCH_MBEDRTOS
struct NyLPC_TcSemaphore
{
    void* _handle;//Semaphore*
};
#elif NyLPC_ARCH==NyLPC_ARCH_WIN32
//dumy definition!
struct NyLPC_TcSemaphore
{
    int dummy;
};
#else
#error Bad NyLPC_ARCH!
#endif


void NyLPC_cSemaphore_initialize(NyLPC_TcSemaphore_t* i_inst);
#define NyLPC_cSemaphore_finalize(i)
void NyLPC_cSemaphore_giveFromISR(const NyLPC_TcSemaphore_t* i_inst);
void NyLPC_cSemaphore_take(const NyLPC_TcSemaphore_t* i_inst,NyLPC_TUInt32 i_timeout);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CSEMAPHORE_H_ */



