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
#ifndef NYLPC_CMUTEX_H_
#define NYLPC_CMUTEX_H_
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
    //not implemented!
#else
#error Bad NyLPC_ARCH!
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************
 *
 * NyLPC_TcMutex class
 *
 **********************************************************************/
typedef struct NyLPC_TcMutex NyLPC_TcMutex_t;

#if NyLPC_ARCH==NyLPC_ARCH_FREERTOS

struct NyLPC_TcMutex
{
    xQueueHandle _mutex;
    NyLPC_TUInt8 _lock_count;
};
#elif NyLPC_ARCH==NyLPC_ARCH_MBEDRTOS
struct NyLPC_TcMutex
{
    void* _mutex;//rtos::Mutex*
    NyLPC_TUInt8 _lock_count;
};
#elif NyLPC_ARCH==NyLPC_ARCH_WIN32
//not work!
struct NyLPC_TcMutex
{
    int dummy;
};
#endif

NyLPC_TBool NyLPC_cMutex_initialize(NyLPC_TcMutex_t* i_inst);

NyLPC_TBool NyLPC_cMutex_lock(NyLPC_TcMutex_t* i_inst);

NyLPC_TBool NyLPC_cMutex_unlock(NyLPC_TcMutex_t* i_inst);

#define NyLPC_cMutex_isLocked(i_inst) ((i_inst)->_lock_count>0)
#define NyLPC_cMutex_finalize(i_inst)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CMUTEX_H_ */
