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
 *
 * Parts of this file were leveraged from uIP:
 *
 * Copyright (c) 2001-2003, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef NYLPC_CIPV4ARP_H_
#define NYLPC_CIPV4ARP_H_
#include "../NyLPC_cIPv4Config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


struct NyLPC_TArpTableItem
{
    struct NyLPC_TIPv4Addr ipaddr;
    struct NyLPC_TEthAddr ethaddr;
    NyLPC_TUInt8 time;
};

/**********************************************************************
 *
 * class NyLPC_TcIPv4Config
 *
 **********************************************************************/


#define NyLPC_TcIPv4Arp_ARPTAB_SIZE 8


typedef struct NyLPC_TcIPv4Arp NyLPC_TcIPv4Arp_t;

/**
 * NyLPC_TcIPv4クラスの構造体です。
 */
struct NyLPC_TcIPv4Arp
{
    const NyLPC_TcIPv4Config_t* _cfg;
    NyLPC_TUInt8 arptime;
    NyLPC_TUInt8 tmpage;
    struct NyLPC_TArpTableItem arp_table[NyLPC_TcIPv4Arp_ARPTAB_SIZE];
};

void NyLPC_cIPv4Arp_initialize(NyLPC_TcIPv4Arp_t* i_inst,const NyLPC_TcIPv4Config_t* i_ref_config);
#define NyLPC_cIPv4Arp_finalize(i_inst)
void NyLPC_cIPv4Arp_periodic(NyLPC_TcIPv4Arp_t* i_inst);
void NyLPC_cIPv4Arp_incomingIp(NyLPC_TcIPv4Arp_t* i_inst,const struct NyLPC_TEthernetIIHeader* i_eth,struct NyLPC_TIPv4Addr i_ip_src);
/**
 * ARPメッセージを処理します。
 * @param o_tx_len
 * 戻り値がある場合そのサイズ
 * @return
 * 応答パケットを格納したcUipService_allocBufで確保したメモリ
 */
void* NyLPC_cIPv4Arp_rx(NyLPC_TcIPv4Arp_t* i_inst,const struct NyLPC_TArpHeader* i_arp, NyLPC_TUInt16 i_len, NyLPC_TUInt16* o_tx_len);
const struct NyLPC_TEthAddr* NyLPC_cIPv4Arp_IPv4toEthAddr(NyLPC_TcIPv4Arp_t* i_inst,const struct NyLPC_TIPv4Addr i_ip_addr);


#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* NYLPC_CIPV4ARP_H_ */
