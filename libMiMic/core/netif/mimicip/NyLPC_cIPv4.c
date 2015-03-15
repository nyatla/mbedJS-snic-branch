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
#include "NyLPC_cIPv4.h"
#include "NyLPC_stdlib.h"
#include "NyLPC_os.h"
#include "NyLPC_cIPv4Payload_protected.h"
#include "NyLPC_cMiMicIpTcpSocket_protected.h"
#include "NyLPC_cMiMicIpTcpListener_protected.h"
#include "NyLPC_cMiMicIpUdpSocket_protected.h"
#include "NyLPC_cIPv4IComp_protected.h"
#include "NyLPC_cMiMicIpNetIf_protected.h"





/****************************************************
 * NyLPC_cIPv4
 ***************************************************/

/**
 * Static関数
 */

static void* tcp_rx(
    NyLPC_TcIPv4_t* i_inst,
    const NyLPC_TcIPv4Payload_t* i_ipp);

static NyLPC_TBool udp_rx(
    NyLPC_TcIPv4_t* i_inst,
    NyLPC_TcIPv4Payload_t* i_ipp);

/**
 * See Header file.
 */
void NyLPC_cIPv4_initialize(
    NyLPC_TcIPv4_t* i_inst)
{
    //IP制御パケットの為に40バイト以上のシステムTXメモリが必要。
    NyLPC_ArgAssert(NyLPC_cMiMicIpNetIf_SYS_TX_BUF_SIZE>40);
    //instanceの初期化
    NyLPC_cMutex_initialize(&(i_inst->_sock_mutex));
    NyLPC_cMutex_initialize(&(i_inst->_listener_mutex));
    i_inst->tcp_port_counter=0;
    i_inst->_ref_config=NULL;
    return;
}

/**
 * See header file.
 */
void NyLPC_cIPv4_finalize(
    NyLPC_TcIPv4_t* i_inst)
{
    NyLPC_cMutex_finalize(&(i_inst->_sock_mutex));
    NyLPC_cMutex_finalize(&(i_inst->_listener_mutex));
    return;
}

/**
 * See header file.
 */
void NyLPC_cIPv4_start(
    NyLPC_TcIPv4_t* i_inst,
    const NyLPC_TcIPv4Config_t* i_ref_configlation)
{
    NyLPC_ArgAssert(i_ref_configlation!=NULL);
    //リストの初期化、ここでするべき？しないべき？
    i_inst->_ref_config=i_ref_configlation;
    //configulationのアップデートを登録されてるソケットに通知
    NyLPC_cMiMicIpNetIf_callSocketStart(i_ref_configlation);
    return;
}

/**
 * See header file.
 */
void NyLPC_cIPv4_stop(
    NyLPC_TcIPv4_t* i_inst)
{
	NyLPC_cMiMicIpNetIf_callSocketStop();
    i_inst->_ref_config=NULL;
    return;
}



#define IS_START(i_inst) ((i_inst)->_ref_config!=NULL)

/**
 * 稼動時に、1s置きに呼び出す関数です。
 */
void NyLPC_cIPv4_periodec(NyLPC_TcIPv4_t* i_inst)
{
	NyLPC_cMiMicIpNetIf_callPeriodic();
}


/**
 * IPv4ペイロードを処理する関数。
 * この関数は、パケット受信タスクから実行します。
 * @param i_rx
 * 先頭ポインタ。
 * @return
 * TRUEなら、i_rxに応答パケットをセットして返します。
 */
void* NyLPC_cIPv4_rx(NyLPC_TcIPv4_t* i_inst,const void* i_rx,NyLPC_TUInt16 i_rx_size)
{
    NyLPC_TcMiMicIpNetIf_t* inst=_NyLPC_TcMiMicIpNetIf_inst;
    NyLPC_TcIPv4Payload_t ipv4;
    //NOT開始状態なら受け付けないよ。
    if(!IS_START(i_inst)){
        NyLPC_OnErrorGoto(ERROR_DROP);
    }

    NyLPC_cIPv4Payload_initialize(&ipv4);
    //IPフラグメントを読出し用にセットする。
    if(!NyLPC_cIPv4Payload_attachRxBuf(&ipv4,i_rx,i_rx_size))
    {
        NyLPC_OnErrorGoto(ERROR_DROP);
    }
    switch(ipv4.header->proto)
    {
    case UIP_PROTO_TCP:
        //TCP受信処理
        return tcp_rx(i_inst,&ipv4);
    case UIP_PROTO_UDP:
        //UDP処理
        udp_rx(i_inst,&ipv4);//r
        return NyLPC_TBool_FALSE;
    case UIP_PROTO_ICMP:
        return NyLPC_cIPv4IComp_rx(&(inst->_icomp),&ipv4);
    }
    return NULL;
ERROR_DROP:
    return NULL;
}

NyLPC_TUInt16 NyLPC_cIPv4_getNewPortNumber(NyLPC_TcIPv4_t* i_inst)
{
    NyLPC_TUInt16 i,n;
    for(i=0;i<0x0fff;i--){
        i_inst->tcp_port_counter=(i_inst->tcp_port_counter+1)%0x0fff;
        n=i_inst->tcp_port_counter+49152;
        if(NyLPC_cMiMicIpNetIf_isClosedTcpPort(n))
        {
            return n;
        }
    }
    return 0;
}


/**********************************************************************
 *
 * packet handler
 *
 **********************************************************************/


static void* tcp_rx(
    NyLPC_TcIPv4_t* i_inst,
    const NyLPC_TcIPv4Payload_t* i_ipp)
{
    NyLPC_TcMiMicIpTcpSocket_t* sock;
    NyLPC_TcMiMicIpTcpListener_t* listener;

    //自分自身のIPに対する呼び出し？
    if(!NyLPC_TIPv4Addr_isEqual(&(i_ipp->header->destipaddr),&(i_inst->_ref_config->ip_addr)))
    {
        //自分以外のパケットはドロップ
        goto DROP;
    }
    //チェックサムの計算
    if((NyLPC_TIPv4Header_makeTcpChecksum(i_ipp->header) != 0xffff))
    {
        //受信エラーのあるパケットはドロップ
        goto DROP;
    }
    //アクティブなTCPソケットを探す。
    sock=NyLPC_cMiMicIpNetIf_getMatchTcpSocket(i_ipp->payload.tcp->destport,i_ipp->header->srcipaddr,i_ipp->payload.tcp->srcport);
    if(sock!=NULL)
    {
        //既存の接続を処理
        return NyLPC_cMiMicIpTcpSocket_parseRx(sock,i_ipp);
    }

    //このポートに対応したListenerを得る。
    listener=NyLPC_cMiMicIpNetIf_getListenerByPeerPort(i_ipp->payload.tcp->destport);
    if(listener==NULL){
        //Listen対象ではない。RST送信
        return NyLPC_cMiMicIpTcpSocket_allocTcpReverseRstAck(i_ipp);
    }
    //リスナにソケットのバインドを依頼する。
    NyLPC_cMiMicIpTcpListener_synPacket(listener,i_ipp);
    return NULL;//LISTEN成功。送信データなし
DROP:
    return NULL;
}



static NyLPC_TBool udp_rx(
    NyLPC_TcIPv4_t* i_inst,
    NyLPC_TcIPv4Payload_t* i_ipp)
{
    NyLPC_TcMiMicIpUdpSocket_t* sock=NULL;
    if(!NyLPC_TIPv4Addr_isEqual(&(i_ipp->header->destipaddr),&(i_inst->_ref_config->ip_addr)))
    {
        sock=NyLPC_cMiMicIpNetIf_getMatchUdpSocket(i_ipp->payload.udp->destport);
    }else{
        if(NyLPC_TIPv4Addr_isEqualWithMask(&(i_ipp->header->destipaddr),&NyLPC_TIPv4Addr_MULTICAST,&NyLPC_TIPv4Addr_MULTICAST_MASK)){
        //MultiCast?
            //マルチキャストに参加している&&portの一致するソケットを検索
            sock=NyLPC_cMiMicIpNetIf_getMatchMulticastUdpSocket(&(i_ipp->header->destipaddr),i_ipp->payload.udp->destport);
        }else if(!NyLPC_TIPv4Addr_isEqual(&(i_ipp->header->destipaddr),&NyLPC_TIPv4Addr_BROADCAST)){
        //Broadcast?
            sock=NyLPC_cMiMicIpNetIf_getMatchUdpSocket(i_ipp->payload.udp->destport);
        }
    }
    if(sock==NULL)
    {
        goto DROP;
    }
    //パケットのエラーチェック
    if((NyLPC_TIPv4Header_makeTcpChecksum(i_ipp->header) != 0xffff))
    {
        //受信エラーのあるパケットはドロップ
        goto DROP;
    }
    //既存の接続を処理
    return NyLPC_cMiMicIpUdpSocket_parseRx(sock,i_ipp);
DROP:
    return NyLPC_TBool_FALSE;
}


