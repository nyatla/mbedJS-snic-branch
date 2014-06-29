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
#include "NyLPC_stdlib.h"
#include "NyLPC_uip.h"
#include "NyLPC_cIPv4.h"
#include "NyLPC_cIPv4Payload_protected.h"
#include "NyLPC_cTcpSocket_protected.h"
#include "NyLPC_cUdpSocket_protected.h"
#include "NyLPC_cTcpListener_protected.h"
#include "NyLPC_cIPv4IComp_protected.h"
#include "NyLPC_cUipService_protected.h"



/****************************************************
 * Socketテーブルに関する宣言
 ***************************************************/

#define cSocketTbl_initialize(i_inst,buf) NyLPC_cPtrTbl_initialize(i_inst,buf,NyLPC_cIPv4_MAX_SOCKET)
#define cSocketTbl_finalize(i_inst)

/**
 * 条件に一致する、アクティブなTCPソケットオブジェクトを取得します。
 * この関数は、ローカルIPが一致していると仮定して検索をします。
 * @param i_rip
 * リモートIPアドレスを指定します。
 */
static NyLPC_TcTcpSocket_t* cSocketTbl_getMatchTcpSocket(
    NyLPC_TcPtrTbl_t* i_inst,
    NyLPC_TUInt16 i_lport,
    struct NyLPC_TIPv4Addr i_rip,
    NyLPC_TUInt16 i_rport)
{
    NyLPC_TcBaseSocket_t** p=(NyLPC_TcBaseSocket_t**)(i_inst->buf);
    NyLPC_TcTcpSocket_t* tp;
    int i;
    //一致するポートを検索
    for(i=i_inst->size-1;i>=0;i--){
        if(p[i]==NULL || p[i]->_typeid!=NyLPC_TcBaseSocket_TYPEID_TCP_SOCK){
            continue;
        }
        tp=(NyLPC_TcTcpSocket_t*)p[i];
        if(tp->tcpstateflags==UIP_CLOSED){
            continue;
        }
        //パラメータの一致チェック
        if(i_lport!=tp->uip_connr.lport || i_rport!= tp->uip_connr.rport || i_rip.v!=tp->uip_connr.ripaddr.v)
        {
            continue;
        }
        return tp;
    }
    return NULL;
}
static NyLPC_TcUdpSocket_t* cSocketTbl_getMatchUdpSocket(
    NyLPC_TcPtrTbl_t* i_inst,
    NyLPC_TUInt16 i_lport)
{
    NyLPC_TcBaseSocket_t** p=(NyLPC_TcBaseSocket_t**)(i_inst->buf);
    NyLPC_TcUdpSocket_t* tp;
    int i;
    //一致するポートを検索
    for(i=i_inst->size-1;i>=0;i--){
        if(p[i]==NULL || p[i]->_typeid!=NyLPC_TcBaseSocket_TYPEID_UDP_SOCK){
            continue;
        }
        tp=(NyLPC_TcUdpSocket_t*)p[i];
        //パラメータの一致チェック
        if(i_lport==tp->uip_udp_conn.lport){
        //unicast
            return tp;
        }
    }
    return NULL;
}
static NyLPC_TcUdpSocket_t* cSocketTbl_getMatchMulticastUdpSocket(
    NyLPC_TcPtrTbl_t* i_inst,
    const struct NyLPC_TIPv4Addr* i_mcast_ip,
    NyLPC_TUInt16 i_lport)
{
    NyLPC_TcBaseSocket_t** p=(NyLPC_TcBaseSocket_t**)(i_inst->buf);
    NyLPC_TcUdpSocket_t* tp;
    int i;
    //一致するポートを検索
    for(i=i_inst->size-1;i>=0;i--){
        if(p[i]==NULL || p[i]->_typeid!=NyLPC_TcBaseSocket_TYPEID_UDP_SOCK){
            continue;
        }
        tp=(NyLPC_TcUdpSocket_t*)p[i];
        //パラメータの一致チェック
        if(i_lport!=tp->uip_udp_conn.lport || (!NyLPC_TIPv4Addr_isEqual(i_mcast_ip,&(tp->uip_udp_conn.mcastaddr))))
        {
            continue;
        }
        return tp;
    }
    return NULL;
}

/**
 * i_port番号に一致するリスナを返します。
 */
static NyLPC_TcTcpListener_t* cSocketTbl_getListenerByPeerPort(NyLPC_TcPtrTbl_t* i_inst,NyLPC_TUInt16 i_port)
{
    NyLPC_TcBaseSocket_t** p=(NyLPC_TcBaseSocket_t**)(i_inst->buf);
    NyLPC_TcTcpListener_t* lp;
    int i;
    //一致するポートを検索して、acceptをコールする。
    for(i=i_inst->size-1;i>=0;i--){
        if(p[i]==NULL || p[i]->_typeid!=NyLPC_TcBaseSocket_TYPEID_TCP_LISTENER){
            continue;
        }
        lp=(NyLPC_TcTcpListener_t*)p[i];
        if(lp->_port!=i_port){
            continue;
        }
        return lp;
    }
    return NULL;
}
/**
 * 指定番号のTCPポートが未使用かを返す。
 * @return
 * i_lport番のポートが未使用であればTRUE
 */
static NyLPC_TBool cSocketTbl_isClosedTcpPort(
    NyLPC_TcPtrTbl_t* i_inst,
    NyLPC_TUInt16 i_lport)
{
    NyLPC_TcBaseSocket_t** p=(NyLPC_TcBaseSocket_t**)(i_inst->buf);
    NyLPC_TcTcpSocket_t* tp;
    int i;
    //一致するポートを検索
    for(i=i_inst->size-1;i>=0;i--){
        if(p[i]==NULL){
            continue;
        }
        if(p[i]->_typeid!=NyLPC_TcBaseSocket_TYPEID_TCP_SOCK){
            tp=((NyLPC_TcTcpSocket_t*)p[i]);
            //TCPソケット && !クローズ  && ポート一致なら使用中
            if((tp->tcpstateflags!=UIP_CLOSED) && tp->uip_connr.lport==i_lport){
                return NyLPC_TBool_FALSE;
            }
        }
        if(p[i]->_typeid!=NyLPC_TcBaseSocket_TYPEID_TCP_LISTENER){
            //Listenerソケット  && ポート一致なら使用中
            if(((NyLPC_TcTcpListener_t*)p[i])->_port==i_lport){
                return NyLPC_TBool_FALSE;
            }
        }
    }
    //未使用
    return NyLPC_TBool_TRUE;
}
/**
 * テーブルにある有効なソケットのperiodicをすべて呼び出します。
 */
static void cSocketTbl_callPeriodic(
    NyLPC_TcPtrTbl_t* i_inst)
{
    NyLPC_TcBaseSocket_t** p=(NyLPC_TcBaseSocket_t**)(i_inst->buf);
    int i;
    for(i=i_inst->size-1;i>=0;i--){
        if(p[i]==NULL){
            continue;
        }
        switch(p[i]->_typeid){
        case NyLPC_TcBaseSocket_TYPEID_TCP_SOCK:
            //downcast!
            NyLPC_cTcpSocket_periodic((NyLPC_TcTcpSocket_t*)(p[i]));
            break;
        case NyLPC_TcBaseSocket_TYPEID_UDP_SOCK:
            NyLPC_cUdpSocket_periodic((NyLPC_TcUdpSocket_t*)(p[i]));
            break;
        default:
            continue;
        }
    }
}

/**
 * テーブルにある有効なソケットのstartを全て呼び出します。
 */
static void cSocketTbl_callSocketStart(
    NyLPC_TcPtrTbl_t* i_inst,
    const NyLPC_TcIPv4Config_t* i_cfg)
{
    NyLPC_TcBaseSocket_t** p=(NyLPC_TcBaseSocket_t**)(i_inst->buf);
    int i;
    for(i=i_inst->size-1;i>=0;i--){
        if(p[i]==NULL){
            continue;
        }
        switch(p[i]->_typeid){
        case NyLPC_TcBaseSocket_TYPEID_UDP_SOCK:
            NyLPC_cUdpSocket_startService((NyLPC_TcUdpSocket_t*)(p[i]),i_cfg);
            break;
        case NyLPC_TcBaseSocket_TYPEID_TCP_SOCK:
            NyLPC_cTcpSocket_startService((NyLPC_TcTcpSocket_t*)(p[i]),i_cfg);
            break;
        default:
            continue;
        }
    }
}
/**
 * テーブルにある有効なソケットのstartを全て呼び出します。
 */
static void cSocketTbl_callSocketStop(
    NyLPC_TcPtrTbl_t* i_inst)
{
    NyLPC_TcBaseSocket_t** p=(NyLPC_TcBaseSocket_t**)(i_inst->buf);
    int i;
    for(i=i_inst->size-1;i>=0;i--){
        if(p[i]==NULL){
            continue;
        }
        switch(p[i]->_typeid){
        case NyLPC_TcBaseSocket_TYPEID_UDP_SOCK:
            NyLPC_cUdpSocket_stopService((NyLPC_TcUdpSocket_t*)(p[i]));
            break;
        case NyLPC_TcBaseSocket_TYPEID_TCP_SOCK:
            NyLPC_cTcpSocket_stopService((NyLPC_TcTcpSocket_t*)(p[i]));
            break;
        default:
            continue;
        }
    }
}

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
    NyLPC_ArgAssert(NyLPC_cUipService_SYS_TX_BUF_SIZE>40);
    //内部テーブルの初期化
    cSocketTbl_initialize(&(i_inst->_socket_tbl),(void**)(i_inst->_socket_array_buf));
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
    cSocketTbl_finalize(&(i_inst->_socket_tbl));
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
    cSocketTbl_callSocketStart(&(i_inst->_socket_tbl),i_ref_configlation);
    return;
}

/**
 * See header file.
 */
void NyLPC_cIPv4_stop(
    NyLPC_TcIPv4_t* i_inst)
{
    cSocketTbl_callSocketStop(&(i_inst->_socket_tbl));
    i_inst->_ref_config=NULL;
    return;
}

/**
 * See header file.
 */
NyLPC_TBool NyLPC_cIPv4_addSocket(
    NyLPC_TcIPv4_t* i_inst,
    NyLPC_TcBaseSocket_t* i_sock)
{
    //当面、stop中しか成功しない。
    NyLPC_Assert(!NyLPC_cUipService_isRun());
    return NyLPC_cPtrTbl_add(&(i_inst->_socket_tbl),i_sock)>=0;
}

/**
 * See header file.
 */
NyLPC_TBool NyLPC_cIPv4_removeSocket(
    NyLPC_TcIPv4_t* i_inst,
    NyLPC_TcBaseSocket_t* i_sock)
{
    NyLPC_TInt16 i;
    NyLPC_Assert(!NyLPC_cUipService_isRun());
    i=NyLPC_cPtrTbl_getIndex(&(i_inst->_socket_tbl),i_sock);
    if(i>=0){
        NyLPC_cPtrTbl_remove(&(i_inst->_socket_tbl),i);
        return NyLPC_TBool_TRUE;
    }
    return NyLPC_TBool_FALSE;
}


#define IS_START(i_inst) ((i_inst)->_ref_config!=NULL)

/**
 * 稼動時に、1s置きに呼び出す関数です。
 */
void NyLPC_cIPv4_periodec(NyLPC_TcIPv4_t* i_inst)
{
    cSocketTbl_callPeriodic(&(i_inst->_socket_tbl));
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
    NyLPC_TcUipService_t* inst=_NyLPC_TcUipService_inst;
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
        if(cSocketTbl_isClosedTcpPort(&i_inst->_socket_tbl,n))
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
    NyLPC_TcTcpSocket_t* sock;
    NyLPC_TcTcpListener_t* listener;

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
    sock=cSocketTbl_getMatchTcpSocket(&(i_inst->_socket_tbl),i_ipp->payload.tcp->destport,i_ipp->header->srcipaddr,i_ipp->payload.tcp->srcport);
    if(sock!=NULL)
    {
        //既存の接続を処理
        return NyLPC_cTcpSocket_parseRx(sock,i_ipp);
    }

    //未知の接続
    if(!NyLPC_cPtrTbl_hasEmpty(&(i_inst->_socket_tbl))){
        //ソケットテーブルが不十分。RST送信
        return NyLPC_cTcpSocket_allocTcpReverseRstAck(i_ipp);
    }
    //このポートに対応したListenerを得る。
    listener=cSocketTbl_getListenerByPeerPort(&(i_inst->_socket_tbl),i_ipp->payload.tcp->destport);
    if(listener==NULL){
        //Listen対象ではない。RST送信
        return NyLPC_cTcpSocket_allocTcpReverseRstAck(i_ipp);
    }
    //リスナにソケットのバインドを依頼する。
    NyLPC_cTcpListener_synPacket(listener,i_ipp);
    return NULL;//LISTEN成功。送信データなし
DROP:
    return NULL;
}



static NyLPC_TBool udp_rx(
    NyLPC_TcIPv4_t* i_inst,
    NyLPC_TcIPv4Payload_t* i_ipp)
{
    NyLPC_TcUdpSocket_t* sock=NULL;
    if(!NyLPC_TIPv4Addr_isEqual(&(i_ipp->header->destipaddr),&(i_inst->_ref_config->ip_addr)))
    {
        sock=cSocketTbl_getMatchUdpSocket(&(i_inst->_socket_tbl),i_ipp->payload.udp->destport);
    }else{
        if(NyLPC_TIPv4Addr_isEqualWithMask(&(i_ipp->header->destipaddr),&NyLPC_TIPv4Addr_MULTICAST,&NyLPC_TIPv4Addr_MULTICAST_MASK)){
        //MultiCast?
            //マルチキャストに参加している&&portの一致するソケットを検索
            sock=cSocketTbl_getMatchMulticastUdpSocket(&(i_inst->_socket_tbl),&(i_ipp->header->destipaddr),i_ipp->payload.udp->destport);
        }else if(!NyLPC_TIPv4Addr_isEqual(&(i_ipp->header->destipaddr),&NyLPC_TIPv4Addr_BROADCAST)){
        //Broadcast?
            sock=cSocketTbl_getMatchUdpSocket(&(i_inst->_socket_tbl),i_ipp->payload.udp->destport);
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
    return NyLPC_cUdpSocket_parseRx(sock,i_ipp);
DROP:
    return NyLPC_TBool_FALSE;
}


