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
 *	http://nyatla.jp/
 *	<airmail(at)ebony.plala.or.jp> or <nyatla(at)nyatla.jp>
 *
 *********************************************************************************/
#include "NyLPC_cSsdpSocket.h"
#include "NyLPC_http.h"
#include "NyLPC_netif.h"


#include <stdio.h>
#include <string.h>





#define HTTP_SP 0x20

#define PARSE_NULL  0
#define PARSE_ST	0x01
#define PARSE_MAN	0x11
#define PARSE_UNKNOWN 0xff

static const struct NyLPC_TIPv4Addr SSDP_MCAST_IPADDR=NyLPC_TIPv4Addr_pack(239,255,255,250);
static const char* STR_UPNP_ROOT_DEVICE="upnp:rootdevice";

struct TMSearchHeader
{
	struct NyLPC_THttpBasicHeader super;

	const struct NyLPC_TUPnPDeviceRecord* _ref_devices;
	/**
	 * パーサのステータス
	 */
	NyLPC_TUInt8 st;
	/**
	 * メモリ位置
	 */
	const NyLPC_TChar* _rpos;
	struct{
		const NyLPC_TChar* st_str;
		const NyLPC_TChar* man_str;
		NyLPC_TUInt16 st_len;
		NyLPC_TUInt16 man_len;
	}result;
};

//とりあえずprivate
void NyLPC_cSsdpSocket_notify(NyLPC_TcSsdpSocket_t* i_inst);


static NyLPC_TBool urlHandler(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
//　*であるかを確認　未実装
	return NyLPC_TBool_TRUE;
}

#define TIMEOUT_IN_MS 100

/**
 * SERVER MessageHeaderの値
 * 40文字以内であること。
 */
#define SERVER_MESSAGE_HEADER "MiMic/1.4 UPnP/1.0 MiMicUPnP/0.2"


/**
 * MsearchResponseを格納したTxパケットをAllocする。
 * @param i_st
 * ST値
 * @param　i_udn
 * DDESCのUDNの値
 * @param i_usn
 * USNのサフィックスパラメータ
 * @return
 * MsearchResponseを格納したTXメモリ
 */
static void* allocMsearchResponeTx(
	NyLPC_TcSsdpSocket_t* i_inst,
	const NyLPC_TChar* i_st,
	const NyLPC_TChar* i_udn,
	const NyLPC_TChar* i_usn,
	NyLPC_TUInt16 i_st_len,
	NyLPC_TInt16* o_len)
{
	NyLPC_TChar* obuf;
	NyLPC_TUInt16 l;
	NyLPC_TUInt16 len_usn=(NyLPC_TUInt16)((i_usn!=NULL)?strlen(i_usn):0);
	NyLPC_TUInt16 len_udn=(NyLPC_TUInt16)strlen(i_udn);
	NyLPC_TUInt16 len_location=(NyLPC_TUInt16)strlen(i_inst->location_path);

	//	//161Byte
	//	"HTTP/1.1 200 OK\r\n" 							//15+2=17
	//	"CACHE-CONTROL: max-age = nnnn\r\n" 			//29+2=31
	//	"SERVER: [:40byte:]\r\n"						//8+40+2=50
	//	"EXT: \r\n" 									//5+2 = 7
	//	"LOCATION: http://xxx.xxx.xxx.xxx:nnnnn/%s/d.xml\r\n"	//34+2=46
	//	"USN: %s%s\r\n"									//5+2=7
	//	"ST: %s\r\n\r\n"								//4+4=8
	l=166+len_location+len_usn+len_udn+i_st_len;
	obuf=NyLPC_iUdpSocket_allocSendBuf(i_inst->_socket,l,&l,TIMEOUT_IN_MS);

	if(obuf==NULL){
		return NULL;
	}
	//必要なメモリサイズを確保できた?
	if(l<161+len_location+len_usn+len_udn+i_st_len)
	{
		NyLPC_iUdpSocket_releaseSendBuf(i_inst->_socket,obuf);
		return NULL;
	}
	//ワーク変数lの再初期化
	l=0;
	strcpy(obuf,
		"HTTP/1.1 200 OK\r\n"
		"CACHE-CONTROL: max-age = 300\r\n"
		"SERVER: "SERVER_MESSAGE_HEADER"\r\n"
		"EXT: \r\n"
		"LOCATION: http://");
	l+=strlen(obuf);
	//IP addr:port\r\n
	l+=NyLPC_TIPv4Addr_toString(NyLPC_iUdpSocket_getSockIP(i_inst->_socket),obuf+l);
	*(obuf+l)=':';
	l+=1+NyLPC_itoa(i_inst->location_port,obuf+l+1,10);
	*(obuf+l)='/';l++;
	memcpy(obuf+l,i_inst->location_path,len_location);l+=len_location;
	memcpy(obuf+l,"/d.xml",6);l+=6;
	*(obuf+l+0)='\r';
	*(obuf+l+1)='\n';
	l+=2;
	//USN: uuid:xxx
	memcpy(obuf+l,"USN: ",5);		l+=5;
	memcpy(obuf+l,i_udn,len_udn);	l+=len_udn;	//uuid:xxx
	if(i_usn!=NULL){
		*(obuf+l+0)=':';
		*(obuf+l+1)=':';
		l+=2;
		memcpy(obuf+l,i_usn,len_usn);l+=len_usn;	//usn:xxx
	}
	*(obuf+l+0)='\r';
	*(obuf+l+1)='\n';
	l+=2;
	//ST
	memcpy(obuf+l,"ST: ",4);	l+=4;
	memcpy(obuf+l,i_st,i_st_len);l+=i_st_len;
	memcpy(obuf+l,"\r\n\r\n",4);	l+=4;
	*o_len=l;
	return obuf;
}


/**
 * MsearchResponseを格納したTxパケットをAllocする。
 * @param i_udn
 * udn
 * @param　i_udn
 * DDESCのUDNの値
 * @param i_usn
 * USNのサフィックスパラメータ
 * @return
 * MsearchResponseを格納したTXメモリ
 */
static void* allocNotifyTx(
	NyLPC_TcSsdpSocket_t* i_inst,
	const NyLPC_TChar* i_udn,
	const NyLPC_TChar* i_usn,
	NyLPC_TInt16* o_len)
{
	NyLPC_TChar* obuf;
	NyLPC_TUInt16 l,l2;
	NyLPC_TUInt16 len_usn=(NyLPC_TUInt16)((i_usn!=NULL)?strlen(i_usn):0);
	NyLPC_TUInt16 len_udn=(NyLPC_TUInt16)strlen(i_udn);
	NyLPC_TUInt16 len_location=(NyLPC_TUInt16)strlen(i_inst->location_path);

	//	//193Byte
	//	"NOTIFY * HTTP/1.1\r\n" 						//15+2=17
	//	"HOST: 239.255.255.250:1900\r\n"				//26+2=28
	//	"CACHE-CONTROL: max-age = 1800\r\n" 			//29+2=31
	//	"SERVER: [:40byte:]\r\n"						//8+40+2=50
	//	"NTS: ssdp:alive\r\n"							//14+2 =17
	//	"LOCATION: http://xxx.xxx.xxx.xxx:nnnnn/%s/d.xml\r\n"//44+2=46
	//	"USN: %s%s\r\n"									//5+2=7
	//	"NT: %s\r\n\r\n"								//4+4=8
	l2=204+len_location+len_usn+len_udn+((len_usn>0)?len_usn:len_udn);
	obuf=NyLPC_iUdpSocket_allocSendBuf(i_inst->_socket,l2,&l,TIMEOUT_IN_MS);
	if(obuf==NULL){
		return NULL;
	}
	//必要なメモリサイズを確保できた?
	if(l<l2)
	{
		NyLPC_iUdpSocket_releaseSendBuf(i_inst->_socket,obuf);
		return NULL;
	}
	//ワーク変数lの再初期化
	l=0;
	strcpy(obuf,
		"NOTIFY * HTTP/1.1\r\n"
		"HOST: 239.255.255.250:1900\r\n"
		"CACHE-CONTROL: max-age = 300\r\n"
		"SERVER: "SERVER_MESSAGE_HEADER"\r\n"
		"NTS: ssdp:alive\r\n"
		"LOCATION: http://");
	l+=strlen(obuf);
	//IP addr:port\r\n
	l+=NyLPC_TIPv4Addr_toString(NyLPC_iUdpSocket_getSockIP(i_inst->_socket),obuf+l);
	*(obuf+l)=':';
	l+=1+NyLPC_itoa(i_inst->location_port,obuf+l+1,10);
	*(obuf+l)='/';l++;
	memcpy(obuf+l,i_inst->location_path,len_location);l+=len_location;
	memcpy(obuf+l,"/d.xml",6);l+=6;
	*(obuf+l+0)='\r';
	*(obuf+l+1)='\n';
	l+=2;
	//USN: uuid:xxx
	memcpy(obuf+l,"USN: ",5);	l+=5;
	memcpy(obuf+l,i_udn,len_udn);	l+=len_udn;	//uuid:xxx
	if(i_usn!=NULL){
		*(obuf+l+0)=':';
		*(obuf+l+1)=':';
		l+=2;
		memcpy(obuf+l,i_usn,len_usn);l+=len_usn;	//usn:xxx
	}
	*(obuf+l+0)='\r';
	*(obuf+l+1)='\n';
	l+=2;
	//NT
	memcpy(obuf+l,"NT: ",4);	l+=4;
	if(len_usn>0){
		memcpy(obuf+l,i_usn,len_usn);l+=len_usn;
	}else{
		memcpy(obuf+l,i_udn,len_udn);l+=len_udn;
	}
	memcpy(obuf+l,"\r\n\r\n",4);	l+=4;
	*o_len=l;
	return obuf;
}


static NyLPC_TBool messageHandler(NyLPC_TcHttpBasicHeaderParser_t* i_inst,const NyLPC_TChar* i_name,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
	struct TMSearchHeader* header=(struct TMSearchHeader*)o_out;
	switch(header->st)
	{
		case PARSE_NULL:
			if(NyLPC_stricmp(i_name,"ST")==0){
				//mode==ST
				header->st=PARSE_ST;
				header->result.st_str=NULL;
			}else if(NyLPC_stricmp(i_name,"MAN")==0){
				//mode=MAN
				header->st=PARSE_MAN;
				header->result.man_str=NULL;
			}else{
				header->st=PARSE_UNKNOWN;
				//無視
			}
			break;
		case PARSE_ST:
			if((header->result.st_str==NULL) && (i_c!=HTTP_SP)){
				header->result.st_str=header->_rpos;
			}
			if(i_c=='\0')
			{
				header->result.st_len=header->_rpos-header->result.st_str-1;
				header->st=PARSE_NULL;
			}
			break;
		case PARSE_MAN:
			if((header->result.man_str==NULL) && (i_c!=HTTP_SP)){
				header->result.man_str=header->_rpos;
			}
			if(i_c=='\0'){
				header->result.man_len=header->_rpos-header->result.man_str-1;
				header->st=PARSE_NULL;
			}
			break;
		case PARSE_UNKNOWN:
		default:
			if(i_c=='\0'){
				header->st=PARSE_NULL;
			}
			break;
	}
	return NyLPC_TBool_TRUE;
}

/**
 * デフォルトハンドラ
 */
static const struct NyLPC_TcHttpBasicHeaderParser_Handler handler=
{
	messageHandler,
	urlHandler
};

static NyLPC_TBool parseHeader(struct TMSearchHeader* i_out,const void* i_rx,NyLPC_TInt16 i_rx_size)
{
	NyLPC_TInt16 i;
	NyLPC_TcHttpBasicHeaderParser_t parser;
	//headerの初期化
	i_out->st=PARSE_NULL;
	i_out->result.st_str=NULL;
	i_out->result.man_str=NULL;
	NyLPC_cHttpBasicHeaderParser_initialize(&parser,&handler);
	NyLPC_cHttpBasicHeaderParser_parseInit(&parser,&(i_out->super));
	for(i=0;i<i_rx_size;i++){
		i_out->_rpos=((const char*)(i_rx))+i;
		if(NyLPC_cHttpBasicHeaderParser_parseChar(&parser,i_out->_rpos,1,&(i_out->super))<0){
			NyLPC_cHttpBasicHeaderParser_finalize(&parser);
			return NyLPC_TBool_FALSE;//ERROR
		}
	}
	NyLPC_cHttpBasicHeaderParser_parseFinish(&parser,&(i_out->super));
	NyLPC_cHttpBasicHeaderParser_finalize(&parser);
	return NyLPC_TBool_TRUE;//OK
}

static NyLPC_TBool onPacket(NyLPC_TiUdpSocket_t* i_sock,const void* i_buf,const struct NyLPC_TIPv4RxInfo* i_info)
{
	//パケット解析
	void* tx;
	struct TMSearchHeader header;
	NyLPC_TInt16 tx_len;
	NyLPC_TInt8 i,i2;
	NyLPC_TcSsdpSocket_t* inst=((NyLPC_TcSsdpSocket_t*)i_sock->_tag);
	if(!parseHeader(&header,i_buf,i_info->size)){
		NyLPC_OnErrorGoto(ERROR1);
	}
	//resultチェック
	if(header.result.man_str==NULL || header.result.st_str==NULL){
		NyLPC_OnErrorGoto(ERROR1);
	}
	//Methodチェック
	if(header.super.startline.req.method!=NyLPC_THttpMethodType_M_SEARCH){
		NyLPC_OnErrorGoto(ERROR1);
	}

	//MANチェック
	if(strncmp("\"ssdp:discover\"",header.result.man_str,15)!=0){
		NyLPC_OnErrorGoto(ERROR1);
	}
	//STによる処理分岐
	if(strncmp("ssdp:all",header.result.st_str,8)==0){
		tx=allocMsearchResponeTx(
			inst,header.result.st_str,
			inst->ref_device_record[0]->udn,STR_UPNP_ROOT_DEVICE,
			header.result.st_len,
			&tx_len);
		if(tx==NULL){
			NyLPC_OnErrorGoto(ERROR1);
		}
		if(!NyLPC_iUdpSocket_psend(i_sock,&i_info->peer_ip,i_info->peer_port,tx,tx_len)){
			NyLPC_OnErrorGoto(ERROR2);
		}
		//全デバイスの送信
		for(i=0;i<inst->number_of_device;i++){
			tx=allocMsearchResponeTx(
				inst,header.result.st_str,
				inst->ref_device_record[i]->udn,inst->ref_device_record[i]->device_type,
				header.result.st_len,
				&tx_len);
			if(tx==NULL){
				NyLPC_OnErrorGoto(ERROR1);
			}
			if(!NyLPC_iUdpSocket_psend(i_sock,&i_info->peer_ip,i_info->peer_port,tx,tx_len)){
				NyLPC_OnErrorGoto(ERROR2);
			}
			for(i2=0;i2<inst->ref_device_record[i]->number_of_service;i2++){
				//serviceに一致
				tx=allocMsearchResponeTx(
					inst,header.result.st_str,
					inst->ref_device_record[i]->udn,inst->ref_device_record[i]->services[i2].service_type,
					header.result.st_len,
					&tx_len);
				if(tx==NULL){
					NyLPC_OnErrorGoto(ERROR1);
				}
				if(!NyLPC_iUdpSocket_psend(i_sock,&i_info->peer_ip,i_info->peer_port,tx,tx_len)){
					NyLPC_OnErrorGoto(ERROR2);
				}
			}
		}
	}else if(strncmp("uuid:",header.result.st_str,5)==0){
		//UDNの一致するデバイスの送信
		NyLPC_TInt16 i;
		for(i=inst->number_of_device-1;i>=0;i--){
			if(strncmp(header.result.st_str,inst->ref_device_record[i]->udn,header.result.st_len)==0){
				//UDN一致
				tx=allocMsearchResponeTx(
					inst,header.result.st_str,
					inst->ref_device_record[i]->udn,NULL,
					header.result.st_len,
					&tx_len);
				if(tx==NULL){
					NyLPC_OnErrorGoto(ERROR1);
				}
				if(!NyLPC_iUdpSocket_psend(i_sock,&i_info->peer_ip,i_info->peer_port,tx,tx_len)){
					NyLPC_OnErrorGoto(ERROR2);
				}
				break;//送信処理終了
			}
		}
	}else if(strncmp(STR_UPNP_ROOT_DEVICE,header.result.st_str,15)==0){
		//rootDeviceはSTR_UPNP_ROOT_DEVICE
		tx=allocMsearchResponeTx(
			inst,header.result.st_str,
			inst->ref_device_record[0]->udn,STR_UPNP_ROOT_DEVICE,
			header.result.st_len,
			&tx_len);
		if(tx==NULL){
			NyLPC_OnErrorGoto(ERROR1);
		}
		if(!NyLPC_iUdpSocket_psend(i_sock,&i_info->peer_ip,i_info->peer_port,tx,tx_len)){
			NyLPC_OnErrorGoto(ERROR2);
		}
	}else if(strncmp("urn:",header.result.st_str,4)==0){
		for(i=0;i<inst->number_of_device;i++){
			//urn一致チェック
			if(strncmp(inst->ref_device_record[i]->device_type,header.result.st_str,header.result.st_len)==0){
				//deviceに一致
				tx=allocMsearchResponeTx(
					inst,header.result.st_str,
					inst->ref_device_record[i]->udn,inst->ref_device_record[i]->device_type,
					header.result.st_len,
					&tx_len);
				if(tx==NULL){
					NyLPC_OnErrorGoto(ERROR1);
				}
				if(!NyLPC_iUdpSocket_psend(i_sock,&i_info->peer_ip,i_info->peer_port,tx,tx_len)){
					NyLPC_OnErrorGoto(ERROR2);
				}
				continue;
			}
			for(i2=0;i2<inst->ref_device_record[i]->number_of_service;i2++){
				if(strncmp(inst->ref_device_record[i]->services[i2].service_type,header.result.st_str,header.result.st_len)==0){
					//serviceに一致
					tx=allocMsearchResponeTx(
						inst,header.result.st_str,
						inst->ref_device_record[i]->udn,inst->ref_device_record[i]->services[i2].service_type,
						header.result.st_len,
						&tx_len);
					if(tx==NULL){
						NyLPC_OnErrorGoto(ERROR1);
					}
					if(!NyLPC_iUdpSocket_psend(i_sock,&i_info->peer_ip,i_info->peer_port,tx,tx_len)){
						NyLPC_OnErrorGoto(ERROR2);
					}
				}
			}
		}
	}
	//正常終了
	return NyLPC_TBool_FALSE;
ERROR2:
	NyLPC_iUdpSocket_releaseSendBuf(i_sock,tx);
ERROR1:
	return NyLPC_TBool_FALSE;
}

#define SSDP_NOTIFY_INTERVAL 150*1000	//300*0.5*1000
#define FLAG_ORDER_START_SERVICE	0
#define FLAG_ORDER_STOP_SERVICE		1
#define FLAG_IS_SERVICE_RUNNING		2

static void onPeriodic(NyLPC_TiUdpSocket_t* i_sock)
{
	NyLPC_TcSsdpSocket_t* inst=(NyLPC_TcSsdpSocket_t*)i_sock->_tag;
	if(NyLPC_TUInt8_isBitOn(inst->_flags,FLAG_IS_SERVICE_RUNNING)){
	//実行中
		//停止要求着てる？
		if(NyLPC_TUInt8_isBitOn(inst->_flags,FLAG_ORDER_STOP_SERVICE))
		{
			//状態変更
			NyLPC_TUInt8_unsetBit(inst->_flags,FLAG_IS_SERVICE_RUNNING);
			//要求フラグクリア
			NyLPC_TUInt8_unsetBit(inst->_flags,FLAG_ORDER_STOP_SERVICE);
			//@bug ByeBye送信しろ
		}else if(NyLPC_cStopwatch_isExpired(&inst->_periodic_sw)){
			//Notify送信
			NyLPC_cSsdpSocket_notify(inst);
			//タイマ再始動
			NyLPC_cStopwatch_startExpire(&inst->_periodic_sw,SSDP_NOTIFY_INTERVAL);
		}
	}else{
	//停止中
		//開始要求着てる？
		if(NyLPC_TUInt8_isBitOn(inst->_flags,FLAG_ORDER_START_SERVICE))
		{
			//状態変更
			NyLPC_TUInt8_setBit(inst->_flags,FLAG_IS_SERVICE_RUNNING);
			//要求フラグクリア
			NyLPC_TUInt8_unsetBit(inst->_flags,FLAG_ORDER_START_SERVICE);
			//次回expireするように
			NyLPC_cStopwatch_startExpire(&inst->_periodic_sw,SSDP_NOTIFY_INTERVAL);
		}
	}
}

/**
 * デバイスツリーを展開する。
 */
static void expandDeviceTree(NyLPC_TcSsdpSocket_t* i_inst,const struct NyLPC_TUPnPDevDescDevice* i_dev)
{
	NyLPC_TUInt16 i;
	if(i_inst->number_of_device>=NyLPC_TcSsdpSocket_MAX_DEVICES){
		NyLPC_Warning();//
	}
	i_inst->ref_device_record[i_inst->number_of_device]=i_dev;
	i_inst->number_of_device++;
	for(i=0;i<i_dev->number_of_devices;i++){
		expandDeviceTree(i_inst,i_dev->devices[i]);
	}
	return;
}

void NyLPC_cSsdpSocket_initialize(
		NyLPC_TcSsdpSocket_t* i_inst,
		const struct NyLPC_TUPnPDevDescDevice* i_ref_dev_record,
		NyLPC_TUInt16 i_server_port,const NyLPC_TChar* i_ref_location_path)
{
	i_inst->_socket=NyLPC_cNet_createUdpSocketEx(1900,NyLPC_TSocketType_UDP_NOBUF);
    i_inst->_socket->_tag=i_inst;

	NyLPC_iUdpSocket_setOnRxHandler(i_inst->_socket,onPacket);
	NyLPC_iUdpSocket_setOnPeriodicHandler(i_inst->_socket,onPeriodic);

	NyLPC_iUdpSocket_joinMulticast(i_inst->_socket,&SSDP_MCAST_IPADDR);
	i_inst->_flags=0;
	NyLPC_cStopwatch_initialize(&(i_inst->_periodic_sw));
	i_inst->number_of_device=0;
	expandDeviceTree(i_inst,i_ref_dev_record);
	i_inst->location_port=i_server_port;
	i_inst->location_path=i_ref_location_path;
}
void NyLPC_cSsdpSocket_finalize(NyLPC_TcSsdpSocket_t* i_inst)
{
	NyLPC_cStopwatch_finalize(&(i_inst->_periodic_sw));
	NyLPC_iUdpSocket_finalize(i_inst->_socket);
}

void NyLPC_cSsdpSocket_start(NyLPC_TcSsdpSocket_t* i_inst)
{
	//Notifyを3回送信
	NyLPC_TInt16 i;
	NyLPC_cSsdpSocket_notify(i_inst);
	for(i=0;i<2;i++){
		NyLPC_cThread_sleep(800);
		NyLPC_cSsdpSocket_notify(i_inst);
	}

	//ストップウォッチの開始要求
	NyLPC_TUInt8_setBit(i_inst->_flags,FLAG_ORDER_START_SERVICE);
	do{
		NyLPC_cThread_sleep(10);
		//開始フラグがクリアされるまでループ
	}while(NyLPC_TUInt8_isBitOn(i_inst->_flags,FLAG_ORDER_START_SERVICE));
}
void NyLPC_cSsdpSocket_stop(NyLPC_TcSsdpSocket_t* i_inst)
{
	//今は使えない。
	NyLPC_Abort();
	NyLPC_TUInt8_setBit(i_inst->_flags,FLAG_ORDER_STOP_SERVICE);
	do{
		NyLPC_cThread_sleep(10);
		//開始フラグがクリアされるまでループ
	}while(NyLPC_TUInt8_isBitOn(i_inst->_flags,FLAG_ORDER_STOP_SERVICE));
}
void NyLPC_cSsdpSocket_notify(NyLPC_TcSsdpSocket_t* i_inst)
{
	void* tx;
	NyLPC_TInt16 tx_len;
	NyLPC_TUInt8 i,i2;
	//rootdevice
	tx=allocNotifyTx(
		i_inst,
		i_inst->ref_device_record[0]->udn,STR_UPNP_ROOT_DEVICE,
		&tx_len);
	if(tx==NULL){
		NyLPC_OnErrorGoto(ERROR1);
	}
	if(!NyLPC_iUdpSocket_psend(i_inst->_socket,&SSDP_MCAST_IPADDR,1900,tx,tx_len)){
		NyLPC_OnErrorGoto(ERROR2);
	}
	//all device
	for(i=0;i<i_inst->number_of_device;i++){
		//uuid
		tx=allocNotifyTx(
			i_inst,
			i_inst->ref_device_record[i]->udn,NULL,
			&tx_len);
		if(tx==NULL){
			NyLPC_OnErrorGoto(ERROR1);
		}
		if(!NyLPC_iUdpSocket_psend(i_inst->_socket,&SSDP_MCAST_IPADDR,1900,tx,tx_len)){
			NyLPC_OnErrorGoto(ERROR2);
		}
		//devicatype
		tx=allocNotifyTx(
			i_inst,
			i_inst->ref_device_record[i]->udn,i_inst->ref_device_record[i]->device_type,
			&tx_len);
		if(tx==NULL){
			NyLPC_OnErrorGoto(ERROR1);
		}
		if(!NyLPC_iUdpSocket_psend(i_inst->_socket,&SSDP_MCAST_IPADDR,1900,tx,tx_len)){
			NyLPC_OnErrorGoto(ERROR2);
		}
		for(i2=0;i2<i_inst->ref_device_record[i]->number_of_service;i2++){
			tx=allocNotifyTx(
				i_inst,
				i_inst->ref_device_record[i]->udn,i_inst->ref_device_record[i]->services[i2].service_type,
				&tx_len);
			if(tx==NULL){
				NyLPC_OnErrorGoto(ERROR1);
			}
			if(!NyLPC_iUdpSocket_psend(i_inst->_socket,&SSDP_MCAST_IPADDR,1900,tx,tx_len)){
				NyLPC_OnErrorGoto(ERROR2);
			}
		}
	}
	return;
ERROR2:
	NyLPC_iUdpSocket_releaseSendBuf(i_inst->_socket,tx);
ERROR1:
	return;
}

