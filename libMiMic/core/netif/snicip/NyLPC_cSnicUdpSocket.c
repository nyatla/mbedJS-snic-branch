#include "NyLPC_cSnicUdpSocket.h"
#include "NyLPC_cSnicApi.h"
#include "NyLPC_cSnicNetIf_protected.h"

#define NyLPC_TcSnicUdpSocket_UDP_BUFSIZE 1400

#define lockResource(i_inst) (NyLPC_cMutex_lock((NyLPC_cSnicApi_getSocketMutex(NyLPC_cSnicNetIf_getSnicApi()))))
#define unlockResource(i_inst) (NyLPC_cMutex_unlock((NyLPC_cSnicApi_getSocketMutex(NyLPC_cSnicNetIf_getSnicApi()))))


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
static const struct NyLPC_TIPv4Addr* getSockIP(const NyLPC_TiUdpSocket_t* i_inst);
static void finalize(NyLPC_TiUdpSocket_t* i_inst);

static const struct NyLPC_TiUdpSocket_Interface interface=
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


NyLPC_TBool NyLPC_cSnicUdpSocket_isSocketEqual(const NyLPC_TcSnicUdpSocket_t* i_inst,NyLPC_TUInt8 i_sock)
{
	return (i_inst->_socket==i_sock);
}

/*
 *	Initializer/Finalizer
 */


NyLPC_TBool NyLPC_SnicUdpSocket_initialize(NyLPC_TcSnicUdpSocket_t* i_inst,NyLPC_TUInt16 i_port,void* i_rbuf,NyLPC_TUInt16 i_rbuf_len)
{
	NyLPC_TUInt8 sock;
	NyLPC_TcSnicApi_t* api=NyLPC_cSnicNetIf_getSnicApi();
	i_inst->_super._interface=&interface;
	i_inst->_super._tag=NULL;
    i_inst->as_handler.rx=NULL;
    i_inst->as_handler.periodic=NULL;
    i_inst->lport=i_port;
    NyLPC_cFifoBuffer_initialize(&(i_inst->rxbuf),i_rbuf,i_rbuf_len);
    if(NyLPC_cSnicApi_createUdpSocket(api,0,NyLPC_cSnicNetIf_getLocalAddr(),i_port,&sock)){
    	if(NyLPC_cSnicApi_startUdpReceiveOnSocket(api,sock,NyLPC_TcSnicUdpSocket_UDP_BUFSIZE,NULL)){
			return NyLPC_TBool_TRUE;
    	}
    	NyLPC_cSnicApi_closeSocket(api,sock);
    }
    i_inst->_socket=sock;
    return NyLPC_TBool_FALSE;
}





/**
 * @return
 * true=パケットを処理した。
 * false=パケットを処理できなかった。
 */
NyLPC_TBool NyLPC_cSnicUdpSocket_onConnectionRecvInd(
	NyLPC_TcSnicUdpSocket_t* i_inst,struct NyLPC_TiSnicDevice_Interface* i_dev)
{
	NyLPC_TUInt8 buf[8];
	void* rxbuf;
	NyLPC_TUInt16 datalen;
    struct NyLPC_TIPv4RxInfo dheader;
    //
	lockResource(i_inst);
	//サイズ読出し
	if(!NyLPC_cSnicNetIf_readPayload(&buf,8)){
		//受け取ったけど何バイトシークしたらいいかわからない。
		unlockResource(i_inst);
		goto DROP;
	}
	datalen=(((NyLPC_TUInt16)buf[6])<<8)|buf[7];
//	if(EVENT NOT NULL){
//		pushEvent(onRX,);
//	}




    //受信データサイズを確認
    if(NyLPC_cFifoBuffer_getSpace(&(i_inst->rxbuf))<datalen+sizeof(struct NyLPC_TIPv4RxInfo)){
		unlockResource(i_inst);
        goto DROP;
    }
    dheader.size=datalen;
    memcpy(&dheader.peer_ip,&buf[0],4);
    dheader.peer_port=(((NyLPC_TUInt16)buf[4])<<8) |buf[5];
    dheader.ip=*NyLPC_cSnicNetIf_getLocalAddr();
    dheader.port=i_inst->lport;
    //バッファに格納可能なら、格納。
    NyLPC_cFifoBuffer_push(&(i_inst->rxbuf),&dheader,sizeof(struct NyLPC_TIPv4RxInfo));
	rxbuf=NyLPC_cFifoBuffer_prePush(&i_inst->rxbuf,datalen);
	if(!NyLPC_cSnicNetIf_readPayload(rxbuf,datalen)){
		//受け取ったけど何バイトシークしたらいいかわからない。
		unlockResource(i_inst);
		goto DROP;
	}
	//データキューの空きチェック
	unlockResource(i_inst);
	//confirm更新
	NyLPC_cSnicApi_sendIndicateConfirm(buf[0]|0x08,buf[1]);
	return NyLPC_TBool_TRUE;
DROP:
	return NyLPC_TBool_FALSE;
}







static void finalize(NyLPC_TiUdpSocket_t* i_inst)
{
	NyLPC_TcSnicApi_t* api=NyLPC_cSnicNetIf_getSnicApi();
	NyLPC_TcSnicUdpSocket_t* inst=(NyLPC_TcSnicUdpSocket_t*)i_inst;
	NyLPC_cSnicApi_closeSocket(api,inst->_socket);
    NyLPC_cFifoBuffer_finalize(&(i_inst->rxbuf));
    NyLPC_cSnicNetIf_removeUdpSocket(inst);
    return;
}


static void joinMulticast(NyLPC_TiUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr)
{
	NyLPC_TcSnicApi_t* api=NyLPC_cSnicNetIf_getSnicApi();
	NyLPC_TcSnicUdpSocket_t* inst=(NyLPC_TcSnicUdpSocket_t*)i_inst;
//setsockopt;
}
static void setBroadcast(NyLPC_TiUdpSocket_t* i_inst)
{
	NyLPC_TcSnicApi_t* api=NyLPC_cSnicNetIf_getSnicApi();
	NyLPC_TcSnicUdpSocket_t* inst=(NyLPC_TcSnicUdpSocket_t*)i_inst;
//setsockopt;
}



/**
 * see Header file
 */
static NyLPC_TInt32 precv(NyLPC_TiUdpSocket_t* i_inst,const void** o_buf_ptr,const struct NyLPC_TIPv4RxInfo** o_info,NyLPC_TUInt32 i_wait_msec)
{
	NyLPC_TcSnicUdpSocket_t* inst=(NyLPC_TcSnicUdpSocket_t*)i_inst;
    NyLPC_TUInt16 rlen;
    //タイマを生成
    NyLPC_TcStopwatch_t sw;
    NyLPC_cStopwatch_initialize(&sw);
    const char* b;
    const struct NyLPC_TIPv4RxInfo* rh;

    NyLPC_cStopwatch_setNow(&sw);
    while(NyLPC_cStopwatch_elapseInMsec(&sw)<i_wait_msec)
    {
        //MUTEX LOCK
        lockResource(inst);
        rlen=NyLPC_cFifoBuffer_getLength(&(inst->rxbuf));
        if(rlen>0){
            //受信キューにデータがあれば返す。
            b=(char*)NyLPC_cFifoBuffer_getPtr(&(inst->rxbuf));
            rh=(const struct NyLPC_TIPv4RxInfo*)b;
            *o_buf_ptr=b+sizeof(struct NyLPC_TIPv4RxInfo);
            if(o_info!=NULL){
                *o_info=rh;
            }
            unlockResource(inst);
            NyLPC_cStopwatch_finalize(&sw);
            return rh->size;
        }
        //MUTEX UNLOCK
        unlockResource(inst);
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
	NyLPC_TcSnicUdpSocket_t* inst=(NyLPC_TcSnicUdpSocket_t*)i_inst;
    NyLPC_TUInt16 s;
    const struct NyLPC_TIPv4RxInfo* rh;
    //シークサイズを決定
    lockResource(inst);
    s=NyLPC_cFifoBuffer_getLength(&(inst->rxbuf));
    if(s>0){
		rh=(const struct NyLPC_TIPv4RxInfo*)NyLPC_cFifoBuffer_getPtr(&(inst->rxbuf));
		NyLPC_cFifoBuffer_pop(&(inst->rxbuf),rh->size+sizeof(struct NyLPC_TIPv4RxInfo));
    }
    unlockResource(inst);
}

/**
 * See header file.
 */
static void* allocSendBuf(NyLPC_TiUdpSocket_t* i_inst,NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_buf_size,NyLPC_TUInt32 i_wait_in_msec)
{
	return NyLPC_cSnicNetIf_allocBuf(i_hint,o_buf_size);
}
/**
 * See Header file.
 */
static void releaseSendBuf(NyLPC_TiUdpSocket_t* i_inst,void* i_buf_ptr)
{
	return NyLPC_cSnicNetIf_releaseBuf(i_buf_ptr);
}

/**
 * See header file
 */
static NyLPC_TBool psend(NyLPC_TiUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_port,void* i_buf_ptr,int i_len)
{
	NyLPC_TUInt16 p=0;
	NyLPC_TcSnicUdpSocket_t* inst=(NyLPC_TcSnicUdpSocket_t*)i_inst;
	NyLPC_TcSnicApi_t* api=NyLPC_cSnicNetIf_getSnicApi();
	//非同期に_flagsが無効されることは無いの前提
	if(!NyLPC_cSnicApi_sendUdpPacketFromSocket(api,inst->_socket,0,i_addr,i_port,i_buf_ptr,i_len,NULL)){
		return NyLPC_TBool_FALSE;
	}
    return (p==i_len);
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
	NyLPC_TcSnicUdpSocket_t* inst=(NyLPC_TcSnicUdpSocket_t*)i_inst;
	inst->as_handler.rx=i_handler;
}
static void setOnPeriodicHandler(NyLPC_TiUdpSocket_t* i_inst,NyLPC_TiUdpSocket_onPeriodicHandler i_handler)
{
	NyLPC_TcSnicUdpSocket_t* inst=(NyLPC_TcSnicUdpSocket_t*)i_inst;
	inst->as_handler.periodic=i_handler;
}
static const struct NyLPC_TIPv4Addr* getSockIP(const NyLPC_TiUdpSocket_t* i_inst)
{
	return NyLPC_cSnicNetIf_getLocalAddr();
}




