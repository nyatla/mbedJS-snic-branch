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
#ifndef NYLPC_CBASESOCKET_H_
#define NYLPC_CBASESOCKET_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "NyLPC_stdlib.h"
#include "NyLPC_cIPv4_typedef.h"

/**
 * Base socket class
 * cIPv4 classが管理するソケットオブジェクトのベースクラスです。
 */
typedef struct NyLPC_TcBaseSocket NyLPC_TcBaseSocket_t;

#define NyLPC_TcBaseSocket_TYPEID_UDP_SOCK 1
#define NyLPC_TcBaseSocket_TYPEID_TCP_SOCK 2
#define NyLPC_TcBaseSocket_TYPEID_TCP_LISTENER 3

struct NyLPC_TcBaseSocket
{
    /**タイプID　継承クラスのinitializerで設定。 */
    NyLPC_TUInt8 _typeid;
    NyLPC_TUInt8 _padding8;
    NyLPC_TUInt16 _padding16;
    /** 所属してるIPv4コンローラ*/
    NyLPC_TcIPv4_t* _parent_ipv4;
};
void NyLPC_cBaseSocket_initialize(NyLPC_TcBaseSocket_t* i_inst,NyLPC_TUInt8 i_typeid);
#define NyLPC_cBaseSocket_finalize(i_inst)



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CBASESOCKET_H_ */
