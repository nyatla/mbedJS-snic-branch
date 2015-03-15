/*
 * NyLPC_cSonicSerial.cpp
 *
 *  Created on: 2014/11/20
 *      Author: nyatla
 */

#include "NyLPC_cSnicSerialDevice.h"
#include "mbed.h"
#include "NyLPC_os.h"
#include "NyLPC_stdlib.h"

static void uartHandler();

#define SIZE_OF_RX_BUF 256

/** 受信メモリ*/
static NyLPC_TUInt8 rxbuf[SIZE_OF_RX_BUF];


typedef struct NyLPC_TcSnicIo NyLPC_TcSnicIo_t;

struct NyLPC_cSnicSerialDevice{
	volatile NyLPC_TUInt16 _rp;
	volatile NyLPC_TUInt16 _wp;
	RawSerial* _serial;
	NyLPC_TcSemaphore_t _rx_semapho;
};

static struct NyLPC_cSnicSerialDevice _inst;


void NyLPC_cSnicSerialDevice_initialize(PinName i_rx,PinName i_tx,int i_baud)
{
	_inst._rp=0;
	_inst._wp=0;
	NyLPC_cSemaphore_initialize(&_inst._rx_semapho);
	_inst._serial=new RawSerial(i_rx,i_tx);
	_inst._serial->baud(i_baud);
	_inst._serial->format(8, SerialBase::None, 1);
	_inst._serial->attach(uartHandler);
}
static void NyLPC_cSnicSerialDevice_finalize(void)
{
	delete _inst._serial;
	NyLPC_cSemaphore_finalize(&_inst._rx_semapho);
}

/**
 * 通信キューからデータを受信します。
 * 受診されるまでブロックします。
 */
static const void* NyLPC_cSnicSerialDevice_pread(NyLPC_TUInt16* i_len)
{
	volatile NyLPC_TUInt16 wp;
	volatile NyLPC_TUInt16 rp;
	while(_inst._rp==_inst._wp){
		NyLPC_cSemaphore_take(&_inst._rx_semapho,30);
	}
	wp=_inst._wp;
	rp=_inst._rp;
	if(rp<=wp){
		*i_len=wp-rp;
	}else{
		*i_len=SIZE_OF_RX_BUF-rp;
	}
	return &rxbuf[rp];
}


static void NyLPC_cSnicSerialDevice_pseek(NyLPC_TUInt16 i_len){
	_inst._rp=(_inst._rp+i_len)%SIZE_OF_RX_BUF;
}


static void NyLPC_cSnicSerialDevice_write(const void* i_buf,NyLPC_TUInt16 i_length)
{
	for(int i=0;i<i_length;i++){
		while(!_inst._serial->writeable()){
			NyLPC_cThread_yield();
		}
		_inst._serial->putc(*(((NyLPC_TUInt8*)i_buf)+i));
	}
	return;
}
static void NyLPC_cSnicSerialDevice_reset(void)
{

}


void NyLPC_cSemaphore_take(const NyLPC_TcSemaphore_t* i_inst,NyLPC_TUInt32 i_timeout);


static void uartHandler()
{
	//キュー空き待ち
	while(_inst._serial->readable()){
		if(_inst._rp==((_inst._wp+1)%SIZE_OF_RX_BUF)){
			return;//DROP;
		}
		rxbuf[_inst._wp]=(NyLPC_TUInt8)_inst._serial->getc();
		_inst._wp=(_inst._wp+1)%SIZE_OF_RX_BUF;
		NyLPC_cSemaphore_giveFromISR(&_inst._rx_semapho);
	}
}
/*
 * Interface
 */

static struct NyLPC_TiSnicDevice_Interface _interface={
	NyLPC_cSnicSerialDevice_finalize,
	NyLPC_cSnicSerialDevice_pread,
	NyLPC_cSnicSerialDevice_pseek,
	NyLPC_cSnicSerialDevice_write,
	NyLPC_cSnicSerialDevice_reset
};

struct NyLPC_TiSnicDevice_Interface* NyLPC_cSnicSerialDevice_getDevice(void)
{
	NyLPC_cSnicSerialDevice_initialize(p9, p10,115200);
	return &_interface;
}

