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
#ifndef NYLPC_CIPV4PAYLOAD_PROTECTED_H_
#define NYLPC_CIPV4PAYLOAD_PROTECTED_H_

#include "NyLPC_cIPv4Payload.h"
#include "NyLPC_cIPv4.h"
#include "NyLPC_cTcpSocket.h"
#include "NyLPC_cUdpSocket.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20
#define TCP_CTL 0x3f
#define UIP_TCPH_LEN   20    /* Size of TCP header */
#define UIP_IPH_LEN    20    /* Size of IP header */



void NyLPC_cIPv4Payload_initialize(NyLPC_TcIPv4Payload_t* i_inst);
#define NyLPC_cIPv4Payload_finalize(i)

/**
 * アタッチされているバッファを返します。
 */
#define NyLPC_cIPv4Payload_getBuf(i) ((i)->header)



void NyLPC_cIPv4Payload_attachTxBuf(NyLPC_TcIPv4Payload_t* i_inst,void* i_buf);
const void* NyLPC_cIPv4Payload_detachBuf(NyLPC_TcIPv4Payload_t* i_inst);

NyLPC_TBool NyLPC_cIPv4Payload_attachRxBuf(NyLPC_TcIPv4Payload_t* i_inst,const void* i_buf,NyLPC_TUInt16 i_flagment_size);
void NyLPC_cIPv4Payload_setTcpReverseRstAck(
    NyLPC_TcIPv4Payload_t* i_inst);
void NyLPC_cIPv4Payload_setTcpReverseRstAck2(
    NyLPC_TcIPv4Payload_t* i_inst,
    const NyLPC_TcIPv4Payload_t* i_src);




/**
 * UDPの送信バッファを初期化します。
 */
void* NyLPC_cIPv4Payload_initUdpTx(NyLPC_TcIPv4Payload_t* i_inst,NyLPC_TUInt8 i_iph_word,NyLPC_TUInt16 i_tcp_payload_size);
/**
 * UDPの送信情報を設定します。
 */
void NyLPC_cIPv4Payload_setUdpTxHeaderByConnection(NyLPC_TcIPv4Payload_t* i_inst,const struct uip_udp_conn* i_conn,const struct NyLPC_TIPv4Addr* i_dest_ip,NyLPC_TUInt16 i_dest_port);

void NyLPC_cIPv4Payload_closeUdpTxPacket(
    NyLPC_TcIPv4Payload_t* i_inst);


void NyLPC_TcIPv4TxPacket_initialize_icomp(NyLPC_TcIPv4Payload_t* i_inst,void* i_buf,NyLPC_TUInt8 i_iph_word,NyLPC_TUInt16 i_tcp_payload_size);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CIPV4PAYLOAD_PROTECTED_H_ */
