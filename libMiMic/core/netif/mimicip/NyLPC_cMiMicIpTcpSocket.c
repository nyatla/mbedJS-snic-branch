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
#include "NyLPC_cMiMicIpTcpSocket_protected.h"
#include "NyLPC_stdlib.h"
#include "NyLPC_cMiMicIpNetIf_protected.h"


static NyLPC_TUInt32 iss32=3939;
#define SIZE_OF_IPv4_TCPIP_HEADER 40

/**
 * TCPのRTOの最大値。
 * ms単位である。
 * defaultは64SEC
 */
#define UIP_IP_RTO_MAX_RTO 64000
/**
 * TCPのRTOの初期値。
 * ms単位である。
 * 伝送路の特性に合わせて調整すること。
 */
#define UIP_TCP_RTO_INITIAL 3000

/**
 * CONNECTION時のRTO
 */
#define UIP_TCP_RTO_CONNECTION_INITIAL 200

/**
 * 下限値
 */
#define UIP_TCP_RTO_MINIMUM 100


/**
 * for Debug
 * RTOの情報をログ領域に取る。
 */
#ifdef RTO_LOG
    NyLPC_TUInt32 rto_log[256];
    int rto_log_st=0;
    #define DEBUG_RTO_LOG(i_inst) if(rto_log_st<256){rto_log[rto_log_st++]=i_inst->uip_connr.current_rto32;};
#else
    #define DEBUG_RTO_LOG(i_inst)
#endif

//#define lockResource(i_inst) NyLPC_cMutex_lock(&((i_inst)->_smutex))
//#define unlockResource(i_inst) NyLPC_cMutex_unlock(&((i_inst)->_smutex))
#define lockResource(i_inst) NyLPC_cMutex_lock(NyLPC_cIPv4_getSockMutex(((i_inst)->_super._parent_ipv4)))
#define unlockResource(i_inst) NyLPC_cMutex_unlock(NyLPC_cIPv4_getSockMutex(((i_inst)->_super._parent_ipv4)))

static void sendRst(NyLPC_TcMiMicIpTcpSocket_t* i_inst);




////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Packet writer
//
////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * TCPヘッダに値をセットする。checksum,wndは0初期化する。
 */
static void setTcpTxHeader(struct NyLPC_TTcpHeader* i_struct,NyLPC_TUInt8 i_flag,const struct uip_conn* i_conn)
{
    i_struct->flags    = i_flag;
    //sorce & destination port
    i_struct->srcport  = i_conn->lport;
    i_struct->destport = i_conn->rport;
    //ACK number
    i_struct->ackno32  = NyLPC_htonl(i_conn->rcv_nxt32);
    //Seq Number
    i_struct->seqno32  = NyLPC_htonl(i_conn->snd_nxt32);
    //uip_func_tcp_send_noconn(BUF);
    i_struct->urgp[0]  = i_struct->urgp[1] = 0;
    i_struct->tcpchksum= 0;
}

static void setTxPacket(const NyLPC_TcMiMicIpTcpSocket_t* i_inst,void* i_tx_buf,NyLPC_TUInt8 i_tcpf,const void* i_buf,NyLPC_TUInt16 i_len)
{
    struct NyLPC_TIPv4Header*   iph;
    struct NyLPC_TTcpHeader*    tcph;
    NyLPC_TUInt8 iph_word=0x05;
    NyLPC_TUInt8 tcph_word=(UIP_TCPH_LEN) / 4;
    //IPヘッダの更新
    iph=(struct NyLPC_TIPv4Header*)i_tx_buf;
    iph->vhl=0x40|(0x0f&iph_word);
    iph->destipaddr=i_inst->uip_connr.ripaddr;
    iph->srcipaddr =*(i_inst->uip_connr.lipaddr);
    NyLPC_TIPv4Header_writeTxIpHeader(iph,UIP_PROTO_TCP);
    //TCPヘッダの更新
    tcph=(struct NyLPC_TTcpHeader*)(((NyLPC_TUInt8*)i_tx_buf)+NyLPC_TIPv4Header_getHeaderLength(iph));


    //SYNが有るならMSSの書き込み
    if((TCP_SYN & i_tcpf)){
        tcph_word+=((TCP_OPT_MSS_LEN) / 4);
        NyLPC_TTcpHeader_setMmsOpt(((NyLPC_TUInt8*)(tcph+1)),i_inst->uip_connr.default_mss);
    }
    tcph->tcpoffset=(tcph_word<<4);
    setTcpTxHeader(tcph,i_tcpf,&(i_inst->uip_connr));

    //最終的なパケットサイズと必要ならペイロードを書き込み
    if(i_buf!=NULL){
        iph->len16=NyLPC_htons(i_len+(iph_word+tcph_word)*4);
        memcpy(((NyLPC_TUInt8*)i_tx_buf)+((iph_word+tcph_word)*4),i_buf,i_len);
    }else{
        iph->len16=NyLPC_htons((iph_word+tcph_word)*4);
    }
    //WND設定
    tcph->wnd16=NyLPC_htons(NyLPC_cFifoBuffer_getSpace(&(i_inst->rxbuf)));
    //Checksumの生成
    tcph->tcpchksum=~(NyLPC_TIPv4Header_makeTcpChecksum(iph));
    iph->ipchksum = ~(NyLPC_TIPv4Header_makeIpChecksum(iph));
    return;
}

/**
 * IP/TCPヘッダが40バイト固定として、i_tx_buf+40の位置にあるペイロードに対するIP/TCPヘッダを書き込みます。
 */
static void setTxPacketHeader(const NyLPC_TcMiMicIpTcpSocket_t* i_inst,void* i_tx_buf,NyLPC_TUInt8 i_tcpf,NyLPC_TUInt16 i_len)
{
    struct NyLPC_TIPv4Header*   iph;
    struct NyLPC_TTcpHeader*    tcph;
    NyLPC_TUInt8 iph_word=0x05;
    NyLPC_TUInt8 tcph_word=(UIP_TCPH_LEN) / 4;
    //IPヘッダの更新
    iph=(struct NyLPC_TIPv4Header*)i_tx_buf;
    iph->vhl=0x40|(0x0f&iph_word);
    iph->destipaddr=i_inst->uip_connr.ripaddr;
    iph->srcipaddr =*(i_inst->uip_connr.lipaddr);
    NyLPC_TIPv4Header_writeTxIpHeader(iph,UIP_PROTO_TCP);

    //TCPヘッダの更新
    tcph=(struct NyLPC_TTcpHeader*)(((NyLPC_TUInt8*)i_tx_buf)+NyLPC_TIPv4Header_getHeaderLength(iph));
    tcph->tcpoffset=(tcph_word<<4);
    setTcpTxHeader(tcph,i_tcpf,&(i_inst->uip_connr));

    //最終的なパケットサイズと必要ならペイロードを書き込み
    iph->len16=NyLPC_htons(i_len+(iph_word+tcph_word)*4);
    //WND設定
    tcph->wnd16=NyLPC_htons(NyLPC_cFifoBuffer_getSpace(&(i_inst->rxbuf)));
    //Checksumの生成
    tcph->tcpchksum=~(NyLPC_TIPv4Header_makeTcpChecksum(iph));
    iph->ipchksum = ~(NyLPC_TIPv4Header_makeIpChecksum(iph));
    return;
}





////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Mainclass::private
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * ACK番号を更新する。
 * @param i_ackno
 * ネットワークオーダーのACK番号
 */
static void updateAckNo(void* i_tx_buf,NyLPC_TUInt32 i_ackno)
{
    struct NyLPC_TIPv4Header*   iph=(struct NyLPC_TIPv4Header*)i_tx_buf;
    struct NyLPC_TTcpHeader*    tcph=(struct NyLPC_TTcpHeader*)(((NyLPC_TUInt8*)i_tx_buf)+NyLPC_TIPv4Header_getHeaderLength(iph));

/*  union{
        NyLPC_TUInt32 l;
        NyLPC_TUInt8 b[4];
    }old_ack,new_ack;
    NyLPC_TUInt16 v1;
    //checksumの計算
    old_ack.l=i_inst->payload.tcp->ackno32;//古いACK番号
    new_ack.l=i_ackno;//新しいACK番号
    v1=NyLPC_ntohs(~(i_inst->payload.tcp->tcpchksum));//1の補数を取って、ホストオーダーに戻す。
    //減算
    v1=sub16c(v1,(old_ack.b[0]<<8)+old_ack.b[1]);
    v1=sub16c(v1,(old_ack.b[2]<<8)+old_ack.b[3]);
    //加算
    v1=add16c(v1,(new_ack.b[0]<<8)+new_ack.b[1]);
    v1=add16c(v1,(new_ack.b[2]<<8)+new_ack.b[3]);
    v1=~NyLPC_htons(v1);*/
NyLPC_Trace();
    tcph->ackno32=i_ackno;
NyLPC_Trace();
    tcph->tcpchksum = 0;
NyLPC_Trace();
    tcph->tcpchksum = ~(NyLPC_TIPv4Header_makeTcpChecksum(iph));
NyLPC_Trace();

/*
    if((i_inst->payload.tcp->tcpchksum!=v1)){
        NyLPC_Warning();
    }*/
}



/**
 * 指定した送信パケットがACK済であるか調べる。
 */
static NyLPC_TBool isPacketAcked(NyLPC_TcMiMicIpTcpSocket_t* i_inst,NyLPC_TUInt32 i_sq)
{
    int rp;
    struct NyLPC_TcTcpSocket_TxQItem* q=i_inst->txbuf.txq;
    rp=i_inst->txbuf.rp;
    while(rp!=i_inst->txbuf.wp){
        if(q[rp].ackno==i_sq){
            return NyLPC_TBool_FALSE;
        }
        rp=(rp+1)%NyLPC_TcTcpSocket_NUMBER_OF_TXQ;
    }
    return NyLPC_TBool_TRUE;
}
/**
 * 送信キューからi_sq以前に送信したパケットを除外して、残り個数を返却する。
 */
static int getNumOfSending(NyLPC_TcMiMicIpTcpSocket_t* i_inst,NyLPC_TUInt32 i_sq)
{
    int rp,n;
    struct NyLPC_TcTcpSocket_TxQItem* q=i_inst->txbuf.txq;
    rp=i_inst->txbuf.rp;
    n=0;
    while(rp!=i_inst->txbuf.wp){
        if(q[rp].ackno==i_sq){
            return n;
        }
        n++;
        rp=(rp+1)%NyLPC_TcTcpSocket_NUMBER_OF_TXQ;
    }
    return n;
}
/**
 * この関数は、コネクションをリセットします。
 * ロック状態でコールしてください。
 * 関数は、現在バッファにある再送信待ちデータを開放します。
 */
static void resetTxQWithUnlock(NyLPC_TcMiMicIpTcpSocket_t* i_inst)
{
    int i,l;
    struct NyLPC_TcTcpSocket_TxQItem* q=i_inst->txbuf.txq;
    void* dlist[NyLPC_TcTcpSocket_NUMBER_OF_TXQ];

    l=0;
    while(i_inst->txbuf.rp!=i_inst->txbuf.wp){
        dlist[l]=q[i_inst->txbuf.rp].packet;
        l++;
        i_inst->txbuf.rp=(i_inst->txbuf.rp+1)%NyLPC_TcTcpSocket_NUMBER_OF_TXQ;
    }
    i_inst->txbuf.rp=i_inst->txbuf.wp=0;
    //ロック解除
    unlockResource(i_inst);
    //セーブしたバッファを開放
    for(i=0;i<l;i++){
        NyLPC_cMiMicIpNetIf_releaseTxBuf(dlist[i]);
    }
    return;
}
/**
 * TXバッファの再送パケットのACK番号を更新します。
 * ロックして実行してください。
 * @param i_ackno
 * ネットワークオーダーのACK番号
 */
static void updateTxAck(NyLPC_TcMiMicIpTcpSocket_t* i_inst,NyLPC_TUInt32 i_ackno)
{
    NyLPC_TUInt8 rp;
    struct NyLPC_TcTcpSocket_TxQItem* q=i_inst->txbuf.txq;
    NyLPC_ArgAssert(i_inst!=NULL);
    rp=i_inst->txbuf.rp;
    while(rp!=i_inst->txbuf.wp){
        updateAckNo(q[rp].packet,i_ackno);
        rp=(rp+1)%NyLPC_TcTcpSocket_NUMBER_OF_TXQ;
    }
}

/**
 * RTOの予測関数
 */
static void estimateRTO(NyLPC_TcMiMicIpTcpSocket_t* i_inst,int s,int n)
{
    NyLPC_TcStopwatch_t sw;
    NyLPC_TUInt32 cr_rtt_min,cr_rtt_max,sk_rto,new_rto,w;
    int i;
    struct NyLPC_TcTcpSocket_TxQItem* q=i_inst->txbuf.txq;
    NyLPC_cStopwatch_initialize(&sw);

    sk_rto=i_inst->uip_connr.current_rto32;
    //ACKされたパケットの個数は？
    switch(n){
    case 1:
        NyLPC_cStopwatch_set(&sw,q[s].tick_of_sent);
        cr_rtt_min=NyLPC_cStopwatch_elapseInMsec(&sw);
        if(sk_rto<cr_rtt_min){
            //現在のRTOよりも大きい→再送があった。(再送の理由が回線遅延によるものかわからないので、基本RTOを25%増やす。)
            new_rto=sk_rto*10/8;
        }else if(sk_rto/4<cr_rtt_min){
            //現在のRTOの1/4< n < 現在のRTO　想定内の変動。1/8
            new_rto=(sk_rto+(cr_rtt_min*3*7))/8;
        }else{
            //現在の1/4以下。RTOを再計算。 RTOが大きすぎるので再計算。(計測値を優先した現在値との平均値)
            new_rto=(sk_rto+(cr_rtt_min*3*3))/4;
        }
        break;
    default:
        //複数のパケットなら、最大と最小の時刻を得る。
        NyLPC_cStopwatch_set(&sw,q[s].tick_of_sent);
        cr_rtt_min=cr_rtt_max=NyLPC_cStopwatch_elapseInMsec(&sw);
        for(i=1;i<n;i++){
            NyLPC_cStopwatch_set(&sw,q[(s+i)%NyLPC_TcTcpSocket_NUMBER_OF_TXQ].tick_of_sent);
            w=NyLPC_cStopwatch_elapseInMsec(&sw);
            if(cr_rtt_min>w){
                cr_rtt_min=w;
            }
            if(cr_rtt_max<w){
                cr_rtt_max=w;
            }
        }
        if(sk_rto<cr_rtt_min && sk_rto<cr_rtt_max){
            //最大値,最小値とも現在のRTTより大きい→低速な回線を検出。
            new_rto=cr_rtt_max*10/8;//最大経過時間の25%増しの時間を設定。
        }else if(sk_rto/4<cr_rtt_min){
            //現在のRTOの1/4< n < 現在のRTO 想定範囲内。1/8の加重平均で速度計算。
            new_rto=(sk_rto+(cr_rtt_min*3*7))/8;
        }else{
            //現在の1/4以下。RTOが大きすぎるので再計算。(計測値を優先した加重平均)
            new_rto=(sk_rto+(cr_rtt_min*3*3))/4;
        }
        break;
    }
    NyLPC_cStopwatch_finalize(&sw);
    if(new_rto<UIP_TCP_RTO_MINIMUM){
        new_rto=UIP_TCP_RTO_MINIMUM;
    }
    i_inst->uip_connr.current_rto32=new_rto;
}

/**
 * TXキューから、入力されたシーケンス番号より前のパケットを除外します。
 * リングバッファのrp->wp-1までをチェックして、sqに等しいi_sq以前のパケットバッファをo_dlistへ返します。
 *
 */
static int updateTxQByIndex(NyLPC_TcMiMicIpTcpSocket_t* i_inst,NyLPC_TUInt32 i_sq,void* o_dlist[])
{
    int rp,n;
    struct NyLPC_TcTcpSocket_TxQItem* q=i_inst->txbuf.txq;
    //ロック状態なう
    rp=i_inst->txbuf.rp;
    n=0;
    //This is debug
    DEBUG_RTO_LOG(i_inst);

    while(rp!=i_inst->txbuf.wp){
        o_dlist[n]=q[rp].packet;
        if(q[rp].ackno==i_sq){
            //i_inst->txbuf.rp->rpのパケットのRTOからbaseRTOの値を再計算。
            estimateRTO(i_inst,i_inst->txbuf.rp,n+1);
            i_inst->txbuf.rp=(rp+1)%NyLPC_TcTcpSocket_NUMBER_OF_TXQ;
            return n+1;
        }
        n++;
        rp=(rp+1)%NyLPC_TcTcpSocket_NUMBER_OF_TXQ;
    }
    return 0;
}



/**
 * 空きキューを1個返します。
 * 空きキューの
 */
static struct NyLPC_TcTcpSocket_TxQItem* getTxQ(NyLPC_TcMiMicIpTcpSocket_t* i_inst,NyLPC_TcStopwatch_t* i_timer)
{
    int i;
    struct NyLPC_TcTcpSocket_TxQItem* q=i_inst->txbuf.txq;
    do{
        //クローズドに遷移してしまったら、エラーである。
        if(i_inst->tcpstateflags==UIP_CLOSED){
            return NULL;
        }
        //キューの空きをチェック。wp+1==rpなら、キューがいっぱい。rp==wpなら、キューが空。
        if(((i_inst->txbuf.wp+1)%NyLPC_TcTcpSocket_NUMBER_OF_TXQ)==i_inst->txbuf.rp){
            //一時的なアンロック
            unlockResource(i_inst);
            //タスクスイッチ
            NyLPC_cThread_yield();
            //ロック
            lockResource(i_inst);
            continue;
        }
        i=i_inst->txbuf.wp;
        i_inst->txbuf.wp=(i+1)%NyLPC_TcTcpSocket_NUMBER_OF_TXQ;
        return &(q[i]);
    }while(!NyLPC_cStopwatch_isExpired(i_timer));
    //失敗。タイムアウト。
    return NULL;
}






/**********************************************************************
 * public 関数
 **********************************************************************/
static const struct NyLPC_TIPv4Addr* getPeerAddr(const NyLPC_TiTcpSocket_t* i_inst);
static NyLPC_TUInt16 getPeerPort(const NyLPC_TiTcpSocket_t* i_inst);
static NyLPC_TBool accept(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt32 i_wait_in_msec);
static NyLPC_TInt32 precv(NyLPC_TiTcpSocket_t* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_wait_msec);
static void pseek(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt16 i_seek);
static NyLPC_TInt32 send(NyLPC_TiTcpSocket_t* i_inst,const void* i_buf_ptr,NyLPC_TInt32 i_len,NyLPC_TUInt32 i_wait_in_msec);
static void close(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt32 i_wait_in_msec);
static void* allocSendBuf(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_buf_size,NyLPC_TUInt32 i_wait_in_msec);
static void releaseSendBuf(NyLPC_TiTcpSocket_t* i_inst,void* i_buf_ptr);
static NyLPC_TBool psend(NyLPC_TiTcpSocket_t* i_inst,void* i_buf_ptr,int i_len,NyLPC_TUInt32 i_wait_in_msec);
static NyLPC_TBool connect(NyLPC_TiTcpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_peer_port,NyLPC_TUInt32 i_wait_in_msec);
static void finalize(NyLPC_TiTcpSocket_t* i_inst);

const static struct NyLPC_TiTcpSocket_Interface _interface=
{
	getPeerAddr,
	getPeerPort,
	accept,
	precv,
	pseek,
	send,
	close,
	allocSendBuf,
	releaseSendBuf,
	psend,
	connect,
	finalize
};

static const struct NyLPC_TIPv4Addr* getPeerAddr(const NyLPC_TiTcpSocket_t* i_inst)
{
	const NyLPC_TcMiMicIpTcpSocket_t* inst=(const NyLPC_TcMiMicIpTcpSocket_t*)i_inst;
	return &inst->uip_connr.ripaddr;
}
static NyLPC_TUInt16 getPeerPort(const NyLPC_TiTcpSocket_t* i_inst)
{
	const NyLPC_TcMiMicIpTcpSocket_t* inst=(const NyLPC_TcMiMicIpTcpSocket_t*)i_inst;
	return inst->uip_connr.rport;
}




NyLPC_TBool NyLPC_cMiMicIpTcpSocket_initialize(NyLPC_TcMiMicIpTcpSocket_t* i_inst,void* i_rbuf,NyLPC_TUInt16 i_rbuf_len)
{
    int i;
    NyLPC_TcMiMicIpNetIf_t* srv=_NyLPC_TcMiMicIpNetIf_inst;
    i_inst->_super._super.tcp_sock._interface=&_interface;
    NyLPC_cMiMicIpBaseSocket_initialize(&(i_inst->_super),NyLPC_TcMiMicIpBaseSocket_TYPEID_TCP_SOCK);
    //uipサービスは初期化済であること。
    NyLPC_Assert(NyLPC_cMiMicIpNetIf_isInitService());

    NyLPC_cFifoBuffer_initialize(&(i_inst->rxbuf),i_rbuf,i_rbuf_len);
    //  NyLPC_AbortIfNot(NyLPC_cMutex_initialize(&(i_inst->_smutex)));//個別Mutex
//  i_inst->_smutex=NyLPC_cIPv4_getSockMutex(&(srv->_tcpv4));//共有Mutex
    i_inst->tcpstateflags=UIP_CLOSED;
    i_inst->txbuf.rp=i_inst->txbuf.wp=0;
    for(i=0;i<NyLPC_TcTcpSocket_NUMBER_OF_TXQ;i++){
        i_inst->txbuf.txq[i].packet=NULL;
    }
    //管理リストへ登録。
    return NyLPC_cIPv4_addSocket(&(srv->_tcpv4),&(i_inst->_super));
}



NyLPC_TBool NyLPC_cMiMicIpTcpSocket_listenSyn(NyLPC_TcMiMicIpTcpSocket_t* i_inst,const struct NyLPC_TTcpSocketSynParam* i_lq,NyLPC_TUInt16 i_lport)
{
//  NyLPC_Assert(NyLPC_cMutex_isLocked(i_inst->_smutex));
    lockResource(i_inst);
    //ソケットが無効であること。
    if(i_inst->tcpstateflags==UIP_CLOSED)
    {
        //localipとdefault_mmsは別枠で設定
        /* Fill in the necessary fields for the new connection. */
        i_inst->uip_connr.current_rto32 = UIP_TCP_RTO_INITIAL;
        i_inst->uip_connr.lport = i_lport;
        i_inst->uip_connr.rport = i_lq->rport;
        i_inst->uip_connr.ripaddr=i_lq->srcaddr;
        i_inst->uip_connr.snd_nxt32=iss32;
        /* rcv_nxt should be the seqno from the incoming packet + 1. */
        i_inst->uip_connr.rcv_nxt32= i_lq->rcv_nxt32;
        //MSSの設定
        i_inst->uip_connr.peer_mss=(i_lq->mss!=0)?i_lq->mss:i_inst->uip_connr.default_mss;
        i_inst->uip_connr.peer_win=0;
        NyLPC_cFifoBuffer_clear(&(i_inst->rxbuf));
        //ここでステータスがかわる。
        i_inst->tcpstateflags = UIP_SYN_RCVD;
        //前回のデータが残っていた場合の保険
        if(i_inst->txbuf.rp!=i_inst->txbuf.wp){
            resetTxQWithUnlock(i_inst);
        }else{
            unlockResource(i_inst);
        }
        return NyLPC_TBool_TRUE;
    }
    unlockResource(i_inst);
    return NyLPC_TBool_FALSE;
}


/**
 * sq番のTxがキューから消え去るのを待ちます。
 * この関数は、アンロック状態でコールしてください。
 * <div>
 * パケットがキューからなくなる条件は、以下の２つです。
 * <ul>
 * <li>ACKを受信してパケットキューが更新された。</li>
 * <li>RSTを受信して(CLOSEDに遷移して)、キューがクリアされた。</li>
 * <li>送信タイムアウトで関数が(CLOSEDに遷移させて)キューをクリアした。</li>
 * </ul>
 * </div>
 * @param i_wait_msec
 * @return
 * 1番目の条件でパケットが消失したときのみ、TRUEを返します。
 * 失敗した場合、TCPステータスがCLOSEDでなければ、RSTを送信してステータスをCLOSEDにします。
 */
static NyLPC_TBool waitForTxRemove(NyLPC_TcMiMicIpTcpSocket_t* i_inst,NyLPC_TUInt32 i_sq,NyLPC_TcStopwatch_t* i_timer)
{
    NyLPC_TUInt8 f;
    lockResource(i_inst);
    do{
        //パケットが送信中か調べる。
        if(!isPacketAcked(i_inst,i_sq)){
            //まだある場合は、タスクスイッチを繰り返して消失を待つ。
            unlockResource(i_inst);
            NyLPC_cThread_yield();
            lockResource(i_inst);
            continue;
        }
        //なくなった場合は、原因を調べる。
        f=i_inst->tcpstateflags;
        unlockResource(i_inst);
        return (f==UIP_CLOSED)?NyLPC_TBool_FALSE:NyLPC_TBool_TRUE;
    }while(!NyLPC_cStopwatch_isExpired(i_timer));
    unlockResource(i_inst);
    return NyLPC_TBool_FALSE;
}


/**
 * 再送信処理をセットして、パケットを送信します。
 * この関数は「アンロック状態で」実行してください。
 * @param i_len
 * 送信データサイズを指定します。
 * この番号は、シーケンス番号の加算値ではありませんので、注意をしてください。
 * @return
 * <ul>
 * <li>n=-1:送信キューへの投入に失敗した。</li>
 * <li>n>=0:nバイトのデータを送信キューへの投入することに成功した。</li>
 * </ul>
 * 送信キューに失敗する理由は2つあります。1つは、TXバッファがフルでタイムアウト。もうひとつは、非同期なコネクリョンのリセットです。
 * 失敗した場合、TCPステータスがCLOSEDでなければ、RSTを送信してステータスをCLOSEDにします。
 */
static NyLPC_TInt32 sendWithRetransmit(NyLPC_TcMiMicIpTcpSocket_t* i_inst,NyLPC_TUInt8 i_tcpf,const void* i_buf,NyLPC_TUInt16 i_len,NyLPC_TcStopwatch_t* i_timer,NyLPC_TUInt32* o_ack)
{
    struct NyLPC_TcTcpSocket_TxQItem* txq;
    NyLPC_TUInt16 s;
    void* buf;
    NyLPC_TUInt32 next_ack;
    //送信バッファを取得
    //@bug オブションパケット送信時に4バイト足りないメモリ要求しない？問題になってないけど。
    for(;;){
        buf=NyLPC_cMiMicIpNetIf_allocTxBuf(i_len+(SIZE_OF_IPv4_TCPIP_HEADER),&s);
        if(buf!=NULL){
            break;
        }
        //タイムアウト確認
        if(NyLPC_cStopwatch_isExpired(i_timer)){
            return -1;
        }
    };
    lockResource(i_inst);
    //ペイロードがある場合のみ、相手のwindowサイズが0以上になるのを待つ。
    if(i_len>0){
        while(i_inst->uip_connr.peer_win==0){
            unlockResource(i_inst);
            //時間切れならエラー。
            if(NyLPC_cStopwatch_isExpired(i_timer)){
                return -1;
            }
            NyLPC_cThread_yield();
            lockResource(i_inst);
        }
    }
    //送信キューの取得
    txq=getTxQ(i_inst,i_timer);
    //送信キューが取れなかった。
    if(txq==NULL){
        //シーケンス番号をロールバックできないので、エラーとする。
        unlockResource(i_inst);
        NyLPC_cMiMicIpNetIf_releaseTxBuf(buf);
        return -1;
    }

    //送信バッファを基準とした送信サイズを計算
    s-=SIZE_OF_IPv4_TCPIP_HEADER;
    //送信サイズよりMMSが小さければ、送信サイズを修正
    if(i_inst->uip_connr.peer_mss<s){
        s=i_inst->uip_connr.peer_mss;
    }
    //送信サイズよりpeerのウインドウサイズが小さければ修正
    if(i_inst->uip_connr.peer_win<s){
        s=i_inst->uip_connr.peer_win;
    }
    //送信サイズより、データサイズが小さければ、送信サイズを修正
    if(i_len<s){
        s=i_len;
    }
    //ACK番号の計算
    next_ack=i_inst->uip_connr.snd_nxt32+s+(((i_tcpf&(TCP_FIN|TCP_SYN))!=0x00)?1:0);
    txq->rto32=i_inst->uip_connr.current_rto32;
    txq->tick_of_sent=NyLPC_cStopwatch_now();

    //パケットの書き込み
    setTxPacket(i_inst,buf,i_tcpf,i_buf,s);
    txq->packet=buf;

    //シーケンス番号の更新
    i_inst->uip_connr.snd_nxt32=next_ack;
    //Peerのウインドウサイズを更新
    i_inst->uip_connr.peer_win-=s;
    //ACK番号の返却
    *o_ack=txq->ackno=NyLPC_HTONL(next_ack);
    unlockResource(i_inst);
    NyLPC_cMiMicIpNetIf_sendIPv4Tx(buf);
    return s;
}
/**
 * RSTを1フレームだけ送信します。
 * この関数は、クローズドステータスのソケットにしてからコールします。
 * この関数は、アンロック状態でコールしてね。
 */
static void sendRst(NyLPC_TcMiMicIpTcpSocket_t* i_inst)
{
    void* buf;

    NyLPC_Assert(i_inst->tcpstateflags==UIP_CLOSED);
    //ペイロードライタの初期化

    //@bug バッファが取れるまで通信がブロックするの。ここはなんとかしないと。
    buf=NyLPC_cMiMicIpNetIf_allocSysTxBuf();
    lockResource(i_inst);
    i_inst->uip_connr.snd_nxt32++;
    unlockResource(i_inst);
    setTxPacket(i_inst,buf,TCP_RST|TCP_ACK,NULL,0);
    NyLPC_cMiMicIpNetIf_sendIPv4Tx(buf);
    NyLPC_cMiMicIpNetIf_releaseTxBuf(buf);
    NyLPC_cIPv4Payload_finalize(&ipv4);
    return;
}



/**
 * 受信データをバッファに書き込む。
 * 十分な空き領域がない場合、失敗する。
 * この関数は、ロックして実行してください。
 */
static NyLPC_TBool addRecvData(NyLPC_TcMiMicIpTcpSocket_t* i_inst,const void* i_data,NyLPC_TUInt16 i_data_size)
{
    //受信データサイズを確認
    if(NyLPC_cFifoBuffer_getSpace(&(i_inst->rxbuf))>=i_data_size){
        //バッファに格納可能なら、格納。
        NyLPC_cFifoBuffer_push(&(i_inst->rxbuf),i_data,i_data_size);
    }else{
        //エラー:ドロップする。
        return NyLPC_TBool_FALSE;
    }

    return NyLPC_TBool_TRUE;
}





/**
 * Public function
 */
static void finalize(NyLPC_TiTcpSocket_t* i_inst)
{
    int i;
	NyLPC_TcMiMicIpTcpSocket_t* inst=(NyLPC_TcMiMicIpTcpSocket_t*)i_inst;
    NyLPC_Assert(NyLPC_cMiMicIpNetIf_isInitService());
    //uipサービスは初期化済であること。
    if(!NyLPC_cIPv4_removeSocket(&(_NyLPC_TcMiMicIpNetIf_inst->_tcpv4),&(inst->_super))){
        //削除失敗、それは死を意味する。
        NyLPC_Abort();
    }
    //開放漏れの保険
    if(inst->txbuf.rp!=inst->txbuf.wp){
        lockResource(inst);
        resetTxQWithUnlock(inst);
    }
    for(i=0;i<NyLPC_TcTcpSocket_NUMBER_OF_TXQ;i++){
    	inst->txbuf.txq[i].packet=NULL;
    }
    NyLPC_cFifoBuffer_finalize(&(inst->rxbuf));
//  NyLPC_cMutex_finalize(&(i_inst->_smutex));
    NyLPC_cMiMicIpBaseSocket_finalize(&(inst->_super));
    NyLPC_cMiMicIpNetIf_releaseTcpSocketMemory(inst);

    return;
}

static NyLPC_TBool connect(NyLPC_TiTcpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_peer_port,NyLPC_TUInt32 i_wait_in_msec)
{
    volatile NyLPC_TUInt8 f;
	NyLPC_TcMiMicIpTcpSocket_t* inst=(NyLPC_TcMiMicIpTcpSocket_t*)i_inst;
    NyLPC_TUInt32 sq;
    NyLPC_TcStopwatch_t sw;
    NyLPC_TUInt16 lport;
    lockResource(inst);
    //ソケットが無効であること。
    if(inst->tcpstateflags!=UIP_CLOSED)
    {
        NyLPC_OnErrorGoto(Error);
    }
    //ポート番号の取得(lockResourceが他のソケットと共有なので、重複ポートの割当は起こりえない。でもちょっと注意して)
    lport=NyLPC_htons(NyLPC_cIPv4_getNewPortNumber(inst->_super._parent_ipv4));
    if(lport==0){
        NyLPC_OnErrorGoto(Error);
    }
    //connectの為の準備

    //localipとdefault_mmsは別枠で設定
    /* Fill in the necessary fields for the new connection. */
    inst->uip_connr.current_rto32 = UIP_TCP_RTO_CONNECTION_INITIAL;//RTOを短くしてARP発行時の再接続短縮を期待する。
    inst->uip_connr.lport = lport;
    inst->uip_connr.rport = NyLPC_htons(i_peer_port);
    inst->uip_connr.ripaddr=*i_addr;
    inst->uip_connr.snd_nxt32=iss32;//should be random
    /* rcv_nxt should be the seqno from the incoming packet + 1. */
    inst->uip_connr.rcv_nxt32=0;
    //MSSの設定
    inst->uip_connr.peer_mss=inst->uip_connr.default_mss;
    inst->uip_connr.peer_win=1;//periodicの再送信を期待するために相手のWindowサイズは1と仮定する。
    NyLPC_cFifoBuffer_clear(&(inst->rxbuf));
    //ここでステータスがかわる。
    inst->tcpstateflags = UIP_SYN_SENT;
    //前回のデータが残っていた場合の保険
    if(inst->txbuf.rp!=inst->txbuf.wp){
        resetTxQWithUnlock(inst);
    }else{
        unlockResource(inst);
    }

    NyLPC_cStopwatch_initialize(&sw);

    NyLPC_cStopwatch_startExpire(&sw,i_wait_in_msec);
    if(sendWithRetransmit(inst,TCP_SYN,NULL,0,&sw,&sq)==0){
        //ちょっと待つ。
        NyLPC_cThread_yield();
        //キューにあるTXが消えるのを待つ。
        if(waitForTxRemove(inst,sq,&sw)){
            //ACK受信に成功して、TXが消失
            NyLPC_cStopwatch_finalize(&sw);
            return NyLPC_TBool_TRUE;
        }
    }
    //ロックして、強制的なステータス遷移
    lockResource(inst);
    f=inst->tcpstateflags;
    if(f!=UIP_CLOSED){
        //もし、強制CLOSE遷移であれば、RSTも送信。
    	inst->tcpstateflags=UIP_CLOSED;
        unlockResource(inst);
        sendRst(inst);
    }else{
        unlockResource(inst);
    }
    return NyLPC_TBool_FALSE;
Error:
    unlockResource(inst);
    return NyLPC_TBool_FALSE;
}

/**
 * この関数は、UIP_SYN_RCVDステータスのソケットを、ESTABLISHEDへ遷移させます。
 * cTcpListener_listen関数を通過したインスタンスに実行してください。
 * この関数は、アプリケーションが呼び出します。
 * @return
 *
 */
static NyLPC_TBool accept(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt32 i_wait_in_msec)
{
    volatile NyLPC_TUInt8 f;
	NyLPC_TcMiMicIpTcpSocket_t* inst=(NyLPC_TcMiMicIpTcpSocket_t*)i_inst;
	NyLPC_TUInt32 sq;
    NyLPC_TcStopwatch_t sw;

    NyLPC_cStopwatch_initialize(&sw);
    //ステータスチェック
    f=inst->tcpstateflags;
    switch(f)
    {
    case UIP_ESTABLISHED:
        return NyLPC_TBool_TRUE;
    case UIP_SYN_RCVD:
        //処理対象
        break;
    default:
        return NyLPC_TBool_FALSE;
    }
    NyLPC_cStopwatch_startExpire(&sw,i_wait_in_msec);
    if(sendWithRetransmit(inst,TCP_SYN|TCP_ACK,NULL,0,&sw,&sq)==0){
        //ちょっと待つ。
        NyLPC_cThread_yield();
        //キューにあるTXが消えるのを待つ。
        if(waitForTxRemove(inst,sq,&sw)){
            //ACK受信に成功して、TXが消失
            NyLPC_cStopwatch_finalize(&sw);
            return NyLPC_TBool_TRUE;
        }
    }
    //ロックして、強制的なステータス遷移
    lockResource(inst);
    f=inst->tcpstateflags;
    if(f!=UIP_CLOSED){
        //もし、強制CLOSE遷移であれば、RSTも送信。
    	inst->tcpstateflags=UIP_CLOSED;
        unlockResource(inst);
        sendRst(inst);
    }else{
        unlockResource(inst);
    }
    return NyLPC_TBool_FALSE;
}


/**
 * この関数は、ソケットの受信バッファの読み取り位置と、読み出せるデータサイズを返却します。
 * 関数はポインターを返却するだけで、バッファの読み取り位置をシークしません。
 * シークするにはNyLPC_cTcpSocket_pseekを使います。
 */
static NyLPC_TInt32 precv(NyLPC_TiTcpSocket_t* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_wait_msec)
{
	NyLPC_TcMiMicIpTcpSocket_t* inst=(NyLPC_TcMiMicIpTcpSocket_t*)i_inst;
    volatile NyLPC_TUInt8 st;
    NyLPC_TUInt16 rlen;
    //タイマを生成
    NyLPC_TcStopwatch_t sw;
    NyLPC_cStopwatch_initialize(&sw);

    //ESTABLISHED以外の場合は、エラー。
    NyLPC_cStopwatch_setNow(&sw);
    do{
        //読み出しバッファ情報のコピー
        //MUTEX LOCK
        lockResource(inst);
        st=inst->tcpstateflags;
        rlen=NyLPC_cFifoBuffer_getLength(&(inst->rxbuf));
        *o_buf_ptr=NyLPC_cFifoBuffer_getPtr(&(inst->rxbuf));
        //MUTEX UNLOCK
        unlockResource(inst);

        //バッファが空の場合は、ステータスチェック。ESTABLISHEDでなければ、エラー(PASVCLOSE等の場合)
        switch(st){
        case UIP_ESTABLISHED:
            if(rlen>0){
                //バッファにパケットがあれば返却
                NyLPC_cStopwatch_finalize(&sw);
                return rlen;
            }
            break;
        case UIP_CLOSE_WAIT:
            if(rlen>0){
                //バッファにパケットがあれば返却
                NyLPC_cStopwatch_finalize(&sw);
                return rlen;
            }
            //引き続きエラー処理
        default:
            //他の場合はエラー
            NyLPC_cStopwatch_finalize(&sw);
            return -1;
        }
        //タスクスイッチ
        NyLPC_cThread_yield();
    }while(NyLPC_cStopwatch_elapseInMsec(&sw)<i_wait_msec);
    //規定時間内に受信が成功しなかった。
    NyLPC_cStopwatch_finalize(&sw);
    return 0;
}
/**
 * 受信バッファをシークします。
 * シーク後に、遅延ACKを送出します。
 */
static void pseek(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt16 i_seek)
{
    void* buf;
    NyLPC_TcMiMicIpTcpSocket_t* inst=(NyLPC_TcMiMicIpTcpSocket_t*)i_inst;

    NyLPC_ArgAssert(i_seek<=NyLPC_cFifoBuffer_getLength(&(inst->rxbuf)));
    if(i_seek==0){
        return;
    }

    //ACK送信バッファの取得
    buf=NyLPC_cMiMicIpNetIf_allocSysTxBuf();

    //MUTEX LOCK
    lockResource(inst);

    //受信バッファを読み出しシーク
    NyLPC_cFifoBuffer_pop(&(inst->rxbuf),i_seek);
    //ACKパケットの生成
    setTxPacket(inst,buf,TCP_ACK,NULL,0);
    unlockResource(inst);
    //ACK送信
    NyLPC_cMiMicIpNetIf_sendIPv4Tx(buf);
    NyLPC_cMiMicIpNetIf_releaseTxBuf(buf);

}

/**
 * See header file.
 */
static void* allocSendBuf(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_buf_size,NyLPC_TUInt32 i_wait_in_msec)
{
	NyLPC_TcMiMicIpTcpSocket_t* inst=(NyLPC_TcMiMicIpTcpSocket_t*)i_inst;

    NyLPC_TUInt16 s;
    void* buf;
    NyLPC_TcStopwatch_t sw;

    NyLPC_cStopwatch_initialize(&sw);
    NyLPC_cStopwatch_startExpire(&sw,i_wait_in_msec);

    //送信バッファを取得
    //@bug バッファが取れるまで通信がブロックするの。ここはなんとかしないと。
    for(;;){
        //ESTABLISHED以外に非同期遷移
        if(inst->tcpstateflags!=UIP_ESTABLISHED){
            NyLPC_cStopwatch_finalize(&sw);
            return NULL;
        }
        buf=NyLPC_cMiMicIpNetIf_allocTxBuf(i_hint+(SIZE_OF_IPv4_TCPIP_HEADER),&s);
        if(buf!=NULL){
            break;
        }
        //タイムアウト時もエラー
        if(NyLPC_cStopwatch_isExpired(&sw)){
            NyLPC_cStopwatch_finalize(&sw);
            return NULL;
        }
    }

//@todo 前段処理と順番を入れ替えて、要求サイズとpeerのwinのうち、小さいほうを割り当てたほうが良くない？
//ここで相手のwin待ちをする理由は、相手に確実に受け取れるサイズを決定する為。
    lockResource(inst);
    //ペイロードがある場合のみ、相手のwindowサイズが0以上になるのを待つ。
    while(inst->uip_connr.peer_win==0){
        unlockResource(inst);
        //ESTABLISHED以外に非同期遷移 orタイムアウト確認
        if(NyLPC_cStopwatch_isExpired(&sw)||(inst->tcpstateflags!=UIP_ESTABLISHED)){
            NyLPC_cMiMicIpNetIf_releaseTxBuf(buf);
            NyLPC_cStopwatch_finalize(&sw);
            return NULL;
        }
        NyLPC_cThread_yield();
        lockResource(inst);
    }
    //送信バッファを基準とした送信サイズを計算
    s-=SIZE_OF_IPv4_TCPIP_HEADER;
    //送信サイズよりMMSが小さければ、送信サイズを修正
    if(inst->uip_connr.peer_mss<s){
        s=inst->uip_connr.peer_mss;
    }
    //送信サイズよりpeerのウインドウサイズが小さければ修正
    if(inst->uip_connr.peer_win<s){
        s=inst->uip_connr.peer_win;
    }
    unlockResource(inst);
    //バッファサイズ確定。
    *o_buf_size=s;
    NyLPC_cStopwatch_finalize(&sw);
    return (NyLPC_TUInt8*)buf+SIZE_OF_IPv4_TCPIP_HEADER;
}
/**
 * See Header file.
 */
static void releaseSendBuf(NyLPC_TiTcpSocket_t* i_inst,void* i_buf_ptr)
{
    NyLPC_cMiMicIpNetIf_releaseTxBuf((NyLPC_TUInt8*)i_buf_ptr-SIZE_OF_IPv4_TCPIP_HEADER);
}


/**
 * 事前にAllocしたTxパケットを送信します。
 * このAPIはゼロコピー送信をサポートするためのものです。
 * @param i_buf_ptr
 * allocSendBufで取得したメモリを指定します。
 * @return
 * 関数が失敗した場合、i_buf_ptrは「開放されません。」
 */
static NyLPC_TBool psend(NyLPC_TiTcpSocket_t* i_inst,void* i_buf_ptr,int i_len,NyLPC_TUInt32 i_wait_in_msec)
{
	NyLPC_TcMiMicIpTcpSocket_t* inst=(NyLPC_TcMiMicIpTcpSocket_t*)i_inst;
    struct NyLPC_TcTcpSocket_TxQItem* txq;
    void* buf;
    NyLPC_TcStopwatch_t sw;
    //ESTABLISHEDでなければエラー
    if(inst->tcpstateflags!=UIP_ESTABLISHED){
        //ESTABLISHEDでなければエラー
        return NyLPC_TBool_FALSE;
    }
    //送信データ0なら何もしない。
    if(i_len<1){
        releaseSendBuf(i_inst,i_buf_ptr);
        return NyLPC_TBool_TRUE;
    }
    NyLPC_cStopwatch_initialize(&sw);
    NyLPC_cStopwatch_startExpire(&sw,i_wait_in_msec);

    //先頭ポインタは、i_buf-sizeof(SIZE_OF_IPv4_TCPIP_HEADER)固定
    buf=(NyLPC_TUInt8*)i_buf_ptr-SIZE_OF_IPv4_TCPIP_HEADER;
    lockResource(inst);
    //送信キューの取得
    txq=getTxQ(inst,&sw);
    //送信キューが取れなかった。
    if(txq==NULL){
        //シーケンス番号をロールバックできないので、エラーとする。
        unlockResource(inst);
        NyLPC_cStopwatch_finalize(&sw);
        return NyLPC_TBool_FALSE;
    }
    //ここから先はi_bufの所有権はインスタンスになってる。

    //IPv4ペイロードの書き込み

    //allocをした時点でwin,mssは考慮されているので、そのままそうしんしる。

    //ACK番号の計算
    txq->rto32=inst->uip_connr.current_rto32;
    txq->tick_of_sent=NyLPC_cStopwatch_now();
    //パケットヘッダの生成(ヘッダ長はpreadで定義した値(4+6)*4=40です。)
    setTxPacketHeader(inst,buf,TCP_ACK|TCP_PSH,i_len);
    txq->packet=buf;

    //シーケンス番号の更新
    inst->uip_connr.snd_nxt32=inst->uip_connr.snd_nxt32+i_len;
    //Peerのウインドウサイズを更新
    inst->uip_connr.peer_win-=i_len;
    //ACK番号の返却
    txq->ackno=NyLPC_HTONL(inst->uip_connr.snd_nxt32);
    unlockResource(inst);
    NyLPC_cMiMicIpNetIf_sendIPv4Tx(buf);
    NyLPC_cStopwatch_finalize(&sw);
    return NyLPC_TBool_TRUE;
}

/**
 * See header file.
 */
static NyLPC_TInt32 send(NyLPC_TiTcpSocket_t* i_inst,const void* i_buf_ptr,NyLPC_TInt32 i_len,NyLPC_TUInt32 i_wait_in_msec)
{
    NyLPC_TInt16 hint;
    NyLPC_TUInt16 s;
    void* buf;
    if(i_len<1){
        return 0;
    }
    hint=(i_len>32767)?32767:i_len;
    buf=allocSendBuf(i_inst,hint,&s,i_wait_in_msec);
    if(buf==NULL){
        return -1;
    }
    //送信サイズの計算
    s=((NyLPC_TInt32)s<i_len)?s:(NyLPC_TUInt16)i_len;
    memcpy(buf,i_buf_ptr,s);
    if(!psend(i_inst,buf,s,i_wait_in_msec)){
        releaseSendBuf(i_inst,buf);
        return -1;//error
    }
    return s;
}


static void close(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt32 i_wait_in_msec)
{
	NyLPC_TcMiMicIpTcpSocket_t* inst=(NyLPC_TcMiMicIpTcpSocket_t*)i_inst;
    NyLPC_TcStopwatch_t sw;
    volatile NyLPC_TUInt8 f;
    NyLPC_TUInt32 sq;
    NyLPC_cStopwatch_initialize(&sw);
    NyLPC_cStopwatch_startExpire(&sw,i_wait_in_msec);
    lockResource(inst);

    f=inst->tcpstateflags;
    //ステータスチェック
    switch(f)
    {
    case UIP_CLOSED:
        //閉じている。
        goto ReturnWithUnlock;
    case UIP_ESTABLISHED:
        //アクティブクローズ。
    	inst->tcpstateflags=UIP_FIN_WAIT_1;
        //送信のために一旦解除
        unlockResource(inst);
        //FINの送信
        if(sendWithRetransmit(inst,TCP_FIN|TCP_ACK,NULL,0,&sw,&sq)==0){
            //ちょっと待つ。
            NyLPC_cThread_yield();
            //TXの消去待ち
            if(waitForTxRemove(inst,sq,&sw)){
                //再ロック
                lockResource(inst);
                //タイムアウトするか、UIP_CLOSED、もしくはTIME_WAITに遷移するのを待つ。(遷移はRxprocで自動的に実行。)
                do{
                    switch(inst->tcpstateflags)
                    {
                    case UIP_TIME_WAIT:
                    	inst->tcpstateflags=UIP_CLOSED;
                    case UIP_CLOSED:
                        NyLPC_Assert(inst->txbuf.rp==inst->txbuf.wp);
                        //成功。
                        goto ReturnWithUnlock;
                    case UIP_FIN_WAIT_1:
                    case UIP_FIN_WAIT_2:
                    case UIP_CLOSING:
                        //一時的なアンロック
                        unlockResource(inst);
                        NyLPC_cThread_yield();
                        lockResource(inst);
                    default:
                        break;
                    }
                }while(!NyLPC_cStopwatch_isExpired(&sw));
                unlockResource(inst);
            }
        }
        break;
    case UIP_CLOSE_WAIT:
        //LAST_ACKへ遷移
    	inst->tcpstateflags=UIP_LAST_ACK;
        //送信のために一旦解除
        unlockResource(inst);
        if(sendWithRetransmit(inst,TCP_FIN|TCP_ACK,NULL,0,&sw,&sq)==0){
            //ちょっと待つ。
            NyLPC_cThread_yield();
            //TXの消去待ち
            if(waitForTxRemove(inst,sq,&sw)){
                //再ロック
                lockResource(inst);
                //TX消去後にCLOSEDに遷移していればOK
                if(inst->tcpstateflags==UIP_CLOSED)
                {
                    NyLPC_Assert(inst->txbuf.rp==inst->txbuf.wp);
                    goto ReturnWithUnlock;
                }
                unlockResource(inst);
            }
        }
        //エラー。RSTで切断。
        break;
    default:
        unlockResource(inst);
        NyLPC_Warning();
        break;
    }
//  if(i_inst->_smutex._lock_count>0){
//      NyLPC_Warning();
//  }
    //このパスに到達するのは、FIN送信/ACKに成功したにも拘らず、規定時間内にCLOSEDに遷移しなかった場合。
    //コネクションを強制遷移して、RST
    lockResource(inst);
    f=inst->tcpstateflags;
    if(f!=UIP_CLOSED){
        //もし、強制CLOSE遷移であれば、RSTも送信。
    	inst->tcpstateflags=UIP_CLOSED;
        unlockResource(inst);
        sendRst(inst);
    }else{
        unlockResource(inst);
    }
    NyLPC_cStopwatch_finalize(&sw);
    return;
ReturnWithUnlock:
    unlockResource(inst);
    NyLPC_cStopwatch_finalize(&sw);
    return;
}

/**
 * uipサービスタスクが実行する関数です。
 * 定期的に実行する関数。最低でも1s単位で実行してください。
 */
void NyLPC_cMiMicIpTcpSocket_periodic(
	NyLPC_TcMiMicIpTcpSocket_t* i_inst)
{
    int i;
    struct NyLPC_TcTcpSocket_TxQItem* q=i_inst->txbuf.txq;
    NyLPC_TcStopwatch_t sw;
    NyLPC_TUInt32 now;
    int rp;
    NyLPC_cStopwatch_initialize(&sw);
    now=NyLPC_cStopwatch_now();
    //MUTEX LOCK
    lockResource(i_inst);
    if(i_inst->tcpstateflags==UIP_CLOSED)
    {
        //CLOSEDなら、バッファ開放。
        resetTxQWithUnlock(i_inst);
    }else if(i_inst->txbuf.rp==i_inst->txbuf.wp){
        //再送信パケットがなければ何もしないよ。
        unlockResource(i_inst);
    }else if(i_inst->uip_connr.peer_win==0){
        //peer_winが0の場合は何もしない。
        unlockResource(i_inst);
    }else{
        //再送信処理
        rp=i_inst->txbuf.rp;
        NyLPC_cStopwatch_set(&sw,q[rp].tick_of_sent);
        if(NyLPC_cStopwatch_elapseInMsec(&sw)>q[rp].rto32){
            //最古のパケットの送信時間をチェックして、タイムアウトが発生したら、再送時間と送信時刻をセット
            //最古パケットRTOを2倍。
            q[rp].rto32*=2;
            for(i=rp;i!=i_inst->txbuf.wp;i=(i+1)%NyLPC_TcTcpSocket_NUMBER_OF_TXQ){
                q[i].tick_of_sent=now;
            }
            if(q[rp].rto32>UIP_IP_RTO_MAX_RTO){
                //最古のRTOが64秒を超えたら、CLOSED
                i_inst->tcpstateflags =UIP_CLOSED;
                resetTxQWithUnlock(i_inst);
                sendRst(i_inst);
            }else{
                //規定時間内なら、再送処理
                for(i=rp;i!=i_inst->txbuf.wp;i=(i+1)%NyLPC_TcTcpSocket_NUMBER_OF_TXQ){
//                  NyLPC_cUipService_sendIPv4Tx(NyLPC_cIPv4Payload_getBuf(&(q[i].data)));
                    NyLPC_cMiMicIpNetIf_sendIPv4Tx(q[i].packet);
                }
                unlockResource(i_inst);
            }
        }else{
            unlockResource(i_inst);
        }
    }
    NyLPC_cStopwatch_finalize(&sw);
    return;
}
/**
 * uipサービスタスクが実行する関数です。
 * サービスの開始を通知します。
 */
void NyLPC_cMiMicIpTcpSocket_startService(NyLPC_TcMiMicIpTcpSocket_t* i_inst,const NyLPC_TcIPv4Config_t* i_config)
{
    NyLPC_Assert(i_inst->tcpstateflags==UIP_CLOSED);//閉じてなければおかしい。
    i_inst->uip_connr.lipaddr=&(i_config->ip_addr);
    i_inst->uip_connr.default_mss=i_config->default_mss;
    //NyLPC_cTcpSocket_setSynPayload関数でも実行するけど、IFのリセット時なのでここでもやる。
    NyLPC_cFifoBuffer_clear(&(i_inst->rxbuf));
    return;
}
/**
 * uipサービスタスクが実行する関数です。
 * サービスの停止を通知します。
 */
void NyLPC_cMiMicIpTcpSocket_stopService(NyLPC_TcMiMicIpTcpSocket_t* i_inst)
{
    lockResource(i_inst);
    if(i_inst->tcpstateflags==UIP_CLOSED)
    {
        unlockResource(i_inst);
    }else{
        i_inst->tcpstateflags=UIP_CLOSED;
        resetTxQWithUnlock(i_inst);
        sendRst(i_inst);
    }
    return;
}


void* NyLPC_cMiMicIpTcpSocket_parseRx(
	NyLPC_TcMiMicIpTcpSocket_t* i_inst,
    const NyLPC_TcIPv4Payload_t* i_ipp)
{
    int i,s;
    NyLPC_TUInt16 tmp16;
    NyLPC_TUInt16 data_size;
    NyLPC_TUInt8 in_tcpflag=i_ipp->payload.tcp->flags;
    const void* tcp_data_offset;
    NyLPC_TBool is_new_packet;
    int num_of_noack;
    void* dlist[NyLPC_TcTcpSocket_NUMBER_OF_TXQ];
    void* ret;

    //パラメータの計算

    tmp16=NyLPC_TTcpHeader_getHeaderLength(i_ipp->payload.tcp);
    //TCPペイロードの長さは、IPパケットの長さ-(IPヘッダ+TCPヘッダ)
    data_size=NyLPC_TIPv4Header_getPacketLength(i_ipp->header)-NyLPC_TIPv4Header_getHeaderLength(i_ipp->header)-tmp16;
    //TCPデータオフセット
    tcp_data_offset=i_ipp->payload.rawbuf+tmp16;

    //インスタンスをロックする。
    lockResource(i_inst);

    //RSTのチェック。RST受信時は、状態にかかわらず、CLOSEDステータスに移行する。
    if (in_tcpflag & TCP_RST)
    {
        i_inst->tcpstateflags =UIP_CLOSED;
        goto DROP;
    }


    is_new_packet=NyLPC_ntohl(i_ipp->payload.tcp->seqno32)==i_inst->uip_connr.rcv_nxt32;


    //OPTIONの反映

    //MSSの取得
    if(NyLPC_TTcpHeader_getTcpMmsOpt(i_ipp->payload.tcp,&tmp16)){
        //取得で着たら更新
        i_inst->uip_connr.peer_mss=tmp16;
    }
    //受信パケットを元に、未ACKパケットの数を計算
    num_of_noack=getNumOfSending(i_inst,i_ipp->payload.tcp->ackno32);//i_inst->txbuf.num_of_txq;

    //ステータス毎のACK応答
    switch(i_inst->tcpstateflags)
    {
    case UIP_SYN_RCVD:
        //ACKを受信したら、ESTABLISHEDへ。
        //すべてのパケットをACKしたかで判定。()
        if(num_of_noack==0){
            i_inst->tcpstateflags=UIP_ESTABLISHED;
        }else{
            //それ以外のパケットはドロップする。
            break;//goto DROP;
        }
        //新しいパケットがなければ、無応答
        if(!is_new_packet){
            break;//goto DROP;
        }
        //引き続き、ESTABLISHEDの処理へ。
    case UIP_ESTABLISHED:
        if(data_size>0){
            if(is_new_packet){
                if(addRecvData(i_inst,tcp_data_offset,data_size)){
                    //通常のACK返却
                    i_inst->uip_connr.rcv_nxt32+=data_size;
                }else{
                    //失敗したときは必要に応じて単純ACK
                }
            }
        }
        //どちらにしろ、ACK送信
        if(is_new_packet && (in_tcpflag & TCP_FIN)){
            //FINがあるときは、ステータスをCLOSE_WAITへセットして、ACKを返す。
            i_inst->tcpstateflags = UIP_CLOSE_WAIT;
            i_inst->uip_connr.rcv_nxt32++;
        }
        break;
    case UIP_CLOSE_WAIT:
        //必要に応じたACK応答
        break;
    case UIP_LAST_ACK:
        //ACK(by FIN)が得られたなら、CLOSEDへ。
        if(num_of_noack==0){
            i_inst->tcpstateflags=UIP_CLOSED;
        }
        //必要に応じたACK応答
        break;
    case UIP_FIN_WAIT_1:
        //FIN受信->CLOSINGへ
        if(is_new_packet){
            i_inst->uip_connr.rcv_nxt32+=data_size;
            if(in_tcpflag & TCP_FIN){
                i_inst->uip_connr.rcv_nxt32++;
                if(num_of_noack==0){
                    //FINとACKを受信
                    i_inst->tcpstateflags=UIP_TIME_WAIT;
                }else{
                    //FINのみ
                    i_inst->tcpstateflags=UIP_CLOSING;
                }
            }
        }else if(num_of_noack==0){
            //ACKのみ
            i_inst->tcpstateflags=UIP_FIN_WAIT_2;
        }
        //必要に応じたACK応答
        break;
    case UIP_FIN_WAIT_2:
        //FIN受信->TIME_WAITへ(pureACK)
        if(is_new_packet && (in_tcpflag & TCP_FIN)){
            i_inst->uip_connr.rcv_nxt32++;
            i_inst->tcpstateflags=UIP_TIME_WAIT;
        }
        break;
    case UIP_CLOSING:
        //ACK受信したら、TIME_WAITへ
        if(num_of_noack==0){
            i_inst->tcpstateflags=UIP_TIME_WAIT;
        }
        break;
    case UIP_CLOSED:
        //何もできない。何もしない。
        break;
    case UIP_TIME_WAIT:
        //最終ACKを送り続ける。
        break;
    case UIP_SYN_SENT:
        //connect関数実行中しか起動しないステータス
        if(num_of_noack==0){
            i_inst->tcpstateflags=UIP_ESTABLISHED;
            i_inst->uip_connr.rcv_nxt32=NyLPC_ntohl(i_ipp->payload.tcp->seqno32)+1;
        }else{
            //それ以外のパケットはドロップする。
            break;//goto DROP;
        }
        //ACKを送る。
        break;
    default:
        goto DROP;
    }
    //ウインドウサイズを更新
    i_inst->uip_connr.peer_win=NyLPC_ntohs(i_ipp->payload.tcp->wnd16);

    //送信キューから、Peerが受信したデータを削除する。
    if(in_tcpflag & TCP_ACK){
        //再送パケットキューから送信済みのデータを回収(後で開放)
        NyLPC_Trace();
        s=updateTxQByIndex(i_inst,i_ipp->payload.tcp->ackno32,dlist);
        NyLPC_Trace();
    }else{
        s=0;
    }
    //新しいパケットがきた場合は、再送キューのACKを更新する。
    if(is_new_packet){
        //再送キューのACKを更新
        updateTxAck(i_inst,NyLPC_htonl(i_inst->uip_connr.rcv_nxt32));
    }

    //送信キューのない
    if(((in_tcpflag&(TCP_FIN|TCP_SYN))!=0x00) ||
        ((!is_new_packet) && (data_size>0)))
    {
        //ソケットからPureACKを生成 as setPacket(i_inst,i_ipp,TCP_ACK,NULL,0);
        ret=NyLPC_cMiMicIpNetIf_allocSysTxBuf();
        setTxPacket(i_inst,ret,TCP_ACK,NULL,0);
    }else{
        ret=NULL;
    }
    unlockResource(i_inst);
    //取り外したTXメモリの開放
    for(i=0;i<s;i++){
        //取り外したTXメモリを開放
        NyLPC_cMiMicIpNetIf_releaseTxBuf(dlist[i]);
    }
NyLPC_Trace();
    return ret;
DROP:
    //ACKしたパケットを送信キューから削除
    unlockResource(i_inst);
NyLPC_Trace();
    return NULL;
}


/**
 * 入力されたパケットからRSTパケットを生成して返す。
 */
void* NyLPC_cMiMicIpTcpSocket_allocTcpReverseRstAck(
    const NyLPC_TcIPv4Payload_t* i_src)
{
    struct NyLPC_TIPv4Header*   iph;
    struct NyLPC_TTcpHeader*    tcph;
    NyLPC_TUInt8 iph_word=0x05;
    NyLPC_TUInt8 tcph_word=(UIP_TCPH_LEN) / 4;
    void* txb=NyLPC_cMiMicIpNetIf_allocSysTxBuf();
    //IPヘッダの更新
    iph=(struct NyLPC_TIPv4Header*)txb;
    iph->vhl=0x40|(0x0f&iph_word);
    iph->destipaddr=i_src->header->srcipaddr;
    iph->srcipaddr =i_src->header->destipaddr;
    NyLPC_TIPv4Header_writeTxIpHeader(iph,UIP_PROTO_TCP);

    //TCPヘッダの更新
    tcph=(struct NyLPC_TTcpHeader*)(((NyLPC_TUInt8*)txb)+NyLPC_TIPv4Header_getHeaderLength(iph));

    tcph->tcpoffset=(tcph_word<<4);

    tcph->flags    = TCP_RST | TCP_ACK;
    //sorce & destination port
    tcph->srcport  = i_src->payload.tcp->destport;
    tcph->destport = i_src->payload.tcp->srcport;
    //ACK number
    tcph->ackno32  = NyLPC_htonl(NyLPC_ntohl(i_src->payload.tcp->seqno32)+1);
    //Seq Number
    tcph->seqno32  = i_src->payload.tcp->ackno32;
    //uip_func_tcp_send_noconn(BUF);
    tcph->urgp[0]  = tcph->urgp[1] = 0;
    tcph->tcpchksum= 0;


    //最終的なパケットサイズと必要ならペイロードを書き込み
    iph->len16=NyLPC_htons((iph_word+tcph_word)*4);
    //WND設定
    tcph->wnd16=0;
    //Checksumの生成
    tcph->tcpchksum=~(NyLPC_TIPv4Header_makeTcpChecksum(iph));
    iph->ipchksum = ~(NyLPC_TIPv4Header_makeIpChecksum(iph));
    return txb;
}




