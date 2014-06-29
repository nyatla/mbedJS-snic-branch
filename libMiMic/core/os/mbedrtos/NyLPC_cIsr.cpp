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
#include "../NyLPC_cIsr.h"
#if NyLPC_ARCH==NyLPC_ARCH_MBEDRTOS
#include "mbed.h"
#include "rtos.h"

void NyLPC_cIsr_enterCritical(void)
{
    __disable_irq();
}

/**
 * 全ての割込みとタスクスイッチを再開します。
 */
void NyLPC_cIsr_exitCritical(void)
{
    __enable_irq();
}

static NyLPC_cIsr_EventHandler _eth_irs=NULL;
void NyLPC_cIsr_setEnetISR(NyLPC_cIsr_EventHandler i_handler)
{
    _eth_irs=i_handler;
}

/**
 * 割込み解除を通知するセマフォ。lEMACInitで設定する。
 */
extern "C" void ENET_IRQHandler(void)
{
    unsigned long ulStatus;
    ulStatus = LPC_EMAC->IntStatus;
    /* Clear the interrupt. */
    LPC_EMAC->IntClear = ulStatus;
    if(_eth_irs!=NULL){
        _eth_irs(ulStatus);
    }

}
#endif
