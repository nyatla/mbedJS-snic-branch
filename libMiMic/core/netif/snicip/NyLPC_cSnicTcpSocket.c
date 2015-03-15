#include "NyLPC_stdlib.h"
#include "NyLPC_cSnicTcpSocket.h"
#include "NyLPC_cSnicApi.h"
#include "NyLPC_cSnicNetIf_protected.h"
#include "NyLPC_cSnicNetIf.h"


//#define lockResource(i_inst) NyLPC_cMutex_lock(&((i_inst)->_smutex))
//#define unlockResource(i_inst) NyLPC_cMutex_unlock(&((i_inst)->_smutex))
#define lockResource(i_inst) (NyLPC_cMutex_lock((NyLPC_cSnicApi_getSocketMutex(NyLPC_cSnicNetIf_getSnicApi()))))
#define unlockResource(i_inst) (NyLPC_cMutex_unlock((NyLPC_cSnicApi_getSocketMutex(NyLPC_cSnicNetIf_getSnicApi()))))

#define NyLPC_TcSnicTcpSocket_TCP_RX_SIZE NyLPC_TcSnicNetIf_TCP_RX_BUFFER_SIZE

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Mainclass::private
//
////////////////////////////////////////////////////////////////////////////////////////////////////



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

#define FLAGS_SOCKET_ENABLE 0x01
#define FLAGS_ACCEPTED 0x02

NyLPC_TBool NyLPC_cSnicTcpSocket_initialize(NyLPC_TcSnicTcpSocket_t* i_inst,void* i_rxbuf,NyLPC_TUInt16 i_rxsize,NyLPC_TSocketType i_socktype)
{
	i_inst->_flags=0x00;
	i_inst->_sock_type=i_socktype;
	i_inst->super._interface=&_interface;
	NyLPC_cFifoBuffer_initialize(&i_inst->rxbuf,i_rxbuf,i_rxsize);
	return NyLPC_TBool_TRUE;
}

NyLPC_TBool NyLPC_cSnicTcpSocket_bindServerSocket(NyLPC_TcSnicTcpSocket_t* i_inst,NyLPC_TUInt8 i_socket,const struct NyLPC_TIPv4Addr* i_peer_addr,NyLPC_TInt16 i_peer_port)
{
	//BINDもCONNECTもしてないこと。
	NyLPC_Assert((i_inst->_flags&FLAGS_SOCKET_ENABLE)==0);
	i_inst->_socket=i_socket;
	i_inst->_flags|=(FLAGS_SOCKET_ENABLE|FLAGS_ACCEPTED);
	i_inst->peer_addr=*i_peer_addr;
	i_inst->peer_port=i_peer_port;
	return NyLPC_TBool_TRUE;
}
NyLPC_TBool NyLPC_cSnicTcpSocket_isSocketEqual(const NyLPC_TcSnicTcpSocket_t* i_inst,NyLPC_TUInt8 i_sock)
{
	return (i_inst->_socket==i_sock) && ((i_inst->_flags & FLAGS_SOCKET_ENABLE)==FLAGS_SOCKET_ENABLE);
}
/**
 * Public function
 */
static void finalize(NyLPC_TiTcpSocket_t* i_inst)
{
	NyLPC_TcSnicApi_t* api=NyLPC_cSnicNetIf_getSnicApi();
	NyLPC_TcSnicTcpSocket_t* inst=(NyLPC_TcSnicTcpSocket_t*)i_inst;
	if((inst->_flags&FLAGS_SOCKET_ENABLE)!=0){
		NyLPC_cSnicApi_closeSocket(api,inst->_socket);
	}
	//ソケットテーブルから取りはずす。
	NyLPC_cSnicNetIf_removeTcpSocket(inst);
    return;
}

static NyLPC_TBool connect(NyLPC_TiTcpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_peer_port,NyLPC_TUInt32 i_wait_in_msec)
{
	NyLPC_TUInt8 sock;
	NyLPC_TcSnicApi_t* api=NyLPC_cSnicNetIf_getSnicApi();
	NyLPC_TcSnicTcpSocket_t* inst=(NyLPC_TcSnicTcpSocket_t*)i_inst;
	/**
	 * 事前チェック
	 */
	lockResource(inst);
	//ENABLEDソケットはConnectできないよ
	if((inst->_flags&FLAGS_SOCKET_ENABLE)!=0){
		unlockResource(inst);
		return NyLPC_TBool_TRUE;
	}
	unlockResource(inst);

	if(!NyLPC_cSnicApi_createTcpSocket(api,0,NyLPC_cSnicNetIf_getLocalAddr(),i_peer_port,&sock))
	{
		return NyLPC_TBool_FALSE;
	}
	if(!NyLPC_cSnicApi_connectToServer(api,sock,i_addr,i_peer_port,(NyLPC_TUInt16)i_wait_in_msec,NyLPC_TcSnicTcpSocket_TCP_RX_SIZE,NULL)){
		NyLPC_cSnicApi_sysCloseSocket(sock);
		return NyLPC_TBool_FALSE;
	}
	lockResource(inst);
	//ソケットの保存。!ENABLEの場合のみ
	if((inst->_flags&FLAGS_SOCKET_ENABLE)!=0){
		unlockResource(inst);
		NyLPC_cSnicApi_sysCloseSocket(sock);
		return NyLPC_TBool_FALSE;
	}else{
		inst->_socket=sock;
	}
	unlockResource(inst);
	return NyLPC_TBool_FALSE;
}

static NyLPC_TBool accept(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt32 i_wait_in_msec)
{
	NyLPC_TcSnicTcpSocket_t* inst=(NyLPC_TcSnicTcpSocket_t*)i_inst;
	lockResource(inst);
	//soketto
	if((inst->_flags&FLAGS_SOCKET_ENABLE)!=0){
		if((inst->_flags&FLAGS_ACCEPTED)==0){
			inst->_flags|=FLAGS_ACCEPTED;
			unlockResource(inst);
			return NyLPC_TBool_TRUE;
		}
	}
	unlockResource(inst);
	return NyLPC_TBool_FALSE;
}


static const struct NyLPC_TIPv4Addr* getPeerAddr(const NyLPC_TiTcpSocket_t* i_inst)
{
	NyLPC_TcSnicTcpSocket_t* inst=(NyLPC_TcSnicTcpSocket_t*)i_inst;
	return &inst->peer_addr;
}
static NyLPC_TUInt16 getPeerPort(const NyLPC_TiTcpSocket_t* i_inst)
{
	NyLPC_TcSnicTcpSocket_t* inst=(NyLPC_TcSnicTcpSocket_t*)i_inst;
	return inst->peer_port;
}


/**
 * この関数は、ソケットの受信バッファの読み取り位置と、読み出せるデータサイズを返却します。
 * 関数はポインターを返却するだけで、バッファの読み取り位置をシークしません。
 * シークするにはNyLPC_cTcpSocket_pseekを使います。
 */
static NyLPC_TInt32 precv(NyLPC_TiTcpSocket_t* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_wait_msec)
{
	NyLPC_TcSnicTcpSocket_t* inst=(NyLPC_TcSnicTcpSocket_t*)i_inst;
	NyLPC_TInt32 len;

	lockResource(inst);
	len=NyLPC_cFifoBuffer_getLength(&inst->rxbuf);
	*o_buf_ptr=NyLPC_cFifoBuffer_getPtr(&inst->rxbuf);
	unlockResource(inst);
	return len;
}
/**
 * 受信バッファをシークします。
 * シーク後に、遅延ACKを送出します。
 */
static void pseek(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt16 i_seek)
{
	NyLPC_TcSnicTcpSocket_t* inst=(NyLPC_TcSnicTcpSocket_t*)i_inst;
	lockResource(inst);
	NyLPC_cFifoBuffer_pop(&inst->rxbuf,i_seek);
	unlockResource(inst);
}

/**
 * See header file.
 */
static void* allocSendBuf(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_buf_size,NyLPC_TUInt32 i_wait_in_msec)
{
	return NyLPC_cSnicNetIf_allocBuf(i_hint,o_buf_size);
}
/**
 * See Header file.
 */
static void releaseSendBuf(NyLPC_TiTcpSocket_t* i_inst,void* i_buf_ptr)
{
	return NyLPC_cSnicNetIf_releaseBuf(i_buf_ptr);
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
	NyLPC_TUInt16 p=0;
	struct NyLPC_TSnicApi_SendFromSocketResult result;
	NyLPC_TcSnicTcpSocket_t* inst=(NyLPC_TcSnicTcpSocket_t*)i_inst;
	NyLPC_TcSnicApi_t* api=NyLPC_cSnicNetIf_getSnicApi();
	//非同期に_flagsが無効されることは無いの前提
	if((inst->_flags&(FLAGS_SOCKET_ENABLE|FLAGS_ACCEPTED))==(FLAGS_SOCKET_ENABLE|FLAGS_ACCEPTED)){
		while(p<i_len){
			//
			if(!NyLPC_cSnicApi_sendFromSocket(api,inst->_socket,0,i_buf_ptr,i_len,&result)){
				return NyLPC_TBool_FALSE;
			}
			p+=result.number_of_byte_sent;
		}
	}
    return NyLPC_TBool_TRUE;
}

/**
 * See header file.
 */
static NyLPC_TInt32 send(NyLPC_TiTcpSocket_t* i_inst,const void* i_buf_ptr,NyLPC_TInt32 i_len,NyLPC_TUInt32 i_wait_in_msec)
{
	NyLPC_TBool r;
	NyLPC_TUInt16 l;
	void* buf=allocSendBuf(i_inst,i_len,&l,i_wait_in_msec);
	if(buf==NULL){
		return -1;
	}
	if(i_len<l){
		l=i_len;
	}

	r=psend(i_inst,buf,l,i_wait_in_msec);
	releaseSendBuf(i_inst,buf);
	if(r){
		return l;
	}else{
		return -1;
	}
}


static void close(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt32 i_wait_in_msec)
{
	NyLPC_TBool go_close=NyLPC_TBool_FALSE;
	NyLPC_TcSnicTcpSocket_t* inst=(NyLPC_TcSnicTcpSocket_t*)i_inst;
	NyLPC_TcSnicApi_t* api=NyLPC_cSnicNetIf_getSnicApi();
	lockResource(inst);
	//soketto
	if((inst->_flags&FLAGS_SOCKET_ENABLE)!=0){
		inst->_flags=0;
		go_close=NyLPC_TBool_TRUE;
	}
	unlockResource(inst);
	if(go_close){
		NyLPC_cSnicApi_closeSocket(api,inst->_socket);
	}
	return;
}

#define getSocket(i) NULL;

/**
 * @return
 * true=パケットを処理した。
 * false=パケットを処理できなかった。
 */
NyLPC_TBool NyLPC_cSnicTcpSocket_onConnectionRecvInd(
		NyLPC_TcSnicTcpSocket_t* i_inst,struct NyLPC_TiSnicDevice_Interface* i_dev)
{
	NyLPC_TUInt8 buf[4];
	void* rxbuf;
	NyLPC_TUInt16 buflen;
	NyLPC_TUInt16 datalen;
	//
	lockResource(inst);
	if((i_inst->_flags&(FLAGS_SOCKET_ENABLE|FLAGS_ACCEPTED))==(FLAGS_SOCKET_ENABLE|FLAGS_ACCEPTED)){
		unlockResource(i_inst);
		goto DROP;
	}
	//サイズ読出し
	if(!NyLPC_cSnicNetIf_readPayload(buf,2)){
		//受け取ったけど何バイトシークしたらいいかわからない。
		unlockResource(i_inst);
		goto DROP;
	}
	datalen=(((NyLPC_TUInt16)buf[0])<<8)|buf[1];
	//データキューの空きチェック
	buflen=NyLPC_cFifoBuffer_getSpace(&i_inst->rxbuf);
	if(buflen<datalen){
		//タスク切り替えてみてあかなきゃ諦める。
		NyLPC_cThread_sleep(1);
		buflen=NyLPC_cFifoBuffer_getSpace(&i_inst->rxbuf);
		if(buflen<datalen){
			//受け取らなかったことにする（できるの？）
			unlockResource(i_inst);
			NyLPC_cSnicNetIf_seekPayload(datalen);
			return NyLPC_TBool_TRUE;
		}
	}
	rxbuf=NyLPC_cFifoBuffer_prePush(&i_inst->rxbuf,datalen);
	//ロック中で実行するけどReadはRxスレッドからしか実行しないからデットロックしないよね？
	if(!NyLPC_cSnicNetIf_readPayload(rxbuf,datalen)){
		//受け取ったけど何バイトシークしたらいいかわからない。
		unlockResource(i_inst);
		goto DROP;
	}
	unlockResource(i_inst);
	//confirm更新
	NyLPC_cSnicApi_sendIndicateConfirm(buf[0]|0x08,buf[1]);
	return NyLPC_TBool_TRUE;
DROP:
	return NyLPC_TBool_FALSE;
}

