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
#include "NyLPC_cNet.h"
#include "dhcp/NyLPC_cDhcpClient.h"
#include "../uip/NyLPC_cUipService_protected.h"



void NyLPC_cNet_initialize(NyLPC_TcNet_t* i_inst)
{
    //uipサービス初期化。いろいろ利用可能に。
    if(!NyLPC_TcUipService_isInitService()){
        NyLPC_cUipService_initialize();
    }
}

void NyLPC_cNet_start(NyLPC_TcNet_t* i_inst,const NyLPC_TcNetConfig_t* i_ref_config)
{
    NyLPC_cUipService_start(&(i_ref_config->super));
    return;
}

void NyLPC_cNet_stop(NyLPC_TcNet_t* i_inst)
{
    NyLPC_cUipService_stop();
    return;
}

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
NyLPC_TBool NyLPC_cNet_requestAddrDhcp(NyLPC_TcNet_t* i_net,NyLPC_TcIPv4Config_t* i_cfg,NyLPC_TInt16 i_repeat)
{
    NyLPC_TBool ret;
    NyLPC_TcDhcpClient_t sock;
    //netを開始
    NyLPC_cDhcpClient_initialize(&sock);
    ret=NyLPC_cDhcpClient_requestAddr(&sock,i_cfg,i_repeat);
    NyLPC_cDhcpClient_finalize(&sock);
    return ret;
}
/**
 * NyLPC_TcIPv4Config_tをAPIPAで更新します。
 * この関数をコールする時は、サービスは停止中でなければなりません。
 * @param i_cfg
 * 更新するi_cfg構造体。
 * emac,default_mssは設定済である必要があります。他のフィールド値は不定で構いません。
 * 更新されるフィールドは、ip,netmast,default_rootの3つです。
 * @return
 * 更新に成功した場合TRUE
 */
NyLPC_TBool NyLPC_cNet_requestAddrApipa(NyLPC_TcNet_t* i_net,NyLPC_TcIPv4Config_t* i_cfg,NyLPC_TInt16 i_repeat)
{
    NyLPC_TBool ret;
    NyLPC_TcApipa_t sock;
    //netを開始
    NyLPC_cApipa_initialize(&sock);
    ret=NyLPC_cApipa_requestAddr(&sock,i_cfg,i_repeat);
    NyLPC_cApipa_finalize(&sock);
    return ret;
}
