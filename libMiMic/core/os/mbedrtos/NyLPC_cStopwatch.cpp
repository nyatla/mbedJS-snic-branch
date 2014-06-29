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
#include "../NyLPC_cStopwatch.h"
#if NyLPC_ARCH==NyLPC_ARCH_MBEDRTOS
#include "mbed.h"
#include "rtos.h"

static Timer _timer;
static NyLPC_TBool _is_start=NyLPC_TBool_FALSE;
/**
 * 現在のtickCountを返します。
 */
NyLPC_TUInt32 NyLPC_cStopwatch_now(void)
{
    return (NyLPC_TUInt32)(_timer. read_ms());
}

/**
 * インスタンスを生成します。
 */
void NyLPC_cStopwatch_initialize(NyLPC_TcStopwatch_t* i_inst)
{
    if(!_is_start){
        _timer.start();
        _is_start=NyLPC_TBool_TRUE;
    }
    return;
}
/**
 * 基準時刻をセットします。
 * 値は、システム依存のtick値です。
 */
void NyLPC_cStopwatch_set(NyLPC_TcStopwatch_t* i_inst,NyLPC_TUInt32 i_initial)
{
    i_inst->_tick=i_initial;
    return;
}

/**
 * 基準時刻に、現在時刻をセットします。
 */
void NyLPC_cStopwatch_setNow(NyLPC_TcStopwatch_t* i_inst)
{
    i_inst->_tick=NyLPC_cStopwatch_now();
}
/**
 * タイムアウト計測を開始します。
 * この関数は、基準時刻に現在の時刻をセットします。
 */
void NyLPC_cStopwatch_startExpire(NyLPC_TcStopwatch_t* i_inst,NyLPC_TUInt32 i_timeout)
{
    NyLPC_cStopwatch_setNow(i_inst);
    i_inst->_ex_timeout=i_timeout;
}

/**
 * NyLPC_cStopwatch_startExpireで設定した時間を経過したかを返します。
 */
NyLPC_TBool NyLPC_cStopwatch_isExpired(NyLPC_TcStopwatch_t* i_inst)
{
    //経過時間の判定
    if(NyLPC_cStopwatch_elapseInMsec(i_inst)>i_inst->_ex_timeout){
        i_inst->_ex_timeout=0;
    }
    return (i_inst->_ex_timeout==0);
}

/**
 * 基準時刻と現在時刻の差を計算して、経過時間をミリ秒で返します。
 */
NyLPC_TUInt32 NyLPC_cStopwatch_elapseInMsec(const NyLPC_TcStopwatch_t* i_inst)
{
    return (NyLPC_cStopwatch_now()-i_inst->_tick);
}

#endif
