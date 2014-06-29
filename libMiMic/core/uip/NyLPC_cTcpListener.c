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
#include "NyLPC_cTcpListener_protected.h"
#include "NyLPC_cTcpSocket_protected.h"
#include "NyLPC_cUipService_protected.h"
#include "NyLPC_cIPv4.h"
#include "NyLPC_stdlib.h"
#include <stdio.h>
/**
 * NyLPC_TTcpListenerListenQ
 */

void NyLPC_TTcpListenerListenQ_init(struct NyLPC_TTcpListenerListenQ* i_struct)
{
    i_struct->wp=0;
    int i;
    for(i=NyLPC_TcTcpListener_NUMBER_OF_Q-1;i>=0;i--){
        i_struct->item[i].rport=0;
    }
}

/**
 * ListenQへSYNパケットの情報を追加する。
 */
void NyLPC_TTcpListenerListenQ_add(struct NyLPC_TTcpListenerListenQ* i_struct,const NyLPC_TcIPv4Payload_t* i_payload)
{
    struct NyLPC_TTcpSocketSynParam* item=&i_struct->item[i_struct->wp];
    //未処理のものがあれば登録しない。
    if(item->rport!=0){
        return;
    }

    //SYNリングバッファにセット
    item->rport = i_payload->payload.tcp->srcport;
    item->srcaddr=i_payload->header->srcipaddr;
    item->rcv_nxt32=NyLPC_ntohl(i_payload->payload.tcp->seqno32)+1;
    //MSSの設定
    if(!NyLPC_TTcpHeader_getTcpMmsOpt(i_payload->payload.tcp,&item->mss)){
        item->mss=0;
    }
    //書込み位置の進行
    i_struct->wp=(i_struct->wp+1)%NyLPC_TcTcpListener_NUMBER_OF_Q;
}

/**
 * 最も古いSYNパケット情報のインデクスをキューから返す。
 * @return
 * 見つからない場合-1である。
 */
int NyLPC_TTcpListenerListenQ_getLastIndex(struct NyLPC_TTcpListenerListenQ* i_struct)
{
    int i,t;
    //古いものから順に返す
    for(i=1;i<=NyLPC_TcTcpListener_NUMBER_OF_Q;i++){
        t=(i_struct->wp+i)%NyLPC_TcTcpListener_NUMBER_OF_Q;
        //有効なデータ?
        if(i_struct->item[t].rport!=0){
            return t;
        }
    }
    return -1;
}

/**
 * ListenQのN番目を削除する。
 */
void NyLPC_TTcpListenerListenQ_remove(struct NyLPC_TTcpListenerListenQ* i_struct,int i_idx)
{
    i_struct->item[i_idx].rport=0;
    return;
}






//#define lockResource(i_inst) NyLPC_cMutex_lock(((i_inst)->_mutex))
//#define unlockResource(i_inst) NyLPC_cMutex_unlock(((i_inst)->_mutex))
#define lockResource(i_inst) NyLPC_cMutex_lock(NyLPC_cIPv4_getListenerMutex(((i_inst)->_super._parent_ipv4)))
#define unlockResource(i_inst) NyLPC_cMutex_unlock(NyLPC_cIPv4_getListenerMutex(((i_inst)->_super._parent_ipv4)))


/**
 * uipサービスが稼働中にのみ機能します。
 */
NyLPC_TBool NyLPC_cTcpListener_initialize(NyLPC_TcTcpListener_t* i_inst,NyLPC_TUInt16 i_port)
{
    NyLPC_TcUipService_t* srv=_NyLPC_TcUipService_inst;
    NyLPC_cBaseSocket_initialize(&(i_inst->_super),NyLPC_TcBaseSocket_TYPEID_TCP_LISTENER);
    NyLPC_TTcpListenerListenQ_init(&i_inst->_listen_q);
    //uipサービスは初期化済であること。
    NyLPC_Assert(NyLPC_TcUipService_isInitService());
    //初期化
//  //  NyLPC_cMutex_initialize(&(i_inst->_mutex));
//  i_inst->_mutex=NyLPC_cIPv4_getListenerMutex(&srv->_tcpv4);//    NyLPC_cMutex_initialize(&(i_inst->_mutex));
    i_inst->_port=NyLPC_htons(i_port);
    //管理リストへ登録。
    return NyLPC_cIPv4_addSocket(&(srv->_tcpv4),&(i_inst->_super));
}

void NyLPC_cTcpListener_finaize(NyLPC_TcTcpListener_t* i_inst)
{
    NyLPC_TcUipService_t* srv=_NyLPC_TcUipService_inst;
    NyLPC_Assert(NyLPC_TcUipService_isInitService());
    //uipサービスは初期化済であること。
    if(!NyLPC_cIPv4_removeSocket(&(srv->_tcpv4),&(i_inst->_super))){
        //削除失敗、それは死を意味する。
        NyLPC_Abort();
    }
    NyLPC_cBaseSocket_finalize(&(i_inst->_super));
    return;
}




NyLPC_TBool NyLPC_cTcpListener_listen(NyLPC_TcTcpListener_t* i_inst,NyLPC_TcTcpSocket_t* i_sock,NyLPC_TUInt32 i_wait_msec)
{
    int qi;
    NyLPC_TcStopwatch_t sw;
    NyLPC_TBool ret=NyLPC_TBool_FALSE;
    //サービスは稼働中であること。
    NyLPC_Assert(NyLPC_cUipService_isRun());

    //入力ソケットはCLOSEDであること。
    if(i_sock->tcpstateflags!=UIP_CLOSED){
        return NyLPC_TBool_FALSE;
    }

    //ストップウォッチを起動
    NyLPC_cStopwatch_initialize(&sw);
    NyLPC_cStopwatch_setNow(&sw);


    //Listenerのリソースロック
    lockResource(i_inst);

    while(NyLPC_cStopwatch_elapseInMsec(&sw)<i_wait_msec){
        qi=NyLPC_TTcpListenerListenQ_getLastIndex(&i_inst->_listen_q);
        if(qi>=0){
            //SYN処理要求がある
            if(!NyLPC_cTcpSocket_listenSyn(i_sock,&i_inst->_listen_q.item[qi],i_inst->_port)){
                ret=NyLPC_TBool_FALSE;
            }else{
                //成功
                ret=NyLPC_TBool_TRUE;
            }
            //処理したSYNの削除
            NyLPC_TTcpListenerListenQ_remove(&i_inst->_listen_q,qi);
            break;
        }else{
            //SYN処理要求は無い(しばらくまつ)
            unlockResource(i_inst);
            NyLPC_cThread_yield();
            lockResource(i_inst);
        }
    }
    //タイムアウト
    unlockResource(i_inst);
    NyLPC_cStopwatch_finalize(&sw);
    return ret;
}

/**
 * この関数は、Uip受信タスクから実行します。
 */
NyLPC_TBool NyLPC_cTcpListener_synPacket(NyLPC_TcTcpListener_t* i_inst,const NyLPC_TcIPv4Payload_t* i_payload)
{
    //パケットチェック。SYN設定されてる？
    if(!(i_payload->payload.tcp->flags & TCP_SYN)){
        //SYNない
        return NyLPC_TBool_FALSE;
    }
    //peer port==0は受け取らない。
    if(i_payload->payload.tcp->srcport==0){
        return NyLPC_TBool_FALSE;
    }
    //Listenerのリソースロック
    lockResource(i_inst);
    //ListenQへ追加
    NyLPC_TTcpListenerListenQ_add(&(i_inst->_listen_q),i_payload);
    //Listenerのリソースアンロック
    unlockResource(i_inst);
    return NyLPC_TBool_TRUE;
}




