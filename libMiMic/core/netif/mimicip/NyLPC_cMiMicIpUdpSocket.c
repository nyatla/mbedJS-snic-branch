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
#include "NyLPC_cMiMicIpUdpSocket_protected.h"
#include "NyLPC_cIPv4Payload_protected.h"
#include "NyLPC_cMiMicIpNetIf_protected.h"

/**
 * フラグ値
 */
#define NyLPC_cMiMicIpUdpSocket_FLAG_BROADCAST 0
/**
 * UDP/IPヘッダのサイズ
 */
#define SIZE_OF_IPv4_UDPIP_HEADER 28

#define lockResource(i_inst) NyLPC_cMutex_lock(((i_inst)->_smutex))
#define unlockResource(i_inst) NyLPC_cMutex_unlock(((i_inst)->_smutex))

/*
 *	 関数テーブル
 */
static void joinMulticast(NyLPC_TiUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr);
static void setBroadcast(NyLPC_TiUdpSocket_t* i_inst);
static NyLPC_TInt32 precv(NyLPC_TiUdpSocket_t* i_inst,const void** o_buf_ptr,const struct NyLPC_TIPv4RxInfo** o_info,NyLPC_TUInt32 i_wait_msec);
static void pseek(NyLPC_TiUdpSocket_t* i_inst);
static void* allocSendBuf(NyLPC_TiUdpSocket_t* i_inst,NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_buf_size,NyLPC_TUInt32 i_wait_in_msec);
static void releaseSendBuf(NyLPC_TiUdpSocket_t* i_inst,void* i_buf_ptr);
static NyLPC_TBool psend(NyLPC_TiUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_port,void* i_buf_ptr,int i_len);
static NyLPC_TInt32 send(NyLPC_TiUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_port,const void* i_buf_ptr,NyLPC_TInt32 i_len,NyLPC_TUInt32 i_wait_in_msec);
static void setOnRxHandler(NyLPC_TiUdpSocket_t* i_inst,NyLPC_TiUdpSocket_onRxHandler i_handler);
static void setOnPeriodicHandler(NyLPC_TiUdpSocket_t* i_inst,NyLPC_TiUdpSocket_onPeriodicHandler i_handler);
static struct NyLPC_TIPv4Addr* getSockIP(const NyLPC_TiUdpSocket_t* i_inst);
static void finalize(NyLPC_TiUdpSocket_t* i_inst);

const struct NyLPC_TiUdpSocket_Interface interface=
{
	joinMulticast,
	setBroadcast,
	precv,
	pseek,
	allocSendBuf,
	releaseSendBuf,
	psend,
	send,
	setOnRxHandler,
	setOnPeriodicHandler,
	getSockIP,
	finalize
};




/*
 *	Initializer/Finalizer
 */


NyLPC_TBool NyLPC_cMiMicIpUdpSocket_initialize(NyLPC_TcMiMicIpUdpSocket_t* i_inst,NyLPC_TUInt16 i_port,void* i_rbuf,NyLPC_TUInt16 i_rbuf_len)
{
	NyLPC_TcMiMicIpNetIf_t* srv=_NyLPC_TcMiMicIpNetIf_inst;
	i_inst->_super._super.udp_sock._interface=&interface;
	i_inst->_super._super.udp_sock._tag=NULL;
    NyLPC_cMiMicIpBaseSocket_initialize(&(i_inst->_super),NyLPC_TcMiMicIpBaseSocket_TYPEID_UDP_SOCK);
    //uipサービスは初期化済であること。
    NyLPC_Assert(NyLPC_cMiMicIpNetIf_isInitService());
    i_inst->_smutex=NyLPC_cIPv4_getSockMutex(&(srv->_tcpv4));
    i_inst->uip_udp_conn.lport=NyLPC_htons(i_port);
    i_inst->uip_udp_conn.mcastaddr=NyLPC_TIPv4Addr_ZERO;
    i_inst->uip_udp_conn.flags=0x00;
    i_inst->as_handler.rx=NULL;
    i_inst->as_handler.periodic=NULL;

    NyLPC_cFifoBuffer_initialize(&(i_inst->rxbuf),i_rbuf,i_rbuf_len);
    //管理リストへ登録。
    return NyLPC_cIPv4_addSocket(&(srv->_tcpv4),&(i_inst->_super));
}



/**
 * IP+UDPヘッダサイズを0x05*4+8バイトとして、UDPの送信バッファをセットします。
 */
static void setUdpTxBufHeader(const NyLPC_TcMiMicIpUdpSocket_t* i_inst,void*i_buf,const struct NyLPC_TIPv4Addr* i_dest_ip,NyLPC_TUInt16 i_dest_port,NyLPC_TUInt8 i_iph_word,NyLPC_TUInt16 i_payload_size)
{
    struct NyLPC_TIPv4Header* header=(struct NyLPC_TIPv4Header*)i_buf;
    struct NyLPC_TUdpHeader* udp    =(struct NyLPC_TUdpHeader*)(((NyLPC_TUInt8*)i_buf)+i_iph_word*4);

    header->vhl=0x40|(0x0f&i_iph_word);
    header->len16=NyLPC_htons(i_payload_size+(i_iph_word*4+8));
    udp->udplen=NyLPC_htons(i_payload_size+(8));
    //IPv4のTxヘッダを書き込む。
    header->destipaddr=*i_dest_ip;
    header->srcipaddr =i_inst->uip_udp_conn.lipaddr;

    NyLPC_TIPv4Header_writeTxIpHeader(header,UIP_PROTO_UDP);

    //UDPのTxヘッダを書き込む
    //sorce & destination port
    udp->srcport  = i_inst->uip_udp_conn.lport;
    udp->destport = NyLPC_htons(i_dest_port);
    udp->udpchksum= 0;

    udp->udpchksum=~(NyLPC_TIPv4Header_makeTcpChecksum(header));
    header->ipchksum = ~(NyLPC_TIPv4Header_makeIpChecksum(header));
}




/**
 * この関数は、rxパケットを処理して、ソケットの状態を更新します。
 * uipサービスタスクが実行する関数です。
 * この関数はNyLPC_cTcpSocket_periodicと排他実行すること。
 */
NyLPC_TBool NyLPC_cMiMicIpUdpSocket_parseRx(
	NyLPC_TcMiMicIpUdpSocket_t* i_inst,
    const NyLPC_TcIPv4Payload_t* i_ipp)
{
    NyLPC_TUInt16 tmp16;
    struct NyLPC_TIPv4RxInfo dheader;
    const void* data_offset;
    //ブロードキャストの場合、フラグを確認
    if(NyLPC_TIPv4Addr_isEqual(&(i_ipp->header->destipaddr),&NyLPC_TIPv4Addr_BROADCAST)){
        if(!NyLPC_TUInt8_isBitOn(i_inst->uip_udp_conn.flags,NyLPC_cMiMicIpUdpSocket_FLAG_BROADCAST)){
            goto DROP;
        }
    }
    //パラメータの計算
    tmp16=NyLPC_TUdpHeader_getHeaderLength(i_ipp->payload.tcp);
    //UDPペイロードの長さは、IPパケットの長さ-(IPヘッダ+UDPヘッダ)
    dheader.size=NyLPC_TIPv4Header_getPacketLength(i_ipp->header)-NyLPC_TIPv4Header_getHeaderLength(i_ipp->header)-tmp16;
    dheader.peer_ip=i_ipp->header->srcipaddr;
    dheader.peer_port=NyLPC_ntohs(i_ipp->payload.udp->srcport);
    dheader.ip=i_ipp->header->destipaddr;
    dheader.port=NyLPC_ntohs(i_ipp->payload.udp->destport);
    if(i_inst->as_handler.rx!=NULL){
        if(!i_inst->as_handler.rx((NyLPC_TiUdpSocket_t*)(i_inst),i_ipp->payload.rawbuf+tmp16,&dheader)){
            return NyLPC_TBool_FALSE;//UDPはReturnパケットなし
        }
    }
    //TCPデータオフセット
    data_offset=i_ipp->payload.rawbuf+tmp16;

    //インスタンスをロックする。
    lockResource(i_inst);
    //受信キューへ追加(データ構造はsize[2]+data[n]).sizeに16ビットの受信サイズ,後続にデータ

    //受信データサイズを確認
    if(NyLPC_cFifoBuffer_getSpace(&(i_inst->rxbuf))<dheader.size+sizeof(struct NyLPC_TIPv4RxInfo)){
        goto DROP;
    }
    //バッファに格納可能なら、格納。
    NyLPC_cFifoBuffer_push(&(i_inst->rxbuf),&dheader,sizeof(struct NyLPC_TIPv4RxInfo));
    NyLPC_cFifoBuffer_push(&(i_inst->rxbuf),data_offset,dheader.size);
    unlockResource(i_inst);
    return NyLPC_TBool_FALSE;//UDPはReturnパケットなし
DROP:
    unlockResource(i_inst);
    return NyLPC_TBool_FALSE;
}




static void finalize(NyLPC_TiUdpSocket_t* i_inst)
{
	NyLPC_TcMiMicIpNetIf_t* srv=_NyLPC_TcMiMicIpNetIf_inst;
    NyLPC_Assert(NyLPC_cMiMicIpNetIf_isInitService());
    //uipサービスは初期化済であること。
    if(!NyLPC_cIPv4_removeSocket(&(srv->_tcpv4),&(((NyLPC_TcMiMicIpUdpSocket_t*)i_inst)->_super))){
        //削除失敗、それは死を意味する。
        NyLPC_Abort();
    }
    NyLPC_cFifoBuffer_finalize(&(i_inst->rxbuf));
    NyLPC_cMiMicIpBaseSocket_finalize(&(((NyLPC_TcMiMicIpUdpSocket_t*)i_inst)->_super));
    NyLPC_cMiMicIpNetIf_releaseUdpSocketMemory((NyLPC_TcMiMicIpUdpSocket_t*)i_inst);
    return;
}


static void joinMulticast(NyLPC_TiUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr)
{
	NyLPC_TcMiMicIpUdpSocket_t* inst=(NyLPC_TcMiMicIpUdpSocket_t*)i_inst;
	inst->uip_udp_conn.mcastaddr=*i_addr;
}
static void setBroadcast(NyLPC_TiUdpSocket_t* i_inst)
{
	NyLPC_TcMiMicIpUdpSocket_t* inst=(NyLPC_TcMiMicIpUdpSocket_t*)i_inst;
    NyLPC_TUInt8_setBit(inst->uip_udp_conn.flags,NyLPC_cMiMicIpUdpSocket_FLAG_BROADCAST);
}



/**
 * see Header file
 */
static NyLPC_TInt32 precv(NyLPC_TiUdpSocket_t* i_inst,const void** o_buf_ptr,const struct NyLPC_TIPv4RxInfo** o_info,NyLPC_TUInt32 i_wait_msec)
{
	NyLPC_TcMiMicIpUdpSocket_t* inst=(NyLPC_TcMiMicIpUdpSocket_t*)i_inst;
    NyLPC_TUInt16 rlen;
    //タイマを生成
    NyLPC_TcStopwatch_t sw;
    NyLPC_cStopwatch_initialize(&sw);
    const char* b;
    const struct NyLPC_TIPv4RxInfo* rh;

    //ESTABLISHED以外の場合は、エラー。
    NyLPC_cStopwatch_setNow(&sw);
    while(NyLPC_cStopwatch_elapseInMsec(&sw)<i_wait_msec)
    {
        //MUTEX LOCK
        lockResource(inst);
        rlen=NyLPC_cFifoBuffer_getLength(&(inst->rxbuf));
        //MUTEX UNLOCK
        unlockResource(inst);
        if(rlen>0){
            //受信キューにデータがあれば返す。
            b=(char*)NyLPC_cFifoBuffer_getPtr(&(inst->rxbuf));
            rh=(const struct NyLPC_TIPv4RxInfo*)b;
            *o_buf_ptr=b+sizeof(struct NyLPC_TIPv4RxInfo);
            if(o_info!=NULL){
                *o_info=rh;
            }
            return rh->size;
        }
        //タスクスイッチ
        NyLPC_cThread_yield();
    };
    NyLPC_cStopwatch_finalize(&sw);
    return 0;
}
/**
 * See header file
 */
static void pseek(NyLPC_TiUdpSocket_t* i_inst)
{
	NyLPC_TcMiMicIpUdpSocket_t* inst=(NyLPC_TcMiMicIpUdpSocket_t*)i_inst;
    NyLPC_TUInt16 s;
    const struct NyLPC_TIPv4RxInfo* rh;
    //シークサイズを決定
    s=NyLPC_cFifoBuffer_getLength(&(inst->rxbuf));
    if(s==0){
        return;
    }
    rh=(const struct NyLPC_TIPv4RxInfo*)NyLPC_cFifoBuffer_getPtr(&(inst->rxbuf));
    NyLPC_cFifoBuffer_pop(&(inst->rxbuf),rh->size+sizeof(struct NyLPC_TIPv4RxInfo));
}

/**
 * See header file.
 */
static void* allocSendBuf(NyLPC_TiUdpSocket_t* i_inst,NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_buf_size,NyLPC_TUInt32 i_wait_in_msec)
{
    NyLPC_TUInt16 s;
    void* buf;
    NyLPC_TcStopwatch_t sw;

    NyLPC_cStopwatch_initialize(&sw);
    NyLPC_cStopwatch_startExpire(&sw,i_wait_in_msec);

    //送信バッファを取得
    //@bug バッファが取れるまで通信がブロックするの。ここはなんとかしないと。
    for(;;){
        buf=NyLPC_cMiMicIpNetIf_allocTxBuf(i_hint+(SIZE_OF_IPv4_UDPIP_HEADER),&s);
        if(buf!=NULL){
            break;
        }
        //タイムアウト確認
        if(NyLPC_cStopwatch_isExpired(&sw)){
            return NULL;
        }
    }
    //バッファサイズ確定。
    *o_buf_size=s;
    NyLPC_cStopwatch_finalize(&sw);
    return (NyLPC_TUInt8*)buf+SIZE_OF_IPv4_UDPIP_HEADER;
}
/**
 * See Header file.
 */
static void releaseSendBuf(NyLPC_TiUdpSocket_t* i_inst,void* i_buf_ptr)
{
    NyLPC_cMiMicIpNetIf_releaseTxBuf((NyLPC_TUInt8*)i_buf_ptr-SIZE_OF_IPv4_UDPIP_HEADER);
}

/**
 * See header file
 */
static NyLPC_TBool psend(NyLPC_TiUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_port,void* i_buf_ptr,int i_len)
{
    void* buf;
	NyLPC_TcMiMicIpUdpSocket_t* inst=(NyLPC_TcMiMicIpUdpSocket_t*)i_inst;
     //ブロードキャストの場合、フラグを確認
    if(NyLPC_TIPv4Addr_isEqual(i_addr,&NyLPC_TIPv4Addr_BROADCAST)){
        if(!NyLPC_TUInt8_isBitOn(inst->uip_udp_conn.flags,NyLPC_cMiMicIpUdpSocket_FLAG_BROADCAST)){
            return NyLPC_TBool_FALSE;
        }
    }

    //先頭ポインタは、i_buf-sizeof(SIZE_OF_IPv4_TCPIP_HEADER)固定
    buf=(NyLPC_TUInt8*)i_buf_ptr-SIZE_OF_IPv4_UDPIP_HEADER;

    lockResource(inst);
    //IPv4ペイロードの書き込み
    setUdpTxBufHeader(inst,buf,i_addr,i_port,0x05,i_len);
    unlockResource(inst);
    // !(BroadCast || Multicast)の場合は送信前にARPテーブルをチェックする。
    if(!(NyLPC_TIPv4Addr_isEqual(i_addr,&NyLPC_TIPv4Addr_BROADCAST) || NyLPC_TIPv4Addr_isEqualWithMask(i_addr,&NyLPC_TIPv4Addr_MULTICAST,&NyLPC_TIPv4Addr_MULTICAST_MASK))){
        if(!NyLPC_cMiMicIpNetIf_hasArpInfo(i_addr)){
            NyLPC_cMiMicIpNetIf_sendArpRequest(i_addr);
            NyLPC_cThread_sleep(30);
        }
    }
    NyLPC_cMiMicIpNetIf_sendIPv4Tx(buf);
    NyLPC_cMiMicIpNetIf_releaseTxBuf(buf);
    return NyLPC_TBool_TRUE;
}

/**
 * See header file.
 */
static NyLPC_TInt32 send(NyLPC_TiUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_port,const void* i_buf_ptr,NyLPC_TInt32 i_len,NyLPC_TUInt32 i_wait_in_msec)
{
    NyLPC_TUInt16 s;
    int i;
    void* buf;
    if(i_len<1 || i_len>1200){
        return 0;
    }
    //バッファの取得確率を上げるために2倍のサイズを要求
    for(i=0;i<3;i++){
        buf=allocSendBuf(i_inst,i_len*2,&s,i_wait_in_msec);
        if(buf==NULL || s<i_len){
            continue;
        }
        break;
    }
    if(buf==NULL){
        return -1;
    }
    //送信サイズの計算
    memcpy(buf,i_buf_ptr,i_len);
    if(!psend(i_inst,i_addr,i_port,buf,i_len)){
        releaseSendBuf(i_inst,buf);
        return -1;
    }
    return i_len;
}

static void setOnRxHandler(NyLPC_TiUdpSocket_t* i_inst,NyLPC_TiUdpSocket_onRxHandler i_handler)
{
	NyLPC_TcMiMicIpUdpSocket_t* inst=(NyLPC_TcMiMicIpUdpSocket_t*)i_inst;
	inst->as_handler.rx=i_handler;
}
static void setOnPeriodicHandler(NyLPC_TiUdpSocket_t* i_inst,NyLPC_TiUdpSocket_onPeriodicHandler i_handler)
{
	NyLPC_TcMiMicIpUdpSocket_t* inst=(NyLPC_TcMiMicIpUdpSocket_t*)i_inst;
	inst->as_handler.periodic=i_handler;
}
static struct NyLPC_TIPv4Addr* getSockIP(const NyLPC_TiUdpSocket_t* i_inst)
{
	NyLPC_TcMiMicIpUdpSocket_t* inst=(NyLPC_TcMiMicIpUdpSocket_t*)i_inst;
	return &inst->uip_udp_conn.lipaddr;
}


void NyLPC_cMiMicIpUdpSocket_startService(NyLPC_TcMiMicIpUdpSocket_t* i_inst,const NyLPC_TcIPv4Config_t* i_config)
{
    i_inst->uip_udp_conn.lipaddr=i_config->ip_addr;
    //受信バッファのクリア
    NyLPC_cFifoBuffer_clear(&(i_inst->rxbuf));
    return;
}


void NyLPC_cMiMicIpUdpSocket_stopService(NyLPC_TcMiMicIpUdpSocket_t* i_inst)
{
    //停止処理？
}


