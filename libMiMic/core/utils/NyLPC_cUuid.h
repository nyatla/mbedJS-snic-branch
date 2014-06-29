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
#ifndef NYLPC_CUUID_H_
#define NYLPC_CUUID_H_
#include "NyLPC_stdlib.h"
#include "NyLPC_uipService.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct NyLPC_TcUuid NyLPC_TcUuid_t;
struct NyLPC_TcUuid
{
    NyLPC_TUInt32 f1;   //time-L
    NyLPC_TUInt16 f2;   //time-M
    NyLPC_TUInt16 f3;   //time-H version
    NyLPC_TUInt8  f4;   //SQ-H RES
    NyLPC_TUInt8  f5;   //SQ-L
    NyLPC_TUInt8  f6[6];//NODE
};
void NyLPC_cUuid_initialize(NyLPC_TcUuid_t* i_inst);
#define NyLPC_cUuid_finalize(i_inst)

void NyLPC_cUuid_setTimeBase(NyLPC_TcUuid_t* i_inst,NyLPC_TUInt32 i_time_l,NyLPC_TUInt32 i_time_h,NyLPC_TUInt16 i_seq,struct NyLPC_TEthAddr* eth_mac);

void NyLPC_cUuid_toString(NyLPC_TcUuid_t* i_inst,NyLPC_TChar* i_buf);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CUUID_H_ */
