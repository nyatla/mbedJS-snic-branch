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
//#include "NyLPC_cIPv4IComp_protected.h"
//#include "NyLPC_cTcpListener_protected.h"
#include "NyLPC_cMiMicIpNetIf_protected.h"
#include "NyLPC_stdlib.h"
#include "../NyLPC_NetIf_ip_types.h"
#include "NyLPC_cIPv4IComp_protected.h"
#include "NyLPC_cMiMicIpTcpSocket_protected.h"
#include "NyLPC_cMiMicIpUdpSocket_protected.h"
#include "NyLPC_cMiMicIpTcpListener_protected.h"





/****************************************************
 * UipServiceに関する宣言:その他
 ***************************************************/
/**
 * イーサネットフレームの読み出し構造体
 */
struct TEthPacket
{
    struct NyLPC_TEthernetIIHeader header;
    union{
        struct NyLPC_TArpHeader arp;
        struct NyLPC_TIPv4Header ipv4;
    }data;
}PACK_STRUCT_END;



/**
 * サービスインスタンスのポインタ。サービスが稼働中はインスタンスのポインタが有効です。
 */
NyLPC_TcMiMicIpNetIf_t* _NyLPC_TcMiMicIpNetIf_inst=NULL;



/**
 * uipタスク
 */
static int uipTask(void *pvParameters);

/** イーサネットドライバからのハンドラ*/
static void ethernet_handler(void* i_param,NyLPC_TiEthernetDevice_EVENT i_type);

//--------------------------------------------------------------


static NyLPC_TBool sendIPv4Tx(struct TEthPacket* i_eth_buf);

//static void sendArpReqest(const struct TEthPacket* i_eth_packet);
static void sendRawEthFrameNL(void* i_buf,NyLPC_TUInt16 i_len);
static void releaseTxBufNL(void* i_buf);

/**メッセージなし*/
#define TTaskMessage_MSG_NULL    0x0000
/**uipコアタスクに、開始要求する*/
#define TTaskMessage_MSG_RUN     0x0001
/**uipコアタスクに、停止要求する*/
#define TTaskMessage_MSG_STOP    0x0002


static NyLPC_TcThread_t th;

NyLPC_TBool NyLPC_cMiMicIpNetIf_initialize(NyLPC_TcMiMicIpNetIf_t* i_inst)
{
    //サービスは停止している事。 - Service must be uninitialized.
    NyLPC_Assert(!NyLPC_cMiMicIpNetIf_isInitService());
    //IP処理部分の初期化
    NyLPC_cIPv4_initialize(&(i_inst->_tcpv4));
    //EMAC割込セマフォ
    NyLPC_cSemaphore_initialize(&i_inst->_emac_semapho);

    i_inst->_status=0x00;
    NyLPC_cStopwatch_initialize(&(i_inst->_arp_sw));
    NyLPC_cStopwatch_initialize(&(i_inst->_periodic_sw));
    NyLPC_AbortIfNot(NyLPC_cMutex_initialize(&(i_inst->_mutex)));

    _NyLPC_TcMiMicIpNetIf_inst=i_inst;
    //タスク起動
    NyLPC_cThread_initialize(&th,NyLPC_cMiMicIpNetIf_config_STACK_SIZE,NyLPC_TcThread_PRIORITY_SERVICE);
    NyLPC_cThread_start(&th,uipTask,NULL);
    return NyLPC_TBool_TRUE;
}







/**
 * UIP処理を開始します。
 * この関数はリエントラントではありません。複数のタスクから共有するときには、排他ロックを掛けてください。
 * @param i_ref_config
 * このコンフィギュレーションは、stopを実行するまでの間、インスタンスから参照します。外部で保持してください。
 */
void NyLPC_cMiMicIpNetIf_start(const NyLPC_TcIPv4Config_t* i_ref_config)
{
	NyLPC_TcMiMicIpNetIf_t* inst=_NyLPC_TcMiMicIpNetIf_inst;
    NyLPC_Assert(NyLPC_cMiMicIpNetIf_isInitService());
    if(!NyLPC_cMiMicIpNetIf_isRun())
    {
        //はじめて起動するときに1度だけデバイス取得(タスクスイッチが動いてないと動かないからここで。)
        if(inst->_ethif==NULL){
            inst->_ethif=getEthernetDevicePnP();
        }
        //コンフィグレーションセット
        inst->_netinfo.current_config=i_ref_config;
        //開始要求セット
        NyLPC_TUInt16_setBit(inst->_status,NyLPC_TcMiMicIpNetIf_ORDER_START);
        //Order実行待ち
        while(NyLPC_TUInt16_isBitOn(inst->_status,NyLPC_TcMiMicIpNetIf_ORDER_START)){
            NyLPC_cThread_sleep(10);
        }
        //デバイス情報の追記
        inst->_netinfo.device_name=NyLPC_iEthernetDevice_getDevicName(inst->_ethif);
    }
    return;
}
/**
 * UIP処理を停止します。
 * この関数はリエントラントではありません。複数のタスクから共有するときには、排他ロックを掛けてください。
 * いまのところ、ストップシーケンスの実装は良くありません。
 * 再設計が必要。
 */
void NyLPC_cMiMicIpNetIf_stop(void)
{
	NyLPC_TcMiMicIpNetIf_t* inst=_NyLPC_TcMiMicIpNetIf_inst;
    NyLPC_Assert(NyLPC_cMiMicIpNetIf_isInitService());
    if(NyLPC_cMiMicIpNetIf_isRun())
    {
        NyLPC_TUInt16_setBit(inst->_status,NyLPC_TcMiMicIpNetIf_ORDER_STOP);
        //Order実行待ち
        while(NyLPC_TUInt16_isBitOn(inst->_status,NyLPC_TcMiMicIpNetIf_ORDER_STOP)){
            NyLPC_cThread_sleep(10);
        }
    }
    return;
}

static const struct NyLPC_TNetInterfaceInfo* NyLPC_cMiMicIpNetIf_getInterfaceInfo(void)
{
	NyLPC_TcMiMicIpNetIf_t* inst=_NyLPC_TcMiMicIpNetIf_inst;
    return &inst->_netinfo;
}

/**********************************************************************
 *
 * </HWコールバックに関わる宣言>
 *
 *********************************************************************/


//PERIODIC rate
#define PERIODIC_TIMER (1*200)
#define ARP_TIMER (60*1000*10)



/**
 * 操作キューを確認して、タスクのステータスをアップデートします。
 * 高速化のため、Proc-Callerを使用していません。複雑なタスク操作をするときには、書き換えてください。
 * @return
 * UIPタスクの実行状態
 */
static NyLPC_TBool updateTaskStatus()
{
	NyLPC_TcMiMicIpNetIf_t* inst=_NyLPC_TcMiMicIpNetIf_inst;
    if(NyLPC_cMiMicIpNetIf_isRun()){
        //開始状態
        if(NyLPC_TUInt16_isBitOn(inst->_status,NyLPC_TcMiMicIpNetIf_ORDER_STOP))
        {
            //停止操作
            NyLPC_iEthernetDevice_stop(inst->_ethif);
            NyLPC_cIPv4_stop(&(inst->_tcpv4));
            NyLPC_cIPv4IComp_finalize(&(inst->_icomp));
            NyLPC_cIPv4Arp_finalize(&(inst->_arp));
//            inst->_ref_config=NULL;
            NyLPC_TUInt16_unsetBit(inst->_status,NyLPC_TcMiMicIpNetIf_STATUSBIT_IS_RUNNING);
            NyLPC_TUInt16_unsetBit(inst->_status,NyLPC_TcMiMicIpNetIf_ORDER_STOP);
            return NyLPC_TBool_FALSE;
        }
        return NyLPC_TBool_TRUE;
    }else{
        //停止状態
        if(NyLPC_TUInt16_isBitOn(inst->_status,NyLPC_TcMiMicIpNetIf_ORDER_START))
        {
            //TCP,ICOMPの初期化
            NyLPC_cIPv4_start(&(inst->_tcpv4),inst->_netinfo.current_config);
            NyLPC_cIPv4IComp_initialize(&(inst->_icomp),inst->_netinfo.current_config);
            //uip_arp_init(msg->start.ref_config);
            NyLPC_cIPv4Arp_initialize(&(inst->_arp),inst->_netinfo.current_config);
            NyLPC_cStopwatch_startExpire(&(inst->_arp_sw),1);//1度ARPを起動するため。
            NyLPC_cStopwatch_startExpire(&(inst->_periodic_sw),PERIODIC_TIMER);
            //EtherNETデバイス初期化
            while(!NyLPC_iEthernetDevice_start(inst->_ethif,&(inst->_netinfo.current_config->eth_mac),ethernet_handler,inst));
            NyLPC_TUInt16_setBit(inst->_status,NyLPC_TcMiMicIpNetIf_STATUSBIT_IS_RUNNING);
            NyLPC_TUInt16_unsetBit(inst->_status,NyLPC_TcMiMicIpNetIf_ORDER_START);
            return NyLPC_TBool_TRUE;
        }
        return NyLPC_TBool_FALSE;
    }
}

/**
 * uipタスクのメインループ
 */
static int uipTask(void *pvParameters)
{
    NyLPC_TUInt16 rx_len,tx_len;
    struct TEthPacket* ethbuf;
    NyLPC_TcMiMicIpNetIf_t* inst=_NyLPC_TcMiMicIpNetIf_inst;
    void* r;
    (void)pvParameters;
    for( ;; )
    {
        //タスク状態の更新
        if(!updateTaskStatus())
        {
            //RUNステータス以外の時は、ここで終了する。
            NyLPC_cThread_sleep(50);
            continue;
        }
        //イーサネットフレームの取得
        //Ethernet Device Lock(ARPを含む)
        NyLPC_cMutex_lock(&(inst->_mutex));
        ethbuf= (struct TEthPacket*)NyLPC_iEthernetDevice_getRxEthFrame(inst->_ethif,&rx_len);
        tx_len=0;
        while(ethbuf != NULL){
            if(rx_len>0)
            {
                //ペイロードサイズを計算
                rx_len-=sizeof(struct NyLPC_TEthernetIIHeader);
                switch(ethbuf->header.type)
                {
                case NyLPC_HTONS(NyLPC_TEthernetIIHeader_TYPE_IP):
                    //ARPテーブルの更新
                    //uip_arp_ipin(&(ethbuf->header),ethbuf->data.ipv4.srcipaddr);
                    NyLPC_cIPv4Arp_incomingIp(&inst->_arp,&(ethbuf->header),ethbuf->data.ipv4.srcipaddr);
                    //Ethernet Device UnLock(NyLPC_cIPv4_rxがallocをコールする可能性があるので一時的にロック解除)
                    NyLPC_cMutex_unlock(&(inst->_mutex));
                    //IPパケットの処理
                    r=NyLPC_cIPv4_rx(&(inst->_tcpv4),&(ethbuf->data.ipv4),rx_len);
                    if(r!=NULL){
                        //IPパケットとして送信
                        NyLPC_cMiMicIpNetIf_sendIPv4Tx(r);
                    }
                    //ロックの復帰
                    NyLPC_cMutex_lock(&(inst->_mutex));
                    if(r!=NULL){
                        releaseTxBufNL(r);
                    }
                    break;
                case NyLPC_HTONS(NyLPC_TEthernetIIHeader_TYPE_ARP):
                    //Ethernet Device UnLock(NyLPC_cIPv4_rxがallocをコールする可能性があるので一時的にロック解除)
                    NyLPC_cMutex_unlock(&(inst->_mutex));
                    r=NyLPC_cIPv4Arp_rx(&inst->_arp,&(ethbuf->data.arp),rx_len,&tx_len);
                    NyLPC_cMutex_lock(&(inst->_mutex));
                    if(r!=NULL){
                        sendRawEthFrameNL(r,tx_len);
                        releaseTxBufNL(r);
                    }
                    break;
                case NyLPC_HTONS(NyLPC_TEthernetIIHeader_TYPE_IPV6):
                    //uip_process_ipv6();
                    break;
                default:
                    break;
                }
            }
            //受信キューを進行。
            NyLPC_iEthernetDevice_nextRxEthFrame(inst->_ethif);
            //受信処理
            ethbuf= (struct TEthPacket*)NyLPC_iEthernetDevice_getRxEthFrame(inst->_ethif,&rx_len);
        }
        //データが無い。
        if(NyLPC_cStopwatch_isExpired(&(inst->_arp_sw))){
            //uip_arp_timer();
            NyLPC_cIPv4Arp_periodic(&inst->_arp);
            NyLPC_cStopwatch_startExpire(&(inst->_arp_sw),ARP_TIMER);
        }
        if(NyLPC_cStopwatch_isExpired(&(inst->_periodic_sw))){
            NyLPC_cMutex_unlock(&(inst->_mutex));
            NyLPC_cIPv4_periodec(&(inst->_tcpv4));
            NyLPC_cMutex_lock(&(inst->_mutex));
            NyLPC_cStopwatch_startExpire(&(inst->_periodic_sw),PERIODIC_TIMER);
        }
        //リソースロックの解除
        NyLPC_cMutex_unlock(&(inst->_mutex));
        //割込によるセマフォの解除か、タイムアウトで再開する。(タイムアウト値は周期関数の実行レート以下にすること。)
        NyLPC_cSemaphore_take(&(_NyLPC_TcMiMicIpNetIf_inst->_emac_semapho),PERIODIC_TIMER);
    }
    return 0;
}


/**
 * イーサネットドライバからのハンドラ
 */
static void ethernet_handler(void* i_param,NyLPC_TiEthernetDevice_EVENT i_type)
{
    switch(i_type){
    case NyLPC_TiEthernetDevice_EVENT_ON_RX:
        //受信系のセマフォブロックの解除
        NyLPC_cSemaphore_giveFromISR(&(((NyLPC_TcMiMicIpNetIf_t*)i_param)->_emac_semapho));
        break;
    default:
        break;
    }
}

/**
 * IPv4パケットのpeerIPを問い合わせるARPパケットを送信します。
 * allocを中でコールするから要UNLOCK状態
 */
void NyLPC_cMiMicIpNetIf_sendArpRequest(const struct NyLPC_TIPv4Addr* i_addr)
{
	NyLPC_TcMiMicIpNetIf_t* inst=_NyLPC_TcMiMicIpNetIf_inst;
    void* p;
    NyLPC_TUInt16 tx_len;
    struct TEthPacket* ethbuf;
    //システムTxBufを得る
    ethbuf=(struct TEthPacket*)NyLPC_cMiMicIpNetIf_allocSysTxBuf();
    //ARPパケットを作る。
    NyLPC_TArpHeader_setArpRequest(&(ethbuf->data.arp),inst->_netinfo.current_config->ip_addr,&(inst->_netinfo.current_config->eth_mac),i_addr);
    tx_len=NyLPC_TEthernetIIHeader_setArpTx(&(ethbuf->header),&(inst->_netinfo.current_config->eth_mac));
    //送信
    p=((struct NyLPC_TEthernetIIHeader*)ethbuf)-1;

    NyLPC_cMutex_lock(&(inst->_mutex));
    NyLPC_iEthernetDevice_sendTxEthFrame(inst->_ethif,p,tx_len);
    NyLPC_iEthernetDevice_releaseTxBuf(inst->_ethif,p);
    NyLPC_cMutex_unlock(&(inst->_mutex));
}




/**
 * allocTxBufで取得したペイロードメモリを"IPパケットとして"送信します。
 * @param i_eth_payload
 * [NyLPC_TEthernetIIHeader][payload]メモリの、[payload]のアドレスを指定します。
 * 通常は、NyLPC_cUipService_allocTxBufの返却したメモリを指定します。
 */

void NyLPC_cMiMicIpNetIf_sendIPv4Tx(void* i_eth_payload)
{
	NyLPC_TcMiMicIpNetIf_t* inst=_NyLPC_TcMiMicIpNetIf_inst;
    void* p=((struct NyLPC_TEthernetIIHeader*)i_eth_payload)-1;
    NyLPC_cMutex_lock(&(inst->_mutex));
    //IPパケットの送信を試行
    if(sendIPv4Tx((struct TEthPacket*)p)){
        NyLPC_cMutex_unlock(&(inst->_mutex));
        return;
    }
    NyLPC_cMutex_unlock(&(inst->_mutex));
    //ARPリクエストを代わりに送信
    NyLPC_cMiMicIpNetIf_sendArpRequest(&((struct NyLPC_TIPv4Header*)i_eth_payload)->destipaddr);
    return;
}


/**
 * ARPテーブルに指定したIPがあるかを返します。
 */
NyLPC_TBool NyLPC_cMiMicIpNetIf_hasArpInfo(const struct NyLPC_TIPv4Addr* i_addr)
{
	NyLPC_TcMiMicIpNetIf_t* inst=_NyLPC_TcMiMicIpNetIf_inst;
    return NyLPC_cIPv4Arp_IPv4toEthAddr(&inst->_arp,*i_addr)!=NULL;
}

/**
 * システム用の送信ペイロードを返します。
 * 関数は必ず成功します。
 */
void* NyLPC_cMiMicIpNetIf_allocSysTxBuf(void)
{
    NyLPC_TUInt16 s;
    NyLPC_TcMiMicIpNetIf_t* inst=_NyLPC_TcMiMicIpNetIf_inst;
    struct TEthPacket* ethbuf;
    //排他処理をして、メモリを取得する。SYSTEMメモリはEthernetドライバの解放待ちのみなのでまとめてLOCKしておｋ
    NyLPC_cMutex_lock(&(inst->_mutex));
    for(;;){
        ethbuf=(struct TEthPacket*)NyLPC_iEthernetDevice_allocTxBuf(inst->_ethif,NyLPC_TcEthernetMM_HINT_CTRL_PACKET,&s);
        if(ethbuf==NULL){
            NyLPC_cThread_yield();
            continue;
        }
        break;
    }
    NyLPC_cMutex_unlock(&(inst->_mutex));
    //イーサネットバッファのアドレスを計算
    return &(ethbuf->data);
}



void* NyLPC_cMiMicIpNetIf_allocTxBuf(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size)
{
    NyLPC_TcMiMicIpNetIf_t* inst=_NyLPC_TcMiMicIpNetIf_inst;
    struct TEthPacket* ethbuf;
    //排他処理をして、メモリを取得する。
    NyLPC_cMutex_lock(&(inst->_mutex));
    ethbuf=(struct TEthPacket*)NyLPC_iEthernetDevice_allocTxBuf(inst->_ethif,i_hint+sizeof(struct NyLPC_TEthernetIIHeader),o_size);
    NyLPC_cMutex_unlock(&(inst->_mutex));
    if(ethbuf==NULL){
        return NULL;
    }
    //イーサネットバッファのサイズを計算
    *o_size-=sizeof(struct NyLPC_TEthernetIIHeader);
    //イーサネットバッファのアドレスを計算
    return &(ethbuf->data);
}


void* NyLPC_cMiMicIpNetIf_releaseTxBuf(void* i_buf)
{
    //排他処理をして、メモリを開放する。
    NyLPC_TcMiMicIpNetIf_t* inst=_NyLPC_TcMiMicIpNetIf_inst;
    NyLPC_cMutex_lock(&(inst->_mutex));
    //ペイロードの位置から、メモリブロックを再生。
    NyLPC_iEthernetDevice_releaseTxBuf(inst->_ethif,((struct NyLPC_TEthernetIIHeader*)i_buf)-1);
    NyLPC_cMutex_unlock(&(inst->_mutex));
    return NULL;
}









/**
 * イーサネットフレームを送信します。
 * この関数はiptaskで実行される関数からのみ使用てください。
 * @i_buf
 * イーサネットフレームを格納したメモリです。
 * @i_len
 * イーサネットペイロードのサイズです。
 */
static void sendRawEthFrameNL(void* i_buf,NyLPC_TUInt16 i_len)
{
    NyLPC_iEthernetDevice_sendTxEthFrame(
        _NyLPC_TcMiMicIpNetIf_inst->_ethif,
        ((struct NyLPC_TEthernetIIHeader*)i_buf)-1,
        i_len);
    return;
}
/**
 * ロック状態で使用できるreleaseTxBuf。
 * この関数はiptaskで実行される関数からのみ使用してください。
 */
static void releaseTxBufNL(void* i_buf)
{
    //ペイロードの位置から、メモリブロックを再生。
    NyLPC_iEthernetDevice_releaseTxBuf(
        _NyLPC_TcMiMicIpNetIf_inst->_ethif,
        ((struct NyLPC_TEthernetIIHeader*)i_buf)-1);
    return;
}
/**
 * マルチキャスとアドレスへ変換する。
 */
static void ip2MulticastEmacAddr(const struct NyLPC_TIPv4Addr* i_addr,struct NyLPC_TEthAddr* o_emac)
{
    NyLPC_TUInt32 n=NyLPC_htonl(i_addr->v);
    o_emac->addr[0]=0x01;
    o_emac->addr[1]=0x00;
    o_emac->addr[2]=0x5E;
    o_emac->addr[3]=((n>>16) & 0x7f);
    o_emac->addr[4]=((n>> 8) & 0xff);
    o_emac->addr[5]=(n & 0xff);
    return;
};

/**
 * ペイロードをIPパケットとしてネットワークへ送出します。
 * コール前に、必ずロックしてから呼び出してください。
 * @param i_eth_payload
 * allocTxBufで確保したメモリを指定してください。
 * ペイロードには、TCP/IPパケットを格納します。
 */
static NyLPC_TBool sendIPv4Tx(struct TEthPacket* i_eth_buf)
{
    NyLPC_TcMiMicIpNetIf_t* inst=_NyLPC_TcMiMicIpNetIf_inst;
    struct NyLPC_TEthAddr emac;
    NyLPC_TUInt16 tx_len;
    const struct NyLPC_TEthAddr* eth_dest;
    //ペイロードのアドレスから、イーサネットフレームバッファのアドレスを復元

    if(NyLPC_TIPv4Addr_isEqual(&(i_eth_buf->data.ipv4.destipaddr),&NyLPC_TIPv4Addr_BROADCAST)) {
        //ブロードキャストならそのまま
        eth_dest=&NyLPC_TEthAddr_BROADCAST;
    }else if(NyLPC_TIPv4Addr_isEqualWithMask(&(i_eth_buf->data.ipv4.destipaddr),&NyLPC_TIPv4Addr_MULTICAST,&NyLPC_TIPv4Addr_MULTICAST_MASK)){
        //マルチキャスト
        ip2MulticastEmacAddr(&(i_eth_buf->data.ipv4.destipaddr),&emac);
        eth_dest=&emac;
    }else{
        //LocalIP以外ならdefaultRootへ変換
        eth_dest=NyLPC_cIPv4Arp_IPv4toEthAddr(
            &inst->_arp,
            NyLPC_cIPv4Config_isLocalIP(inst->_netinfo.current_config, &(i_eth_buf->data.ipv4.destipaddr))?(i_eth_buf->data.ipv4.destipaddr):(inst->_netinfo.current_config->dr_addr));
        //IP->MAC変換をテスト。
        if(eth_dest==NULL){
            return NyLPC_TBool_FALSE;
        }
    }
    //変換可能なら、イーサネットヘッダを更新して、送信処理へ。
    tx_len=NyLPC_TEthernetIIHeader_setIPv4Tx(&(i_eth_buf->header),&(inst->_netinfo.current_config->eth_mac),eth_dest);
    NyLPC_iEthernetDevice_sendTxEthFrame(inst->_ethif,i_eth_buf,tx_len);
    return NyLPC_TBool_TRUE;
}

static NyLPC_TBool isInitService(void)
{
	return _NyLPC_TcMiMicIpNetIf_inst!=NULL;
}

//--------------------------------------------------------------------------------
//
//	NetIF Interface
//
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// ソケットテーブル
//--------------------------------------------------------------------------------

#define FLAGS_USED	0x00000001

struct TTcpTable
{
	NyLPC_TUInt32 flags;
	NyLPC_TcMiMicIpTcpSocket_t socket;
	NyLPC_TUInt8 rxbuf[NyLPC_cMiMicIpNetIf_config_TCPSOCKET_RX_BUFFER_SIZE];
};
struct TUdpTable
{
	NyLPC_TUInt32 flags;
	NyLPC_TcMiMicIpUdpSocket_t socket;
	NyLPC_TUInt8 rxbuf[NyLPC_cMiMicIpNetIf_config_UDPSOCKET_RX_BUFFER_SIZE];
};
struct TUdpNBTable
{
	NyLPC_TUInt32 flags;
	NyLPC_TcMiMicIpUdpSocket_t socket;
};
struct TTcpListenerTable
{
	NyLPC_TUInt32 flags;
	NyLPC_TcMiMicIpTcpListener_t listener;
};



static struct TTcpTable tcp_socket_table[NyLPC_cMiMicIpNetIf_config_TCPSOCKET_MAX];
static struct TUdpTable udp_socket_table[NyLPC_cMiMicIpNetIf_config_UDPSOCKET_MAX];
static struct TUdpNBTable udp_socket_nb_table[NyLPC_cMiMicIpNetIf_config_NB_UDPSOCKET_MAX];
static struct TTcpListenerTable tcp_listener_table[NyLPC_cMiMicIpNetIf_config_TCPLISTENER_MAX];

NyLPC_TcMiMicIpTcpListener_t* NyLPC_cMiMicIpNetIf_getListenerByPeerPort(NyLPC_TUInt16 i_port)
{
    int i;
    //一致するポートを検索して、acceptをコールする。
    for(i=NyLPC_cMiMicIpNetIf_config_TCPLISTENER_MAX-1;i>=0;i--){
        if((tcp_listener_table[i].flags&FLAGS_USED)==0){
            continue;
        }
        if(tcp_listener_table[i].listener._port!=i_port){
            continue;
        }
        return &tcp_listener_table[i].listener;
    }
    return NULL;
}

/**
 * 指定番号のTCPポートが未使用かを返す。
 * @return
 * i_lport番のポートが未使用であればTRUE
 */
NyLPC_TBool NyLPC_cMiMicIpNetIf_isClosedTcpPort(NyLPC_TUInt16 i_lport)
{
    int i;
    //未使用のTCPソケット？
    for(i=NyLPC_cMiMicIpNetIf_config_TCPSOCKET_MAX-1;i>=0;i--){
        if(	((tcp_socket_table[i].flags&FLAGS_USED)!=0) &&
			(NyLPC_cMiMicIpTcpSocket_getLocalPort(&tcp_socket_table[i].socket)==i_lport)&&
			(!NyLPC_cMiMicIpTcpSocket_isClosed(&tcp_socket_table[i].socket))){
        	//ポート使用中
        	return NyLPC_TBool_FALSE;
        }
    }
    for(i=NyLPC_cMiMicIpNetIf_config_TCPLISTENER_MAX-1;i>=0;i--){
        if(	((tcp_listener_table[i].flags&FLAGS_USED)!=0) &&
			(NyLPC_cMiMicIpTcpListener_getLocalPort(&tcp_listener_table[i].listener)==i_lport)){
        	//ポート使用中
        	return NyLPC_TBool_FALSE;
        }
    }
    return NyLPC_TBool_TRUE;
}

/**
 * 条件に一致する、アクティブなTCPソケットオブジェクトを取得します。
 * この関数は、ローカルIPが一致していると仮定して検索をします。
 * @param i_rip
 * リモートIPアドレスを指定します。
 */
NyLPC_TcMiMicIpTcpSocket_t* NyLPC_cMiMicIpNetIf_getMatchTcpSocket(
    NyLPC_TUInt16 i_lport,struct NyLPC_TIPv4Addr i_rip,NyLPC_TUInt16 i_rport)
{
    NyLPC_TcMiMicIpTcpSocket_t* tp;
    int i;
    //一致するポートを検索
    for(i=NyLPC_cMiMicIpNetIf_config_TCPSOCKET_MAX-1;i>=0;i--){
    	if((tcp_socket_table[i].flags&FLAGS_USED)==0){
    		continue;
    	}
		if(NyLPC_cMiMicIpTcpSocket_isClosed(&tcp_socket_table[i].socket)){
    		continue;
		}
		tp=&tcp_socket_table[i].socket;
        //パラメータの一致チェック
        if(i_lport!=tp->uip_connr.lport || i_rport!= tp->uip_connr.rport || i_rip.v!=tp->uip_connr.ripaddr.v)
        {
            continue;
        }
        return tp;
    }
    return NULL;
}
NyLPC_TcMiMicIpUdpSocket_t* NyLPC_cMiMicIpNetIf_getMatchUdpSocket(
    NyLPC_TUInt16 i_lport)
{
    int i;
    for(i=NyLPC_cMiMicIpNetIf_config_UDPSOCKET_MAX-1;i>=0;i--){
    	if((udp_socket_table[i].flags&FLAGS_USED)==0){
    		continue;
    	}
        if(i_lport!=udp_socket_table[i].socket.uip_udp_conn.lport){
        	continue;
        }
    	//unicast
        return &udp_socket_table[i].socket;
    }
    for(i=NyLPC_cMiMicIpNetIf_config_NB_UDPSOCKET_MAX-1;i>=0;i--){
    	if((udp_socket_nb_table[i].flags&FLAGS_USED)==0){
    		continue;
    	}
        if(i_lport!=udp_socket_nb_table[i].socket.uip_udp_conn.lport){
    		continue;
        }
    	//unicast
        return &udp_socket_nb_table[i].socket;
    }
    return NULL;
}
NyLPC_TcMiMicIpUdpSocket_t* NyLPC_cMiMicIpNetIf_getMatchMulticastUdpSocket(
    const struct NyLPC_TIPv4Addr* i_mcast_ip,
    NyLPC_TUInt16 i_lport)
{
    int i;
    for(i=NyLPC_cMiMicIpNetIf_config_UDPSOCKET_MAX-1;i>=0;i--){
    	if((udp_socket_table[i].flags&FLAGS_USED)==0){
    		continue;
    	}
        if(i_lport!=udp_socket_table[i].socket.uip_udp_conn.lport || (!NyLPC_TIPv4Addr_isEqual(i_mcast_ip,&(udp_socket_table[i].socket.uip_udp_conn.mcastaddr))))
        {
            continue;
        }
        return &udp_socket_table[i].socket;
    }
    for(i=NyLPC_cMiMicIpNetIf_config_NB_UDPSOCKET_MAX-1;i>=0;i--){
    	if((udp_socket_nb_table[i].flags&FLAGS_USED)==0){
    		continue;
    	}
        if(i_lport!=udp_socket_nb_table[i].socket.uip_udp_conn.lport || (!NyLPC_TIPv4Addr_isEqual(i_mcast_ip,&(udp_socket_nb_table[i].socket.uip_udp_conn.mcastaddr))))
        {
            continue;
        }
        return &udp_socket_nb_table[i].socket;
    }
    return NULL;
}


void NyLPC_cMiMicIpNetIf_callPeriodic(void)
{
    int i;
    for(i=NyLPC_cMiMicIpNetIf_config_UDPSOCKET_MAX-1;i>=0;i--){
    	if((udp_socket_table[i].flags&FLAGS_USED)!=0){
            NyLPC_cMiMicIpUdpSocket_periodic(&udp_socket_table[i].socket);
    	}
    }
    for(i=NyLPC_cMiMicIpNetIf_config_NB_UDPSOCKET_MAX-1;i>=0;i--){
    	if((udp_socket_nb_table[i].flags&FLAGS_USED)!=0){
            NyLPC_cMiMicIpUdpSocket_periodic(&udp_socket_nb_table[i].socket);
    	}
    }
    for(i=NyLPC_cMiMicIpNetIf_config_TCPSOCKET_MAX-1;i>=0;i--){
    	if((tcp_socket_table[i].flags&FLAGS_USED)!=0){
            NyLPC_cMiMicIpTcpSocket_periodic(&tcp_socket_table[i].socket);
    	}
    }
}
void NyLPC_cMiMicIpNetIf_callSocketStart(
	const NyLPC_TcIPv4Config_t* i_cfg)
{
    int i;
    for(i=NyLPC_cMiMicIpNetIf_config_UDPSOCKET_MAX-1;i>=0;i--){
    	if((udp_socket_table[i].flags&FLAGS_USED)!=0){
            NyLPC_cMiMicIpUdpSocket_startService(&udp_socket_table[i].socket,i_cfg);
    	}
    }
    for(i=NyLPC_cMiMicIpNetIf_config_NB_UDPSOCKET_MAX-1;i>=0;i--){
    	if((udp_socket_nb_table[i].flags&FLAGS_USED)!=0){
            NyLPC_cMiMicIpUdpSocket_startService(&udp_socket_nb_table[i].socket,i_cfg);
    	}
    }
    for(i=NyLPC_cMiMicIpNetIf_config_TCPSOCKET_MAX-1;i>=0;i--){
    	if((tcp_socket_table[i].flags&FLAGS_USED)!=0){
            NyLPC_cMiMicIpTcpSocket_startService(&tcp_socket_table[i].socket,i_cfg);
    	}
    }
}
void NyLPC_cMiMicIpNetIf_callSocketStop(void)
{
    int i;
    for(i=NyLPC_cMiMicIpNetIf_config_UDPSOCKET_MAX-1;i>=0;i--){
    	if((udp_socket_table[i].flags&FLAGS_USED)!=0){
            NyLPC_cMiMicIpUdpSocket_stopService(&udp_socket_table[i].socket);
    	}
    }
    for(i=NyLPC_cMiMicIpNetIf_config_NB_UDPSOCKET_MAX-1;i>=0;i--){
    	if((udp_socket_nb_table[i].flags&FLAGS_USED)!=0){
            NyLPC_cMiMicIpUdpSocket_stopService(&udp_socket_nb_table[i].socket);
    	}
    }
    for(i=NyLPC_cMiMicIpNetIf_config_TCPSOCKET_MAX-1;i>=0;i--){
    	if((tcp_socket_table[i].flags&FLAGS_USED)!=0){
            NyLPC_cMiMicIpTcpSocket_stopService(&tcp_socket_table[i].socket);
    	}
    }
}



//--------------------------------------------------------------------------------
// インタフェイス関数

static NyLPC_TiTcpSocket_t* createTcpSocetEx(NyLPC_TSocketType i_socktype)
{
	NyLPC_TUInt16 i;
	switch(i_socktype){
	case NyLPC_TSocketType_TCP_HTTP:
	case NyLPC_TSocketType_TCP_NORMAL:
		//空きソケットの探索
		for(i=0;i<NyLPC_cMiMicIpNetIf_config_TCPSOCKET_MAX;i++){
			//未使用なソケットを得る
			if((tcp_socket_table[i].flags&FLAGS_USED)==0){
				if(!NyLPC_cMiMicIpTcpSocket_initialize(&tcp_socket_table[i].socket,tcp_socket_table[i].rxbuf,NyLPC_cMiMicIpNetIf_config_TCPSOCKET_RX_BUFFER_SIZE)){
					return NULL;
				}
				//ソケットを使用中に
				tcp_socket_table[i].flags|=FLAGS_USED;
				return &(tcp_socket_table[i].socket._super);
			}
		}
		break;
	default:
		break;
	}
	return NULL;
}

static NyLPC_TiUdpSocket_t* createUdpSocetEx(NyLPC_TUInt16 i_port,NyLPC_TSocketType i_socktype)
{
	NyLPC_TUInt16 i;
	switch(i_socktype){
	case NyLPC_TSocketType_UDP_NORMAL:
		//空きソケットの探索
		for(i=0;i<NyLPC_cMiMicIpNetIf_config_UDPSOCKET_MAX;i++){
			//未使用なソケットを得る
			if((udp_socket_table[i].flags&FLAGS_USED)==0){
				if(!NyLPC_cMiMicIpUdpSocket_initialize(&udp_socket_table[i].socket,i_port,udp_socket_table[i].rxbuf,NyLPC_cMiMicIpNetIf_config_UDPSOCKET_RX_BUFFER_SIZE)){
					return NULL;
				}
				udp_socket_table[i].flags|=FLAGS_USED;
				return &(udp_socket_table[i].socket._super);
			}
		}
		break;
	case NyLPC_TSocketType_UDP_NOBUF:
		//空きソケットの探索
		for(i=0;i<NyLPC_cMiMicIpNetIf_config_NB_UDPSOCKET_MAX;i++){
			//未使用なソケットを得る
			if((udp_socket_nb_table[i].flags&FLAGS_USED)==0){
				if(!NyLPC_cMiMicIpUdpSocket_initialize(&udp_socket_nb_table[i].socket,i_port,NULL,0)){
					return NULL;
				}
				udp_socket_nb_table[i].flags|=FLAGS_USED;
				return &(udp_socket_nb_table[i].socket._super);
			}
		}
		break;
	default:
		break;
	}
	return NULL;
}
static NyLPC_TiTcpListener_t* createTcpListener(NyLPC_TUInt16 i_port)
{
	NyLPC_TUInt16 i;
	//空きソケットの探索
	for(i=0;i<NyLPC_cMiMicIpNetIf_config_TCPLISTENER_MAX;i++){
		//未使用なソケットを得る
		if((tcp_listener_table[i].flags&FLAGS_USED)==0){
			if(!NyLPC_cMiMicIpTcpListener_initialize(&tcp_listener_table[i].listener,i_port)){
				return NULL;
			}
			//ソケットを使用中に
			tcp_listener_table[i].flags|=FLAGS_USED;
			return &(tcp_listener_table[i].listener._super);
		}
	}
	return NULL;
}


static const struct NyLPC_TiNetInterface_Interface _interface=
{
	createTcpSocetEx,
	createUdpSocetEx,
	createTcpListener,
	NyLPC_cMiMicIpNetIf_start,
	NyLPC_cMiMicIpNetIf_stop,
	NyLPC_cMiMicIpNetIf_sendArpRequest,
	NyLPC_cMiMicIpNetIf_hasArpInfo,
	isInitService,//NyLPC_TiNetInterface_isInitService isinitservice;
	NyLPC_cMiMicIpNetIf_getInterfaceInfo
};
//--------------------------------------------------------------------------------
// インスタンスのリリース(protected)

void NyLPC_cMiMicIpNetIf_releaseTcpSocketMemory(const NyLPC_TcMiMicIpTcpSocket_t* i_inst)
{
	NyLPC_TUInt16 i;
	//空きソケットの探索
	for(i=0;i<NyLPC_cMiMicIpNetIf_config_TCPSOCKET_MAX;i++){
		if((&tcp_socket_table[i].socket)==i_inst){
			tcp_socket_table[i].flags&=~FLAGS_USED;
			return;
		}
	}
	return;
}
void NyLPC_cMiMicIpNetIf_releaseUdpSocketMemory(const NyLPC_TcMiMicIpUdpSocket_t* i_inst)
{
	NyLPC_TUInt16 i;
	for(i=0;i<NyLPC_cMiMicIpNetIf_config_UDPSOCKET_MAX;i++){
		if((&udp_socket_table[i].socket)==i_inst){
			udp_socket_table[i].flags&=~FLAGS_USED;
			return;
		}
	}
	for(i=0;i<NyLPC_cMiMicIpNetIf_config_NB_UDPSOCKET_MAX;i++){
		if((&udp_socket_nb_table[i].socket)==i_inst){
			udp_socket_nb_table[i].flags&=~FLAGS_USED;
			return;
		}
	}
	return;
}
void NyLPC_cMiMicIpNetIf_releaseTcpListenerMemory(const NyLPC_TcMiMicIpTcpListener_t* i_inst)
{
	NyLPC_TUInt16 i;
	//空きソケットの探索
	for(i=0;i<NyLPC_cMiMicIpNetIf_config_TCPLISTENER_MAX;i++){
		if((&tcp_listener_table[i].listener)==i_inst){
			tcp_listener_table[i].flags&=~FLAGS_USED;
			return;
		}
	}
	return;
}

static NyLPC_TcMiMicIpNetIf_t _netif;

const struct NyLPC_TiNetInterface_Interface* NyLPC_cMiMicIpNetIf_getNetInterface(void)
{
	NyLPC_cMiMicIpNetIf_initialize(&_netif);
	return &_interface;
}



