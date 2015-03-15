#include "NyLPC_os.h"
#include "NyLPC_cIdPtrTable.h"
#include "NyLPC_iSnicDevice.h"
#include "NyLPC_Snic_types.h"
#include "NyLPC_cIdPtrTable.h"
#include "NyLPC_cSnicNetIf_protected.h"
#include "io/NyLPC_cSnicSerialDevice.h"
#include "NyLPC_cSnicApi.h"
#include "NyLPC_cSnicWifi.h"
#include "NyLPC_cSnicMM.h"

static int messageTask(void* i_param);
static NyLPC_TBool waitForRead(struct NyLPC_TiSnicDevice_Interface* i_dev,NyLPC_TUInt8* i_buf,NyLPC_TUInt8 i_len);


#define SIZE_OF_STACK	(512+256)
#define SIZE_OF_TXB 64

typedef struct NyLPC_TcSnicNetif NyLPC_TcSnicNetif_t;
struct NyLPC_TcSnicNetif{
	volatile NyLPC_TUInt32 _flags;
	NyLPC_TcThread_t _thread;
	NyLPC_TcMutex_t _mutex;	//書き込み占有/リソース保護mutex(Wifi,Api,Generalも使うから注意！)
	NyLPC_TcIdPtrTable_t _tbl;
	struct NyLPC_TiSnicDevice_Interface* _dev;
	struct{
		NyLPC_TUInt8 txbuf[SIZE_OF_TXB];
		NyLPC_TUInt8 checksum;
	}tx_data;
	NyLPC_TcSnicApi_t _snic_api;
	NyLPC_TcSnicWifi_t _snic_wifi;
	NyLPC_TcSnicGeneral_t _snic_general;
	struct NyLPC_TIPv4Addr _local_ip;
	struct NyLPC_TNetInterfaceInfo _netinfo;
};

#define FLAGS_INIT				0x00000000	//初期値
#define FLAGS_THREAD_ACTIVE_REQ	0x00000001	//THREADループの有効値
#define FLAGS_THREAD_ACTIVE_STA	0x00000002	//THREADループの有効値
#define FLAGS_MSGPROC_GO_REQ	0x00000004	//メッセージ処理の開始指示
#define FLAGS_MSGPROC_GO_STA	0x00000008	//メッセージ処理の現在値

static NyLPC_TcSnicNetif_t _inst;
//RXメモリブロック
static NyLPC_TcSnicMM_t _txmem;

const struct NyLPC_TIPv4Addr* NyLPC_cSnicNetIf_getLocalAddr(void)
{
	return &_inst._local_ip;
}

void* NyLPC_cSnicNetIf_allocBuf(NyLPC_TUInt16 i_req_size,NyLPC_TUInt16*o_bufsize)
{
	void* r;
	NyLPC_cMutex_lock(&_inst._mutex);
	r=NyLPC_cSnicMM_alloc(&_txmem);
	NyLPC_cMutex_unlock(&_inst._mutex);
	return r;
}
void NyLPC_cSnicNetIf_releaseBuf(void* i_buf)
{
	NyLPC_cMutex_lock(&_inst._mutex);
	NyLPC_cSnicMM_release(&_txmem,i_buf);
	NyLPC_cMutex_unlock(&_inst._mutex);
}


NyLPC_TBool NyLPC_cSonicNetIf_initialize(void)
{
	//ここでリンクするSNICインタフェイス決めて
	_inst._dev=NyLPC_cSnicSerialDevice_getDevice();
	NyLPC_cSnicMM_initialize(&_txmem);
	if(!NyLPC_cMutex_initialize(&_inst._mutex)){
		NyLPC_Abort();
	}
	NyLPC_cIdPtrTable_initialize(&_inst._tbl);
	NyLPC_cSnicGeneral_initialize(&_inst._snic_general);
	NyLPC_cSnicWifi_initialize(&_inst._snic_wifi);
	NyLPC_cSnicApi_initialize(&_inst._snic_api);

	_inst._flags=FLAGS_INIT|FLAGS_THREAD_ACTIVE_REQ;
	NyLPC_cThread_initialize(&_inst._thread,SIZE_OF_STACK,NyLPC_TcThread_PRIORITY_SERVICE);
	//スレッドの開始待ち
	NyLPC_cThread_start(&_inst._thread,messageTask,NULL);
	while((_inst._flags&FLAGS_THREAD_ACTIVE_STA)==0){
		NyLPC_cThread_sleep(10);
	}
	return NyLPC_TBool_TRUE;
}
void NyLPC_cSonicNetIf_finalize(void)
{
	_inst._dev->finalize();
	//終了待機
	_inst._flags&=~FLAGS_THREAD_ACTIVE_REQ;
	while((_inst._flags&FLAGS_THREAD_ACTIVE_STA)==FLAGS_THREAD_ACTIVE_STA){
		NyLPC_cThread_sleep(10);
	}
	NyLPC_cThread_finalize(&_inst._thread);
	NyLPC_cIdPtrTable_finalize(&_inst._tbl);
	NyLPC_cMutex_finalize(&_inst._mutex);

}

NyLPC_TcSnicApi_t* NyLPC_cSnicNetIf_getSnicApi(void)
{
	return &_inst._snic_api;
}
NyLPC_TcSnicWifi_t* NyLPC_cSnicNetIf_getSnicWifi(void)
{
	return &_inst._snic_wifi;
}
NyLPC_TcSnicGeneral_t* NyLPC_cSnicNetIf_getSnicGeneral(void)
{
	return &_inst._snic_general;
}

NyLPC_TUInt8 NyLPC_cSnicNetIf_regieterObject(void* i_object)
{
	return NyLPC_cIdPtrTable_addItem(&_inst._tbl,i_object);
}
void NyLPC_cSnicNetIf_unregieterObject(void* i_object)
{
	NyLPC_cIdPtrTable_removeItem(&_inst._tbl,i_object);
}
void* NyLPC_cSnicNetIf_lockObjectById(NyLPC_TUInt8 i_id)
{
	return NyLPC_cIdPtrTable_getPtrByIdWithLock(&_inst._tbl,i_id);
}
void NyLPC_cSnicNetIf_unlockObject(const void* i_object)
{
	NyLPC_cIdPtTable_unLockPtr(&_inst._tbl,i_object);
}


NyLPC_TBool NyLPC_cSnicNetIf_readPayload(void* i_buf,NyLPC_TUInt8 i_len)
{
	return waitForRead(_inst._dev,(NyLPC_TUInt8*)i_buf,i_len);
}

void NyLPC_cSnicNetIf_seekPayload(NyLPC_TUInt16 i_seek)
{
	_inst._dev->pseek(i_seek);
}




#define SNIC_DATA_SOM 0x02
#define SNIC_DATA_EOM 0x04
#define SNIC_DATA_ESC 0x10
#define SNIC_DATA_MASK (SNIC_DATA_SOM|SNIC_DATA_EOM|SNIC_DATA_ESC)
#define SNIC_DATA_ESC_BIT 0x7f

/**
 * i_bufに指定長の長さi_lenのデータを読み出す
 */
static NyLPC_TBool waitForRead(struct NyLPC_TiSnicDevice_Interface* i_dev,NyLPC_TUInt8* i_buf,NyLPC_TUInt8 i_len)
{
	NyLPC_TUInt16 l,rp;
	NyLPC_TUInt16 wp=0;
	const NyLPC_TUInt8* d;
	for(;;){
		d=(const NyLPC_TUInt8*)i_dev->pread(&l);
		if(d==NULL){
			goto DROP;
		}
		rp=0;
		for(;rp<l;){
			i_buf[wp]=d[rp];
			wp++;
			if(wp==i_len){
				i_dev->pseek(rp+1);//読んだバイト数だけシーク
				return NyLPC_TBool_TRUE;
			}
			rp++;
		}
		i_dev->pseek(l);
	}
DROP:
	return NyLPC_TBool_FALSE;
}

/**
 * 1バイトのデータが到着するのを待つ
 * @return
 * TRUE:データを受信できた。/FALSE:データを受信できなかった
 */
static NyLPC_TBool waitforRawByte(struct NyLPC_TiSnicDevice_Interface* i_dev,NyLPC_TUInt8 i_byte)
{
	NyLPC_TUInt16 l;
	const NyLPC_TUInt8* d;
	d=(const NyLPC_TUInt8*)i_dev->pread(&l);
	if(d==NULL){
		goto DROP;
	}
	i_dev->pseek(1);
	if(*d!=i_byte){
		goto DROP;
	}
	return NyLPC_TBool_TRUE;
DROP:
	return NyLPC_TBool_FALSE;
}


void NyLPC_cSnicNetIf_startPayload(NyLPC_TUInt8 i_cmd,NyLPC_TUInt16 i_len)
{
	NyLPC_TUInt8* tx=_inst.tx_data.txbuf;
	tx[0]=SNIC_DATA_SOM;
	tx[1]=0x80|(i_len & 0x007f);
	tx[2]=0x80|((i_len & 0x3f80)>>7);
	tx[3]=0x80|i_cmd;
	NyLPC_cMutex_lock(&_inst._mutex);
	_inst._dev->write(tx,4);
	_inst.tx_data.checksum=tx[1]+tx[2]+i_cmd;
}

NyLPC_TUInt16 NyLPC_cSnicNetIf_getPayloadLength(const void* i_buf,NyLPC_TUInt16 i_len)
{
	return i_len;
}

void NyLPC_cSnicNetIf_sendPayload(const void* i_buf,NyLPC_TUInt16 i_len)
{
	if(i_len>0){
		_inst._dev->write(i_buf,i_len);
	}
}

void NyLPC_cSnicNetIf_endPayload()
{
	NyLPC_TUInt8* tx=_inst.tx_data.txbuf;
	tx[0]=_inst.tx_data.checksum|0x80;
	tx[1]=SNIC_DATA_EOM;
	_inst._dev->write(tx,2);
	NyLPC_cMutex_unlock(&_inst._mutex);
}


void NyLPC_cSnicNetIf_sendShortPayload(const void* i_buf,NyLPC_TUInt8 i_cmd,NyLPC_TUInt16 i_len)
{
	NyLPC_TUInt16 l;
	l=NyLPC_cSnicNetIf_getPayloadLength(i_buf,i_len);
	NyLPC_cSnicNetIf_startPayload(i_cmd,l);
	NyLPC_cSnicNetIf_sendPayload(i_buf,i_len);
	NyLPC_cSnicNetIf_endPayload();
}




static NyLPC_TBool recvHeader(struct NyLPC_TiSnicDevice_Interface* i_dev,struct NyLPC_TUartFrameFormatHeader* i_header){
	return waitForRead(i_dev,(NyLPC_TUInt8*)i_header,NyLPC_TUartFrameFormatHeader_SIZE_OF_DATA);
}


static int messageTask(void* i_param)
{
	struct NyLPC_TUartFrameFormatHeader payload_header;
	NyLPC_TcSnicNetif_t* inst=&_inst;
	//スレッドステータス有効
	while((inst->_flags&FLAGS_THREAD_ACTIVE_REQ)!=0){
		inst->_flags|=FLAGS_THREAD_ACTIVE_STA;
		if((inst->_flags&FLAGS_MSGPROC_GO_REQ)==0){
			inst->_flags&=~FLAGS_MSGPROC_GO_STA;
			NyLPC_cThread_sleep(10);
		}else{
			inst->_flags|=FLAGS_MSGPROC_GO_STA;
			if(!waitforRawByte(inst->_dev,SNIC_DATA_SOM)){
				continue;
			}
			if(!recvHeader(inst->_dev,&payload_header)){
				continue;
			}
			switch(NyLPC_TUartFrameFormat_getCmd(&payload_header)){
			case SNIC_UART_CMD_ID_GEN:
				if(!NyLPC_cSnicGeneral_rxHandler(inst->_dev)){
					continue;
				}
				break;
			case SNIC_UART_CMD_ID_SNIC:
				if(!NyLPC_cSnicApi_rxHandler(&inst->_snic_api,inst->_dev)){
					continue;
				}
				break;
			case SNIC_UART_CMD_ID_WIFI:
				if(!NyLPC_cSnicWifi_rxHandler(inst->_dev)){
					continue;
				}
				break;
			}
			if(!waitforRawByte(inst->_dev,SNIC_DATA_ESC)){
				continue;
			}
		}
	}
	//スレッドステータス無効
	inst->_flags&=~FLAGS_THREAD_ACTIVE_STA;
	return 0;
}



#define FLAGS_USED	0x00000001
struct TTcpListenerTable{
	NyLPC_TUInt32 flags;
	NyLPC_TcSnicTcpListener_t listener;
};
struct TTcpSocketTable{
	NyLPC_TUInt32 flags;
	NyLPC_TcSnicTcpSocket_t socket;
	NyLPC_TUInt8 rxbuf[NyLPC_TcSnicNetIf_TCP_RX_BUFFER_SIZE];
};
struct TUdpSocketTable{
	NyLPC_TUInt32 flags;
	NyLPC_TcSnicUdpSocket_t socket;
	NyLPC_TUInt8 rxbuf[NyLPC_TcSnicNetIf_UDP_RX_BUFFER_SIZE];
};
struct TUdpSocketTableNB{
	NyLPC_TUInt32 flags;
	NyLPC_TcSnicUdpSocket_t socket;
};

static struct TTcpListenerTable listener[NyLPC_TcSnicNetIf_TCPLISTENER_MAX];
static struct TTcpSocketTable tcpsocket[NyLPC_TcSnicNetIf_TCPSOCKET_MAX];
static struct TUdpSocketTable udpsocket[NyLPC_TcSnicNetIf_UDPSOCKET_MAX];
static struct TUdpSocketTableNB udpsocket_nb[NyLPC_TcSnicNetIf_UDP_NB_SOCKET_MAX];


NyLPC_TcSnicTcpListener_t* NyLPC_cSnicNetIf_getTcpListenerBySocket(NyLPC_TUInt8 i_sock)
{
	NyLPC_TUInt16 i;
	for(i=0;i<NyLPC_TcSnicNetIf_TCPLISTENER_MAX;i++){
		if(NyLPC_cSnicTcpListener_isSocketEqual(&listener[i].listener,i_sock)){
			return &listener[i].listener;
		}
	}
	return NULL;
}
NyLPC_TcSnicTcpSocket_t* NyLPC_cSnicNetIf_getTcpSocketBySocket(NyLPC_TUInt8 i_sock)
{
	NyLPC_TUInt16 i;
	for(i=0;i<NyLPC_TcSnicNetIf_TCPSOCKET_MAX;i++){
		if(NyLPC_cSnicTcpSocket_isSocketEqual(&tcpsocket[i].socket,i_sock)){
			return &tcpsocket[i].socket;
		}
	}
	return NULL;
}

NyLPC_TcSnicUdpSocket_t* NyLPC_cSnicNetIf_getUdpSocketBySocket(NyLPC_TUInt8 i_sock)
{
	NyLPC_TUInt16 i;
	for(i=0;i<NyLPC_TcSnicNetIf_UDPSOCKET_MAX;i++){
		if(NyLPC_cSnicUdpSocket_isSocketEqual(&udpsocket[i].socket,i_sock)){
			return &udpsocket[i].socket;
		}
	}
	for(i=0;i<NyLPC_TcSnicNetIf_UDP_NB_SOCKET_MAX;i++){
		if(NyLPC_cSnicUdpSocket_isSocketEqual(&udpsocket_nb[i].socket,i_sock)){
			return &udpsocket_nb[i].socket;
		}
	}
	return NULL;
}

void NyLPC_cSnicNetIf_removeTcpListener(const NyLPC_TcSnicTcpListener_t* i_listener)
{
	NyLPC_TUInt16 i;
	for(i=0;i<NyLPC_TcSnicNetIf_TCPLISTENER_MAX;i++){
		if(&listener[i].listener==i_listener){
			listener[i].flags|=FLAGS_USED;
			break;
		}
	}
}
void NyLPC_cSnicNetIf_removeTcpSocket(const NyLPC_TcSnicTcpSocket_t* i_socket)
{
	NyLPC_TInt16 i;
	for(i=0;i<NyLPC_TcSnicNetIf_TCPSOCKET_MAX;i++){
		if(&tcpsocket[i].socket==i_socket){
			tcpsocket[i].flags|=FLAGS_USED;
			break;
		}
	}
}
void NyLPC_cSnicNetIf_removeUdpSocket(const NyLPC_TcSnicUdpSocket_t* i_socket)
{
	NyLPC_TInt16 i;
	for(i=0;i<NyLPC_TcSnicNetIf_UDPSOCKET_MAX;i++){
		if(&udpsocket[i].socket==i_socket){
			udpsocket[i].flags|=FLAGS_USED;
			break;
		}
	}
	for(i=0;i<NyLPC_TcSnicNetIf_UDP_NB_SOCKET_MAX;i++){
		if(&udpsocket_nb[i].socket==i_socket){
			udpsocket_nb[i].flags|=FLAGS_USED;
			break;
		}
	}
}
/*
 *	NetIF Interface
 */

static NyLPC_TiTcpSocket_t* createTcpSocetEx(NyLPC_TSocketType i_socktype)
{
	NyLPC_TUInt16 i;
	switch(i_socktype){
	case NyLPC_TSocketType_TCP_HTTP:
	case NyLPC_TSocketType_TCP_NORMAL:
		break;
	default:
		return NULL;
	}
	//空きソケットの探索
	for(i=0;i<NyLPC_TcSnicNetIf_TCPSOCKET_MAX;i++){
		if((tcpsocket[i].flags&FLAGS_USED)==0){
			if(NyLPC_cSnicTcpSocket_initialize(&tcpsocket[i].socket,tcpsocket[i].rxbuf,NyLPC_TcSnicNetIf_TCP_RX_BUFFER_SIZE,i_socktype)){
				tcpsocket[i].flags|=FLAGS_USED;
				return &tcpsocket[i].socket.super;
			}
		}
	}
	return NULL;
}

static NyLPC_TiUdpSocket_t* createUdpSocetEx(NyLPC_TUInt16 i_port,NyLPC_TSocketType i_socktype)
{
	NyLPC_TUInt16 i;
	switch(i_socktype){
	case NyLPC_TSocketType_UDP_NORMAL:
		if((udpsocket[i].flags&FLAGS_USED)==0){
			//空きソケットの探索
			for(i=0;i<NyLPC_TcSnicNetIf_UDPSOCKET_MAX;i++){
				if(NyLPC_cSnicUdpSocket_initialize(&udpsocket[i].socket,i_port,udpsocket[i].rxbuf,NyLPC_TcSnicNetIf_UDP_RX_BUFFER_SIZE)){
					udpsocket[i].flags|=FLAGS_USED;
					return &udpsocket[i].socket._super;
				}
			}
		}
		break;
	case NyLPC_TSocketType_UDP_NOBUF:
		//空きソケットの探索
		if((udpsocket_nb[i].flags&FLAGS_USED)==0){
			for(i=0;i<NyLPC_TcSnicNetIf_UDP_NB_SOCKET_MAX;i++){
				if(NyLPC_cSnicUdpSocket_initialize(&udpsocket_nb[i].socket,i_port,NULL,0)){
					udpsocket_nb[i].flags|=FLAGS_USED;
					return &udpsocket_nb[i].socket._super;
				}
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
	for(i=0;i<NyLPC_TcSnicNetIf_TCPLISTENER_MAX;i++){
		//未使用なソケットを得る
		if((listener[i].flags&FLAGS_USED)==0){
			if(NyLPC_cSnicTcpListener_initialize(&listener[i].listener,i_port)){
				listener[i].flags|=FLAGS_USED;
				return &listener[i].listener.super;
			}
		}
	}
	return NULL;
}
static const struct NyLPC_TNetInterfaceInfo* getInterfaceInfo(void)
{
	NyLPC_TcSnicNetif_t* inst=&_inst;
    return &inst->_netinfo;
}
const static char* DEV_NAME="MURATA_WIFI";
static void start(const NyLPC_TcIPv4Config_t* i_ref_config)
{
	//ループ開始要求
	_inst._flags|=FLAGS_MSGPROC_GO_REQ;
	while((_inst._flags&FLAGS_MSGPROC_GO_STA)==0){
		NyLPC_cThread_sleep(10);
	}
	_inst._dev->reset();
	NyLPC_cSnicApi_configure(&_inst._snic_api,&i_ref_config->ip_addr,&i_ref_config->netmask,&i_ref_config->dr_addr);
    _inst._netinfo.current_config=i_ref_config;
    _inst._netinfo.device_name=DEV_NAME;
	return;
}
/**
 * UIP処理を停止します。
 * この関数はリエントラントではありません。複数のタスクから共有するときには、排他ロックを掛けてください。
 * いまのところ、ストップシーケンスの実装は良くありません。
 * 再設計が必要。
 */
static void stop(void)
{
	//ループ停止要求
	_inst._flags&=~FLAGS_MSGPROC_GO_REQ;
	while((_inst._flags&FLAGS_MSGPROC_GO_STA)!=0){
		NyLPC_cThread_sleep(10);
	}
    return;
}

static const struct NyLPC_TiNetInterface_Interface _interface=
{
	createTcpSocetEx,
	createUdpSocetEx,
	createTcpListener,
	start,//NyLPC_cSnicNetIf_start,
	stop,//NyLPC_cSnicIpNetIf_stop,
	NULL,//NyLPC_cSnicIpNetIf_sendArpRequest,
	NULL,//NyLPC_cSnicIpNetIf_hasArpInfo,
	NULL,//isInitService,//NyLPC_TiNetInterface_isInitService isinitservice;
	getInterfaceInfo,//NyLPC_cSnicIpNetIf_getInterfaceInfo
};






#ifdef COMMENT_OUT
#define SIZE_OF_TXBUF	1024
static NyLPC_TUInt8 txbuf[SIZE_OF_TXB];
NyLPC_TUInt16 NyLPC_cSnicNetIf_getPayloadLength(const void* i_buf,NyLPC_TUInt16 i_len)
{
	NyLPC_TUInt16 i;
	NyLPC_TUInt16 ret=0;
	for(i=0;i<i_len;i++){
		if((((NyLPC_TUInt8*)i_buf)[i]&SNIC_DATA_MASK)!=0){
			switch(((NyLPC_TUInt8*)i_buf)[i]){
			case SNIC_DATA_SOM:
			case SNIC_DATA_EOM:
			case SNIC_DATA_ESC:
				ret++;
			}
		}
	}
	return ret+i;
}

void NyLPC_cSnicNetIf_sendPayload(const void* i_buf,NyLPC_TUInt16 i_len)
{
	NyLPC_TUInt8* tx=_inst.tx_data.txbuf;
	NyLPC_TUInt16 i;
	NyLPC_TUInt8 d;
	NyLPC_TInt8 wp;
	//Encode
	wp=_inst.tx_data.len;
	for(i=0;i<i_len;i++){
		d=((NyLPC_TUInt8*)i_buf)[i];
		if((d&SNIC_DATA_MASK)!=0){
			switch(d){
			case SNIC_DATA_ESC:
			case SNIC_DATA_SOM:
			case SNIC_DATA_EOM:
				tx[wp]=SNIC_DATA_ESC;
				tx[wp+1]=0x80|d;
				wp+=2;
				break;
			default:
				tx[wp]=d;
				wp++;
				break;
			}
		}else{
			tx[wp]=d;
			wp++;
		}
		//2文字分の空きが無ければNG
		if(wp>=SIZE_OF_TXB-2){
			//送信
			_inst._dev->write(tx,wp);
			wp=0;
		}
	}
	_inst.tx_data.len=wp;
}

void NyLPC_cSnicNetIf_endPayload()
{
	NyLPC_TUInt8* tx=_inst.tx_data.txbuf;
	NyLPC_TInt8 wp=_inst.tx_data.len;
	tx[wp]=_inst.tx_data.checksum|0x80;
	tx[wp+1]=SNIC_DATA_EOM;
	wp+=2;
	_inst._dev->write(tx,wp);
}
static NyLPC_TBool waitForRead(struct NyLPC_TiSnicDevice_Interface* i_dev,NyLPC_TUInt8* i_buf,NyLPC_TUInt8 i_len)
{
	NyLPC_TUInt16 l,rp;
	NyLPC_TUInt16 wp=0;
	const NyLPC_TUInt8* d;
	for(;;){
		d=(const NyLPC_TUInt8*)i_dev->pread(&l);
		if(d==NULL){
			goto DROP;
		}
		rp=0;
		for(;rp<l;){
			if((d[rp]&SNIC_DATA_MASK)!=0){
				switch(d[rp]){
				case SNIC_DATA_ESC:
					//2文字は読めないとね。
					if(l-rp<2){
						//パケット全体をシーク
						i_dev->pseek(l); //読んだバイト数だけシーク
						d=(NyLPC_TUInt8*)i_dev->pread(&l);
						if(d==NULL){
							goto DROP;
						}
						rp=0;
					}else{
						rp++;
					}
					i_buf[wp]=d[rp] & (~SNIC_DATA_ESC_BIT);
					wp++;
					if(wp==i_len){
						i_dev->pseek(rp+1);
						return NyLPC_TBool_TRUE;
					}
					rp++;
					continue;
				case SNIC_DATA_SOM:
				case SNIC_DATA_EOM:
					i_dev->pseek(rp+1);//読んだバイト数だけシーク
					goto DROP;
				default:
					//一般文字処理
					break;
				}
			}
			i_buf[wp]=d[rp];
			wp++;
			if(wp==i_len){
				i_dev->pseek(rp+1);//読んだバイト数だけシーク
				return NyLPC_TBool_TRUE;
			}
			rp++;
		}
		i_dev->pseek(l);
	}
DROP:
	return NyLPC_TBool_FALSE;
}
#endif
