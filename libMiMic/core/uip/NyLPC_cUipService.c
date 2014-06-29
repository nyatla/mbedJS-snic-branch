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
#include "NyLPC_cUipService_protected.h"
#include "NyLPC_cIPv4IComp_protected.h"
#include "NyLPC_cTcpListener_protected.h"
#include "NyLPC_stdlib.h"
#include "NyLPC_uip.h"







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
NyLPC_TcUipService_t* _NyLPC_TcUipService_inst=NULL;

/**
 * 唯一のインスタンス
 */
static NyLPC_TcUipService_t _service_instance;




/**
 * uipタスク
 */
static int uipTask(void *pvParameters);

/** イーサネットドライバからのハンドラ*/
static void ethernet_handler(void* i_param,NyLPC_TiEthernetDevice_EVENT i_type);

//--------------------------------------------------------------


static NyLPC_TBool sendIPv4Tx(struct NyLPC_TTxBufferHeader* i_eth_buf);

//static void sendArpReqest(const struct TEthPacket* i_eth_packet);
static void sendRawEthFrameNL(void* i_buf,NyLPC_TUInt16 i_len);
static void releaseTxBufNL(void* i_buf);

/**メッセージなし*/
#define TTaskMessage_MSG_NULL    0x0000
/**uipコアタスクに、開始要求する*/
#define TTaskMessage_MSG_RUN     0x0001
/**uipコアタスクに、停止要求する*/
#define TTaskMessage_MSG_STOP    0x0002


NyLPC_TcThread_t th;

NyLPC_TBool NyLPC_cUipService_initialize(void)
{
    NyLPC_TcUipService_t* inst=&_service_instance;
    //サービスは停止している事。 - Service must be uninitialized.
    NyLPC_Assert(!NyLPC_TcUipService_isInitService());
    //IP処理部分の初期化
    NyLPC_cIPv4_initialize(&(inst->_tcpv4));
    //EMAC割込セマフォ
    NyLPC_cSemaphore_initialize(&inst->_emac_semapho);

    inst->_status=0x00;
    NyLPC_cStopwatch_initialize(&(inst->_arp_sw));
    NyLPC_cStopwatch_initialize(&(inst->_periodic_sw));
    NyLPC_cIPv4_initialize(&(inst->_tcpv4));
    NyLPC_AbortIfNot(NyLPC_cMutex_initialize(&(inst->_mutex)));

    _NyLPC_TcUipService_inst=inst;
    //タスク起動
    NyLPC_cThread_initialize(&th,NyLPC_cUipService_config_STACK_SIZE,NyLPC_TcThread_PRIORITY_SERVICE);
    NyLPC_cThread_start(&th,uipTask,NULL);
    return NyLPC_TBool_TRUE;
}







/**
 * UIP処理を開始します。
 * この関数はリエントラントではありません。複数のタスクから共有するときには、排他ロックを掛けてください。
 * @param i_ref_config
 * このコンフィギュレーションは、stopを実行するまでの間、インスタンスから参照します。外部で保持してください。
 */
void NyLPC_cUipService_start(const NyLPC_TcIPv4Config_t* i_ref_config)
{
    NyLPC_TcUipService_t* inst=&_service_instance;
    NyLPC_Assert(NyLPC_TcUipService_isInitService());
    if(!NyLPC_cUipService_isRun())
    {
        //はじめて起動するときに1度だけデバイス取得(タスクスイッチが動いてないと動かないからここで。)
        if(inst->_ethif==NULL){
            inst->_ethif=getEthernetDevicePnP();
        }
        //コンフィグレーションセット
        inst->_ref_config=i_ref_config;
        //開始要求セット
        NyLPC_TUInt16_setBit(inst->_status,NyLPC_TcUipService_ORDER_START);
        //Order実行待ち
        while(NyLPC_TUInt16_isBitOn(inst->_status,NyLPC_TcUipService_ORDER_START)){
            NyLPC_cThread_sleep(10);
        }
    }
    return;
}
/**
 * UIP処理を停止します。
 * この関数はリエントラントではありません。複数のタスクから共有するときには、排他ロックを掛けてください。
 * いまのところ、ストップシーケンスの実装は良くありません。
 * 再設計が必要。
 */
void NyLPC_cUipService_stop(void)
{
    NyLPC_TcUipService_t* inst=&_service_instance;
    NyLPC_Assert(NyLPC_TcUipService_isInitService());
    if(NyLPC_cUipService_isRun())
    {
        NyLPC_TUInt16_setBit(inst->_status,NyLPC_TcUipService_ORDER_STOP);
        //Order実行待ち
        while(NyLPC_TUInt16_isBitOn(inst->_status,NyLPC_TcUipService_ORDER_STOP)){
            NyLPC_cThread_sleep(10);
        }
    }
    return;
}


const char* NyLPC_cUipService_refDeviceName(void)
{
    NyLPC_TcUipService_t* inst=&_service_instance;
    return NyLPC_cUipService_isRun()?NyLPC_iEthernetDevice_getDevicName(inst->_ethif):NULL;
}
const NyLPC_TcIPv4Config_t* NyLPC_cUipService_refCurrentConfig(void)
{
    NyLPC_TcUipService_t* inst=&_service_instance;
    return inst->_ref_config;
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
    NyLPC_TcUipService_t* inst=_NyLPC_TcUipService_inst;
    if(NyLPC_cUipService_isRun()){
        //開始状態
        if(NyLPC_TUInt16_isBitOn(inst->_status,NyLPC_TcUipService_ORDER_STOP))
        {
            //停止操作
            NyLPC_iEthernetDevice_stop(inst->_ethif);
            NyLPC_cIPv4_stop(&(inst->_tcpv4));
            NyLPC_cIPv4IComp_finalize(&(inst->_icomp));
            NyLPC_cIPv4Arp_finalize(&(inst->_arp));
            inst->_ref_config=NULL;
            NyLPC_TUInt16_unsetBit(inst->_status,NyLPC_TcUipService_STATUSBIT_IS_RUNNING);
            NyLPC_TUInt16_unsetBit(inst->_status,NyLPC_TcUipService_ORDER_STOP);
            return NyLPC_TBool_FALSE;
        }
        return NyLPC_TBool_TRUE;
    }else{
        //停止状態
        if(NyLPC_TUInt16_isBitOn(inst->_status,NyLPC_TcUipService_ORDER_START))
        {
            //TCP,ICOMPの初期化
            NyLPC_cIPv4_start(&(inst->_tcpv4),inst->_ref_config);
            NyLPC_cIPv4IComp_initialize(&(inst->_icomp),inst->_ref_config);
            //uip_arp_init(msg->start.ref_config);
            NyLPC_cIPv4Arp_initialize(&(inst->_arp),inst->_ref_config);
            NyLPC_cStopwatch_startExpire(&(inst->_arp_sw),1);//1度ARPを起動するため。
            NyLPC_cStopwatch_startExpire(&(inst->_periodic_sw),PERIODIC_TIMER);
            //EtherNETデバイス初期化
            while(!NyLPC_iEthernetDevice_start(inst->_ethif,&(inst->_ref_config->eth_mac),ethernet_handler,inst));
            NyLPC_TUInt16_setBit(inst->_status,NyLPC_TcUipService_STATUSBIT_IS_RUNNING);
            NyLPC_TUInt16_unsetBit(inst->_status,NyLPC_TcUipService_ORDER_START);
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
    NyLPC_TcUipService_t* inst=_NyLPC_TcUipService_inst;
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
                        NyLPC_cUipService_sendIPv4Tx(r);
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
        NyLPC_cSemaphore_take(&(_NyLPC_TcUipService_inst->_emac_semapho),PERIODIC_TIMER);
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
        NyLPC_cSemaphore_giveFromISR(&(((NyLPC_TcUipService_t*)i_param)->_emac_semapho));
        break;
    default:
        break;
    }
}

/**
 * IPv4パケットのpeerIPを問い合わせるARPパケットを送信します。
 * allocを中でコールするから要UNLOCK状態
 */
void NyLPC_cUipService_sendArpRequest(const struct NyLPC_TIPv4Addr* i_addr)
{
    NyLPC_TcUipService_t* inst=_NyLPC_TcUipService_inst;
    struct NyLPC_TTxBufferHeader* p;
    NyLPC_TUInt16 tx_len;
    struct TEthPacket* ethbuf;
    //システムTxBufを得る
    ethbuf=(struct TEthPacket*)NyLPC_cUipService_allocSysTxBuf();
    //ARPパケットを作る。
    NyLPC_TArpHeader_setArpRequest(&(ethbuf->data.arp),inst->_ref_config->ip_addr,&(inst->_ref_config->eth_mac),i_addr);
    tx_len=NyLPC_TEthernetIIHeader_setArpTx(&(ethbuf->header),&(inst->_ref_config->eth_mac));
    //送信
    p=((struct NyLPC_TTxBufferHeader*)(((struct NyLPC_TEthernetIIHeader*)ethbuf)-1))-1;

    NyLPC_cMutex_lock(&(inst->_mutex));
    NyLPC_iEthernetDevice_sendTxEthFrame(inst->_ethif,p,tx_len);
    NyLPC_iEthernetDevice_releaseTxBuf(inst->_ethif,p);
    NyLPC_cMutex_unlock(&(inst->_mutex));
}




/**
 * allocTxBufで取得したペイロードメモリを"IPパケットとして"送信します。
 * @param i_eth_payload
 * [NyLPC_TTxBufferHeader][NyLPC_TEthernetIIHeader][payload]メモリの、[payload]のアドレスを指定します。
 * 通常は、NyLPC_cUipService_allocTxBufの返却したメモリを指定します。
 */

void NyLPC_cUipService_sendIPv4Tx(void* i_eth_payload)
{
    NyLPC_TcUipService_t* inst=_NyLPC_TcUipService_inst;
    struct NyLPC_TTxBufferHeader* p=((struct NyLPC_TTxBufferHeader*)(((struct NyLPC_TEthernetIIHeader*)i_eth_payload)-1))-1;
    NyLPC_cMutex_lock(&(inst->_mutex));
    //IPパケットの送信を試行
    if(sendIPv4Tx(p)){
        NyLPC_cMutex_unlock(&(inst->_mutex));
        return;
    }
    NyLPC_cMutex_unlock(&(inst->_mutex));
    //ARPリクエストを代わりに送信
    NyLPC_cUipService_sendArpRequest(&((struct NyLPC_TIPv4Header*)i_eth_payload)->destipaddr);
    return;
}


/**
 * ARPテーブルに指定したIPがあるかを返します。
 */
NyLPC_TBool NyLPC_cUipService_hasArpInfo(const struct NyLPC_TIPv4Addr* i_addr)
{
    NyLPC_TcUipService_t* inst=_NyLPC_TcUipService_inst;
    return NyLPC_cIPv4Arp_IPv4toEthAddr(&inst->_arp,*i_addr)!=NULL;
}

/**
 * システム用の送信ペイロードを返します。
 * 関数は必ず成功します。
 */
void* NyLPC_cUipService_allocSysTxBuf(void)
{
    NyLPC_TUInt16 s;
    NyLPC_TcUipService_t* inst=_NyLPC_TcUipService_inst;
    struct NyLPC_TTxBufferHeader* ethbuf;
    //排他処理をして、メモリを取得する。SYSTEMメモリはEthernetドライバの解放待ちのみなのでまとめてLOCKしておｋ
    NyLPC_cMutex_lock(&(inst->_mutex));
    for(;;){
        ethbuf=(struct NyLPC_TTxBufferHeader*)NyLPC_iEthernetDevice_allocTxBuf(inst->_ethif,NyLPC_TcEthernetMM_HINT_CTRL_PACKET,&s);
        if(ethbuf==NULL){
            NyLPC_cThread_yield();
            continue;
        }
        break;
    }
    NyLPC_cMutex_unlock(&(inst->_mutex));
    //イーサネットバッファのアドレスを計算
    return &(((struct TEthPacket*)(ethbuf+1))->data);
}



void* NyLPC_cUipService_allocTxBuf(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size)
{
    NyLPC_TcUipService_t* inst=_NyLPC_TcUipService_inst;
    struct NyLPC_TTxBufferHeader* ethbuf;
    //排他処理をして、メモリを取得する。
    NyLPC_cMutex_lock(&(inst->_mutex));
    ethbuf=(struct NyLPC_TTxBufferHeader*)NyLPC_iEthernetDevice_allocTxBuf(inst->_ethif,i_hint+sizeof(struct NyLPC_TEthernetIIHeader),o_size);
    NyLPC_cMutex_unlock(&(inst->_mutex));
    if(ethbuf==NULL){
        return NULL;
    }
    //イーサネットバッファのサイズを計算
    *o_size-=sizeof(struct NyLPC_TEthernetIIHeader);
    //イーサネットバッファのアドレスを計算
    return &(((struct TEthPacket*)(ethbuf+1))->data);
}


void* NyLPC_cUipService_releaseTxBuf(void* i_buf)
{
    //排他処理をして、メモリを開放する。
    NyLPC_TcUipService_t* inst=_NyLPC_TcUipService_inst;
    NyLPC_cMutex_lock(&(inst->_mutex));
    //ペイロードの位置から、メモリブロックを再生。
    NyLPC_iEthernetDevice_releaseTxBuf(inst->_ethif,((struct NyLPC_TTxBufferHeader*)(((struct NyLPC_TEthernetIIHeader*)i_buf)-1))-1);
    NyLPC_cMutex_unlock(&(inst->_mutex));
    return NULL;
}








/**********
 * イーサネットHWのコントロール関数
 */
/**
 * "IPv4パケットを格納した"イーサフレームを送信します。
 * コール前に、必ずロックしてから呼び出してください。
 *//*
static void copyAndSendIPv4Tx(const struct TEthPacket* i_buf)
{
    NyLPC_TcUipService_t* inst=_NyLPC_TcUipService_inst;
    NyLPC_TUInt16 s;
    //送信する。
    s=NyLPC_htons(i_buf->data.ipv4.len16)+sizeof(struct NyLPC_TEthernetIIHeader);
    memcpy(inst->stx.buf,i_buf,s);
    if(!sendIPv4Tx(&(inst->stx.h))){
        //失敗した場合はARPリクエストに変換して再送
//@todo unchecked PASS!
        sendArpReqest(&i_buf->data.ipv4.destipaddr);
    }
    return;
}*/
/**
 * "IPv4パケットを格納した"イーサフレームを送信します。
 * コール前に、必ずロックしてから呼び出してください。
 */
/*
static void copyAndSendIPv4Tx(const struct TEthPacket* i_buf)
{
    NyLPC_TcUipService_t* inst=_NyLPC_TcUipService_inst;
    NyLPC_TUInt16 s;
    //ACK送信用の自己バッファが空くまで待つ
    while(inst->stx.h.is_lock){
        NyLPC_iEthernetDevice_processTx(inst->_ethif);
    }
    //送信する。
    s=NyLPC_htons(i_buf->data.ipv4.len16)+sizeof(struct NyLPC_TEthernetIIHeader);
    memcpy(inst->stx.buf,i_buf,s);
    if(!sendIPv4Tx(&(inst->stx.h))){
        //失敗した場合はARPリクエストに変換して再送
//@todo unchecked PASS!
        sendArpReqest(&i_buf->data.ipv4.destipaddr);
    }
    return;
}
*/


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
        _NyLPC_TcUipService_inst->_ethif,
        ((struct NyLPC_TTxBufferHeader*)(((struct NyLPC_TEthernetIIHeader*)i_buf)-1))-1,
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
        _NyLPC_TcUipService_inst->_ethif,
        ((struct NyLPC_TTxBufferHeader*)(((struct NyLPC_TEthernetIIHeader*)i_buf)-1))-1);
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
static NyLPC_TBool sendIPv4Tx(struct NyLPC_TTxBufferHeader* i_eth_buf)
{
    NyLPC_TcUipService_t* inst=_NyLPC_TcUipService_inst;
    struct NyLPC_TEthAddr emac;
    NyLPC_TUInt16 tx_len;
    const struct NyLPC_TEthAddr* eth_dest;
    struct TEthPacket* ethbuf=(struct TEthPacket*)(i_eth_buf+1);
    //ペイロードのアドレスから、イーサネットフレームバッファのアドレスを復元

    if(NyLPC_TIPv4Addr_isEqual(&(ethbuf->data.ipv4.destipaddr),&NyLPC_TIPv4Addr_BROADCAST)) {
        //ブロードキャストならそのまま
        eth_dest=&NyLPC_TEthAddr_BROADCAST;
    }else if(NyLPC_TIPv4Addr_isEqualWithMask(&(ethbuf->data.ipv4.destipaddr),&NyLPC_TIPv4Addr_MULTICAST,&NyLPC_TIPv4Addr_MULTICAST_MASK)){
        //マルチキャスト
        ip2MulticastEmacAddr(&(ethbuf->data.ipv4.destipaddr),&emac);
        eth_dest=&emac;
    }else{
        //LocalIP以外ならdefaultRootへ変換
        eth_dest=NyLPC_cIPv4Arp_IPv4toEthAddr(
            &inst->_arp,
            NyLPC_cIPv4Config_isLocalIP(inst->_ref_config, &(ethbuf->data.ipv4.destipaddr))?(ethbuf->data.ipv4.destipaddr):(inst->_ref_config->dr_addr));
        //IP->MAC変換をテスト。
        if(eth_dest==NULL){
            return NyLPC_TBool_FALSE;
        }
    }
    //変換可能なら、イーサネットヘッダを更新して、送信処理へ。
    tx_len=NyLPC_TEthernetIIHeader_setIPv4Tx(&(ethbuf->header),&(inst->_ref_config->eth_mac),eth_dest);
    NyLPC_iEthernetDevice_sendTxEthFrame(inst->_ethif,i_eth_buf,tx_len);
    return NyLPC_TBool_TRUE;
}




