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
#ifndef NYLPC_cStopwatch_H
#define NYLPC_cStopwatch_H

#include "NyLPC_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************
 *
 * NyLPC_TcStopwatch class
 *
 **********************************************************************/

typedef struct NyLPC_TcStopwatch NyLPC_TcStopwatch_t;
/**
 * このクラスは、経過時間を計算する関数を提供します。
 */
struct NyLPC_TcStopwatch
{
    NyLPC_TUInt32 _tick;
    NyLPC_TUInt32 _ex_timeout;
};
/**
 * 現在のtickCountを返します。
 */
NyLPC_TUInt32 NyLPC_cStopwatch_now(void);

void NyLPC_cStopwatch_initialize(NyLPC_TcStopwatch_t* i_inst);



#define NyLPC_cStopwatch_finalize(i)

void NyLPC_cStopwatch_set(NyLPC_TcStopwatch_t* i_inst,NyLPC_TUInt32 i_initial);

void NyLPC_cStopwatch_setNow(NyLPC_TcStopwatch_t* i_inst);

void NyLPC_cStopwatch_startExpire(NyLPC_TcStopwatch_t* i_inst,NyLPC_TUInt32 i_timeout);
NyLPC_TUInt32 NyLPC_cStopwatch_elapseInMsec(const NyLPC_TcStopwatch_t* i_inst);
/**
 * NyLPC_cStopwatch_startExpireで設定した時間を経過したかを返します。
 * @bug
 * NyLPC_cStopwatch_startExpireを実行しない状態で関数の戻り値は不定である。
 * 複数のスレッドで共有する場合は注意すること。
 */
NyLPC_TBool NyLPC_cStopwatch_isExpired(NyLPC_TcStopwatch_t* i_inst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
