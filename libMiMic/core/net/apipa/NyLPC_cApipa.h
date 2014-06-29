/*********************************************************************************
 * PROJECT: MiMic
 * --------------------------------------------------------------------------------
 *
 * This file is part of MiMic
 * Copyright (C)2011-2013 Ryo Iizuka
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
#ifndef NYLPC_CAPIPA_H_
#define NYLPC_CAPIPA_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "NyLPC_stdlib.h"
#include "NyLPC_net.h"

typedef struct NyLPC_cApipa NyLPC_TcApipa_t;

struct NyLPC_cApipa
{
    NyLPC_TUInt32 _seed;
};
void NyLPC_cApipa_initialize(NyLPC_TcApipa_t* i_inst);

#define NyLPC_cApipa_finalize(i_inst)


/**
 * i_cfgにAutoIPのアドレスを取得します。
 * この関数はuipを操作します。uipServiceは停止中である必要があります。
 */
NyLPC_TBool NyLPC_cApipa_requestAddr(NyLPC_TcApipa_t* i_inst,NyLPC_TcIPv4Config_t* i_cfg,NyLPC_TInt16 i_repeat);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* NYLPC_CAPIPA_H_ */
