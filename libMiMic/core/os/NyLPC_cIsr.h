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
#ifndef NYLPC_CISR_H_
#define NYLPC_CISR_H_
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
/**********************************************************************
 *
 * NyLPC_cIsr class
 *
 **********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (*NyLPC_cIsr_EventHandler)(unsigned long i_status);

/**
 * 全ての割込みとタスクスイッチを停止します。
 */
void NyLPC_cIsr_enterCritical(void);
/**
 * 全ての割込みとタスクスイッチを再開します。
 */
void NyLPC_cIsr_exitCritical(void);
/**
 * イベントハンドラを登録します。
 * @param i_handler
 * LPC17xxでは、イベントはISRからコールされます。
 */
void NyLPC_cIsr_setEnetISR(NyLPC_cIsr_EventHandler i_handler);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CISR_H_ */

