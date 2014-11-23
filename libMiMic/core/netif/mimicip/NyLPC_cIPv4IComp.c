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
#include "NyLPC_cIPv4IComp_protected.h"
#include "NyLPC_cMiMicIpNetIf_protected.h"
#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO       8



NyLPC_TBool NyLPC_cIPv4IComp_initialize(
    NyLPC_TcIPv4IComp_t* i_inst,
    const NyLPC_TcIPv4Config_t* i_ref_config)
{
    i_inst->_ref_config=i_ref_config;
    return NyLPC_TBool_TRUE;
}
void NyLPC_cIPv4IComp_finalize(
    NyLPC_TcIPv4IComp_t* i_inst)
{
    return;
}
/**
 * ヘッダブロックの後ろにあるIPペイロードのアドレスを返します。
 */
#define NyLPC_TIPv4Header_getHeaderSize(i) (((i)->vhl & 0x0f)*4)

void* NyLPC_cIPv4IComp_rx(
    const NyLPC_TcIPv4IComp_t* i_inst,
    const NyLPC_TcIPv4Payload_t* i_ipp)
{
    NyLPC_TUInt16 tx_size;
    struct NyLPC_TIPv4Header* tx;
    struct NyLPC_TIcmpHeader* payload;
    const struct NyLPC_TIPv4Addr* my_ip=&(i_inst->_ref_config->ip_addr);

    if (NyLPC_TIPv4Addr_isEqual(&(i_inst->_ref_config->ip_addr),&NyLPC_TIPv4Addr_ZERO))
    {
        /* If we are configured to use ping IP address configuration and
         hasn't been assigned an IP address yet, we accept all ICMP
         packets. */
    } else {
        /* Check if the packet is destined for our IP address. */
        if (!NyLPC_TIPv4Addr_isEqual(&(i_ipp->header->destipaddr),my_ip))
        {
            return NyLPC_TBool_FALSE;
        }
    }
    // ICMP echo (i.e., ping) processing. This is simple, we only change the ICMP type from ECHO to ECHO_REPLY and adjust the ICMP checksum before we return the packet.
    if (i_ipp->payload.icmp->type != ICMP_ECHO)
    {
        return NyLPC_TBool_FALSE;
    }
    //返送パケットの取得
    tx=(struct NyLPC_TIPv4Header*)NyLPC_cMiMicIpNetIf_allocTxBuf(NyLPC_NTOHS(i_ipp->header->len16),&tx_size);
    if(tx==NULL){
        return NyLPC_TBool_FALSE;
    }
    //パケットサイズのチェック
    if(tx_size<NyLPC_NTOHS(i_ipp->header->len16)){
        NyLPC_cMiMicIpNetIf_releaseTxBuf(tx);
        return NyLPC_TBool_FALSE;
    }
    //返送パケットの構築
    ;

    //複製
    memcpy(tx,i_ipp->header,NyLPC_NTOHS(i_ipp->header->len16));

    //ペイロードの編集
    payload=(struct NyLPC_TIcmpHeader*)(((NyLPC_TUInt8*)tx)+NyLPC_TIPv4Header_getHeaderSize(tx));
    payload->type = ICMP_ECHO_REPLY;
    //update checksum
    if (payload->icmpchksum >= NyLPC_HTONS(0xffff - (ICMP_ECHO << 8))) {
        payload->icmpchksum += NyLPC_HTONS(ICMP_ECHO << 8) + 1;
    } else {
        payload->icmpchksum += NyLPC_HTONS(ICMP_ECHO << 8);
    }
    //IPヘッダの編集
    tx->destipaddr=tx->srcipaddr;
    tx->srcipaddr=*my_ip;
/*

    //チェックサムの再計算
    i_ipp->payload.icmp->type = ICMP_ECHO_REPLY;
    if (i_ipp->payload.icmp->icmpchksum >= NyLPC_HTONS(0xffff - (ICMP_ECHO << 8))) {
        i_ipp->payload.icmp->icmpchksum += NyLPC_HTONS(ICMP_ECHO << 8) + 1;
    } else {
        i_ipp->payload.icmp->icmpchksum += NyLPC_HTONS(ICMP_ECHO << 8);
    }
    //OUT/INアドレスの反転
    i_ipp->header->destipaddr=i_ipp->header->srcipaddr;
    i_ipp->header->srcipaddr=*my_ip;
*/
    return tx;

}
