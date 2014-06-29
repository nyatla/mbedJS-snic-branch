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
#include "NyLPC_uip.h"
#include "NyLPC_cIPv4Payload_protected.h"
#include "NyLPC_cIPv4Config.h"

/**　イーサネットヘッダのサイズ値*/
#define UIP_ETHERHEADER_LEN    14

/**
 * See header file.
 */
void NyLPC_cIPv4Config_initialzeForEthernet(NyLPC_TcIPv4Config_t* i_inst,const struct NyLPC_TEthAddr* i_ether_addr,NyLPC_TUInt16 i_ether_frame_len)
{
    i_inst->ip_addr=i_inst->netmask=i_inst->dr_addr=NyLPC_TIPv4Addr_ZERO;
    i_inst->eth_mac=*i_ether_addr;
    //mssの計算
    i_inst->default_mss=i_ether_frame_len-(UIP_ETHERHEADER_LEN+UIP_TCPH_LEN + UIP_IPH_LEN);
    return;
}
void NyLPC_cIPv4Config_initialzeCopy(NyLPC_TcIPv4Config_t* i_inst,const NyLPC_TcIPv4Config_t* i_src)
{
    memcpy(i_inst,i_src,sizeof(NyLPC_TcIPv4Config_t));
}
/**
 * See header file.
 */
void NyLPC_cIPv4Config_setDefaultRoute(NyLPC_TcIPv4Config_t* i_inst,const struct NyLPC_TIPv4Addr* i_dr_addr)
{
    i_inst->dr_addr=*i_dr_addr;
    return;
}

/**
 * See header file.
 */
void NyLPC_cIPv4Config_setIp(NyLPC_TcIPv4Config_t* i_inst,const struct NyLPC_TIPv4Addr* i_ipaddr,const struct NyLPC_TIPv4Addr* i_netmask)
{
    i_inst->ip_addr=*i_ipaddr;
    i_inst->netmask=*i_netmask;
    return;
}

/**
 * See header file.
 */
NyLPC_TBool NyLPC_cIPv4Config_isLocalIP(const NyLPC_TcIPv4Config_t* i_inst,const struct NyLPC_TIPv4Addr* i_target_ip)
{
    return NyLPC_TIPv4Addr_isEqualWithMask(&(i_inst->ip_addr),i_target_ip,&(i_inst->netmask));
}
