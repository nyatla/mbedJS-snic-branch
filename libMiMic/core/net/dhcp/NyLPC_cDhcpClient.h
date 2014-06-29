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
#ifndef NYLPC_CDHCPCLIENT_H_
#define NYLPC_CDHCPCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "NyLPC_net.h"

typedef struct NyLPC_TcDhcpClient NyLPC_TcDhcpClient_t;

struct NyLPC_TcDhcpClient{
    NyLPC_TcUdpSocket_t super;
    NyLPC_TcIPv4Config_t* _result;
    NyLPC_TUInt32 txid;
    volatile NyLPC_TUInt16 _status;
    //offer情報
    struct NyLPC_TIPv4Addr _offerserver;
};


/**
 * DHCPソケットを作成します。
 */
NyLPC_TBool NyLPC_cDhcpClient_initialize(NyLPC_TcDhcpClient_t* i_inst);

void NyLPC_cDhcpClient_finalize(NyLPC_TcDhcpClient_t* i_inst);

/**
 * NyLPC_TcIPv4Config_tをDHCPで更新します。
 * この関数をコールする時は、サービスは停止中でなければなりません。
 * @param i_cfg
 * 更新するi_cfg構造体。
 * emac,default_mssは設定済である必要があります。他のフィールド値は不定で構いません。
 * 更新されるフィールドは、ip,netmast,default_rootの3つです。
 * @return
 * 更新に成功した場合TRUE
 */
NyLPC_TBool NyLPC_cDhcpClient_requestAddr(NyLPC_TcDhcpClient_t* i_inst,NyLPC_TcIPv4Config_t* i_cfg,NyLPC_TInt16 i_repeat);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
