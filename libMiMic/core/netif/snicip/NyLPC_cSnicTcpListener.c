#include "NyLPC_cSnicTcpListener.h"
#include "NyLPC_cSnicApi.h"
#include "NyLPC_cSnicNetIf_protected.h"

#define LOCK_RESOURCE(i_inst) (NyLPC_cMutex_lock((NyLPC_cSnicApi_getSocketMutex(NyLPC_cSnicNetIf_getSnicApi()))))
#define UNLOCK_RESOURCE(i_inst) (NyLPC_cMutex_unlock((NyLPC_cSnicApi_getSocketMutex(NyLPC_cSnicNetIf_getSnicApi()))))

#define NyLPC_TcSnicTcpListener_TCP_RX_SIZE	512

static NyLPC_TBool listen(NyLPC_TiTcpListener_t* i_inst,NyLPC_TiTcpSocket_t* i_sock,NyLPC_TUInt32 i_wait_msec);
static void finaize(NyLPC_TiTcpListener_t* i_inst);

const static struct NyLPC_TiTcpListener_Interface interface=
{
	listen,
	finaize
};

#define CLIENT_FLAGS_USED 0x01
/**
 * uipサービスが稼働中にのみ機能します。
 */
NyLPC_TBool NyLPC_cSnicTcpListener_initialize(NyLPC_TcSnicTcpListener_t* i_inst,NyLPC_TUInt16 i_port)
{
	//Snicソケット作る。
	NyLPC_TcSnicApi_t* api=NyLPC_cSnicNetIf_getSnicApi();
	i_inst->_number_of_client=0;
	//ソケット生成
	if(!NyLPC_cSnicApi_createTcpSocket(api,0,NULL,i_port,&i_inst->_socket))
	{
		return NyLPC_TBool_FALSE;
	}
	if(!NyLPC_cSnicApi_createTcpConnectionServer(api,i_inst->_socket,NyLPC_TcSnicTcpListener_TCP_RX_SIZE,NyLPC_TcSnicTcpListener_MAX_CONNECTION,NULL)){
		return NyLPC_TBool_FALSE;
	}
	return NyLPC_TBool_TRUE;
}




static void finaize(NyLPC_TiTcpListener_t* i_inst)
{
	NyLPC_TUInt16 i;
	NyLPC_TcSnicTcpListener_t* inst=(NyLPC_TcSnicTcpListener_t*)i_inst;
	NyLPC_TcSnicApi_t* api=NyLPC_cSnicNetIf_getSnicApi();
	//LISTENERソケット閉じる
	NyLPC_cSnicApi_closeSocket(api,inst->_socket);
	//未回収のソケットを閉じる
	for(i=0;i<inst->_number_of_client;i++){
		NyLPC_cSnicApi_closeSocket(api,inst->_client_socket[i].socket);
	}
	NyLPC_cSnicNetIf_removeTcpListener(inst);

	return;
}


static NyLPC_TBool listen(NyLPC_TiTcpListener_t* i_inst,NyLPC_TiTcpSocket_t* i_sock,NyLPC_TUInt32 i_wait_msec)
{
	struct NyLPC_TcSnicTcpListenerQItem client;
    NyLPC_TcStopwatch_t sw;
	NyLPC_TcSnicTcpListener_t* inst=(NyLPC_TcSnicTcpListener_t*)i_inst;

    //ストップウォッチを起動
    NyLPC_cStopwatch_initialize(&sw);
    NyLPC_cStopwatch_setNow(&sw);

    while(NyLPC_cStopwatch_elapseInMsec(&sw)<i_wait_msec){
		LOCK_RESOURCE(inst);
		if(inst->_number_of_client>0){
			client=inst->_client_socket[0];
			//キューを詰める
			memmove(&inst->_client_socket[0],&inst->_client_socket[1],sizeof(struct NyLPC_TcSnicTcpListenerQItem)*(NyLPC_TcSnicTcpListener_NUMBER_OF_Q-1));
			inst->_number_of_client--;
			UNLOCK_RESOURCE(inst);
			//ソケットをバインド
			NyLPC_cSnicTcpSocket_bindServerSocket((NyLPC_TcSnicTcpSocket_t*)i_sock,client.socket,&client.ip,client.port);
		    return NyLPC_TBool_TRUE;
		}
		UNLOCK_RESOURCE(inst);
		NyLPC_cThread_yield();
    }
    //タイムアウト
    NyLPC_cStopwatch_finalize(&sw);
    return NyLPC_TBool_FALSE;
}

NyLPC_TBool NyLPC_cSnicTcpListener_isSocketEqual(const NyLPC_TcSnicTcpListener_t* i_inst,NyLPC_TUInt8 i_sock)
{
	return i_inst->_socket==i_sock;

}

NyLPC_TBool NyLPC_cSnicTcpListener_onClientSocketInd(NyLPC_TcSnicTcpListener_t* i_tcp_listener,struct NyLPC_TiSnicDevice_Interface* i_dev)
{
	NyLPC_TUInt8 buf[7];//CMD,SEQ,RES
	struct NyLPC_TcSnicTcpListenerQItem* client;
	if(!NyLPC_cSnicNetIf_readPayload(buf,7)){
		goto DROP;
	}
	LOCK_RESOURCE(i_tcp_listener);
	if(i_tcp_listener->_number_of_client<NyLPC_TcSnicTcpListener_NUMBER_OF_Q){
		client=&i_tcp_listener->_client_socket[i_tcp_listener->_number_of_client];
		client->socket=buf[0];
		memcpy(&client->ip.v,&buf[1],4);
		client->port=(((NyLPC_TUInt16)buf[5])<<8)|buf[6];
		i_tcp_listener->_number_of_client++;
		//confirm更新
		UNLOCK_RESOURCE(i_tcp_listener);
		NyLPC_cSnicApi_sendIndicateConfirm(0x80|buf[0],buf[1]);
	}else{
		UNLOCK_RESOURCE(i_tcp_listener);
		//受け取ったことにして
		NyLPC_cSnicApi_sendIndicateConfirm(0x80|buf[0],buf[1]);
		//メッセージをトラップされないようにしてClose
		NyLPC_cSnicApi_sysCloseSocket(buf[0]);
	}
	return NyLPC_TBool_TRUE;
DROP:
	return NyLPC_TBool_FALSE;
}

