#include "NyLPC_os.h"
#include "NyLPC_cIdPtrTable.h"
#include "NyLPC_iSnicDevice.h"
#include "NyLPC_Snic_types.h"
#include "NyLPC_cIdPtrTable.h"
#include "NyLPC_cSnicNetIf_protected.h"
#include "io/NyLPC_cSnicSerialDevice.h"

static int messageTask(void* i_param);
static NyLPC_TBool waitForRead(struct NyLPC_TiSnicDevice_Interface* i_dev,NyLPC_TUInt8* i_buf,NyLPC_TUInt8 i_len);

#define ESC_ENEBLE 1

#define SIZE_OF_STACK	(512+256)
#define SIZE_OF_TXB 32+2

typedef struct NyLPC_TcSnicNetif NyLPC_TcSnicNetif_t;
struct NyLPC_TcSnicNetif{
	NyLPC_TcThread_t _thread;
	NyLPC_TcIdPtrTable_t _tbl;
	struct NyLPC_TiSnicDevice_Interface* _dev;
	struct{
		NyLPC_TUInt8 txbuf[SIZE_OF_TXB];
		NyLPC_TInt8 len;
		NyLPC_TUInt8 checksum;
	}tx_data;
};
static NyLPC_TcSnicNetif_t _inst;

#define SIZE_OF_TXBUF	1024
NyLPC_TUInt8 txbuf[SIZE_OF_TXB];


NyLPC_TBool NyLPC_cSonicNetIf_initialize(void)
{
	//ここでリンクするSNICインタフェイス決めて
	_inst._dev=NyLPC_cSnicSerialDevice_getDevice();
	NyLPC_cIdPtrTable_initialize(&_inst._tbl);
	NyLPC_cThread_initialize(&_inst._thread,SIZE_OF_STACK,NyLPC_TcThread_PRIORITY_SERVICE);
	NyLPC_cThread_start(&_inst._thread,messageTask,NULL);
	_inst._dev->reset();
	return NyLPC_TBool_TRUE;
}
void NyLPC_cSonicNetIf_finalize(void)
{
	_inst._dev->finalize();
	NyLPC_cIdPtrTable_finalize(&_inst._tbl);
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


extern NyLPC_TBool NyLPC_cSnicGeneral_rxHandler(struct NyLPC_TiSnicDevice_Interface* i_dev);
extern NyLPC_TBool NyLPC_cSnicWifi_rxHandler(struct NyLPC_TiSnicDevice_Interface* i_dev);

static int messageTask(void* i_param)
{
	struct NyLPC_TUartFrameFormatHeader payload_header;
	NyLPC_TcSnicNetif_t* inst=&_inst;
	for(;;){
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
			break;
		case SNIC_UART_CMD_ID_WIFI:
			if(!NyLPC_cSnicWifi_rxHandler(inst->_dev)){
				continue;
			}
			break;
		}
//		if(!recvPayload()){
//sendNACK
//			continue;
//		}
//		if(!waitCheckSum()){
//sendNACK
//			continue;
//		}
		if(!waitforRawByte(inst->_dev,SNIC_DATA_ESC)){
			continue;
		}
		//SendACK
	}
	return 0;
}

#ifdef COMMENT_OUT
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
