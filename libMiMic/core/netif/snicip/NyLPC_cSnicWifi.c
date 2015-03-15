/*
 * NyLPC_cSnicGeneralCmdProc.c
 *
 *  Created on: 2014/11/22
 *      Author: nyatla
 */
#include "NyLPC_cSnicNetIf_protected.h"
#include "NyLPC_iSnicDevice.h"
#include "NyLPC_cSnicWifi.h"

#include "NyLPC_os.h"

#define UART_CMD_SID_RESPONSE_BIT_MASK	0x80
#define UART_CMD_SID_RESPONSE_CMD_MASK	(~UART_CMD_SID_RESPONSE_BIT_MASK)


#define UART_CMD_SID_WIFI_ON_REQ             0x00   // Turn on Wifi
#define UART_CMD_SID_WIFI_OFF_REQ            0x01   // Turn off Wifi
#define UART_CMD_SID_WIFI_JOIN_REQ           0x02   // Associate to a network
#define UART_CMD_SID_WIFI_DISCONNECT_REQ     0x03   // Disconnect from a network
#define UART_CMD_SID_WIFI_GET_STATUS_REQ     0x04   // Get WiFi status
#define UART_CMD_SID_WIFI_SCAN_REQ           0x05   // Scan WiFi networks
#define UART_CMD_SID_WIFI_GET_STA_RSSI_REQ   0x06   // Get STA signal strength (RSSI)
#define UART_CMD_SID_WIFI_AP_CTRL_REQ        0x07   // Soft AP on-off control
#define UART_CMD_SID_WIFI_WPS_REQ            0x08   // Start WPS process
#define UART_CMD_SID_WIFI_AP_GET_CLIENT_REQ  0x0A   // Get clients that are associated to the soft AP.
#define UART_CMD_SID_WIFI_NETWORK_STATUS_IND 0x10   // Network status indication
#define UART_CMD_SID_WIFI_SCAN_RESULT_IND    0x11   // Scan result indication

#define UART_CMD_RES_WIFI_SUCCESS               0x00
#define UART_CMD_RES_WIFI_ERR_UNKNOWN_COUNTRY   0x01
#define UART_CMD_RES_WIFI_ERR_INIT_FAIL         0x02
#define UART_CMD_RES_WIFI_ERR_ALREADY_JOINED    0x03
#define UART_CMD_RES_WIFI_ERR_AUTH_TYPE         0x04
#define UART_CMD_RES_WIFI_ERR_JOIN_FAIL         0x05
#define UART_CMD_RES_WIFI_ERR_NOT_JOINED        0x06
#define UART_CMD_RES_WIFI_ERR_LEAVE_FAILED      0x07
#define UART_CMD_RES_WIFI_COMMAND_PENDING       0x08
#define UART_CMD_RES_WIFI_WPS_NO_CONFIG         0x09
#define UART_CMD_RES_WIFI_NETWORK_UP            0x10
#define UART_CMD_RES_WIFI_NETWORK_DOWN          0x11
#define UART_CMD_RES_WIFI_FAIL                  0xFF

static void sendCmd(NyLPC_TcSnicWifi_t* i_inst,NyLPC_TUInt8* i_data,NyLPC_TUInt16 i_data_len)
{
	i_inst->_signal=NyLPC_TUInt8_FALSE;
	NyLPC_cSnicNetIf_sendShortPayload(i_data,SNIC_UART_CMD_ID_WIFI,i_data_len);
	//シグナル待ち
	do{
		NyLPC_cThread_yield();
	}while(!i_inst->_signal);
	NyLPC_cSnicNetIf_unregieterObject(i_inst);
}

NyLPC_TBool NyLPC_cSnicWifi_initialize(NyLPC_TcSnicWifi_t* i_inst)
{
	return NyLPC_TBool_TRUE;
}
void NyLPC_cSnicWifi_finalize(NyLPC_TcSnicWifi_t* i_inst)
{
}

NyLPC_TBool NyLPC_cSnicWifi_on(NyLPC_TcSnicWifi_t* i_inst,NyLPC_Snic_TCuntryCode i_country_code)
{
	NyLPC_TUInt8 data[4];
	i_inst->last_cmd=UART_CMD_SID_WIFI_ON_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	data[2]=i_country_code[0];
	data[3]=i_country_code[1];
	sendCmd(i_inst,data,4);
	return i_inst->last_status==UART_CMD_RES_WIFI_SUCCESS;
}

NyLPC_TBool NyLPC_cSnicWifi_off(NyLPC_TcSnicWifi_t* i_inst)
{
	NyLPC_TUInt8 data[4];
	i_inst->last_cmd=UART_CMD_SID_WIFI_OFF_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	i_inst->_signal=NyLPC_TUInt8_FALSE;
	sendCmd(i_inst,data,2);
	return i_inst->last_status==UART_CMD_RES_WIFI_SUCCESS;
}
NyLPC_TBool NyLPC_cSnicWifi_join(NyLPC_TcSnicWifi_t* i_inst,const void* i_ssid,NyLPC_TUInt16 i_ssid_len,NyLPC_TUInt8 i_smode,const void* i_seq_key,NyLPC_TUInt16 i_seq_key_len)
{
	NyLPC_TUInt16 l;
	NyLPC_TUInt8 data[5];
	i_inst->last_cmd=UART_CMD_SID_WIFI_JOIN_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	data[2]=0;
	data[3]=i_smode;
	data[4]=i_seq_key_len;
	i_inst->_signal=NyLPC_TUInt8_FALSE;
	//長さの先行計算
	l=	NyLPC_cSnicNetIf_getPayloadLength(data,5)+
		NyLPC_cSnicNetIf_getPayloadLength(i_ssid,i_ssid_len)+
		NyLPC_cSnicNetIf_getPayloadLength(i_seq_key,i_seq_key_len);
	NyLPC_cSnicNetIf_startPayload(SNIC_UART_CMD_ID_WIFI,l);
	NyLPC_cSnicNetIf_sendPayload(&data[0],2);
	NyLPC_cSnicNetIf_sendPayload(i_ssid,i_ssid_len);
	NyLPC_cSnicNetIf_sendPayload(&data[2],3);//terminate,mode
	NyLPC_cSnicNetIf_sendPayload(i_seq_key,i_seq_key_len);//terminate,mode
	NyLPC_cSnicNetIf_endPayload();
	//シグナル待ち
	do{
		NyLPC_cThread_yield();
	}while(!i_inst->_signal);
	NyLPC_cSnicNetIf_unregieterObject(i_inst);
	return i_inst->last_status==UART_CMD_RES_WIFI_SUCCESS;
}
NyLPC_TBool NyLPC_cSnicWifi_disconnect(NyLPC_TcSnicWifi_t* i_inst)
{
	NyLPC_TUInt8 data[4];
	i_inst->last_cmd=UART_CMD_SID_WIFI_DISCONNECT_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	i_inst->_signal=NyLPC_TUInt8_FALSE;
	NyLPC_cSnicNetIf_sendShortPayload(data,SNIC_UART_CMD_ID_WIFI,2);
	//シグナル待ち
	do{
		NyLPC_cThread_yield();
	}while(!i_inst->_signal);
	NyLPC_cSnicNetIf_unregieterObject(i_inst);
	return i_inst->last_status==UART_CMD_RES_WIFI_SUCCESS;
}
/**
 * FALSEを返却したら次のSOMまでまつよ
 */
NyLPC_TBool NyLPC_cSnicWifi_rxHandler(struct NyLPC_TiSnicDevice_Interface* i_dev)
{
	NyLPC_TcSnicWifi_t* inst;
	NyLPC_TUInt8 buf[3];//CMD,SEQ,RES
	if(!NyLPC_cSnicNetIf_readPayload(buf,3)){
		goto DROP;
	}
	//SEQ->INST変換
	inst=NyLPC_cSnicNetIf_lockObjectById(buf[1]);
	if(inst==NULL){
		goto DROP;
	}
	//SEQがインスタンスの要求したものに一致するか?
	if(inst->last_seq!=buf[1]){
		goto DROP_UNLOCK;
	}
	//最後に実行したコマンドのレスポンスか?
	if(inst->last_cmd!=(buf[0]&UART_CMD_SID_RESPONSE_CMD_MASK)){
		goto DROP_UNLOCK;
	}
	//コマンドコードの保存
	inst->last_status=buf[2];
	//リクエストタイプごとの処理
	switch(buf[0]){
	case (UART_CMD_SID_WIFI_ON_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
	case (UART_CMD_SID_WIFI_OFF_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
	case (UART_CMD_SID_WIFI_JOIN_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
	case (UART_CMD_SID_WIFI_DISCONNECT_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
		if(inst->last_status==UART_CMD_RES_WIFI_SUCCESS){
			inst->_signal=NyLPC_TUInt8_TRUE;//シグナル設定
			NyLPC_cSnicNetIf_unlockObject(inst);
			return NyLPC_TBool_TRUE;
		}
		goto DROP_UNLOCK;
	default:
		goto DROP_UNLOCK;
	}
DROP_UNLOCK:
	inst->_signal=NyLPC_TUInt8_TRUE;//シグナル設定
	NyLPC_cSnicNetIf_unlockObject(inst);
DROP:
	return NyLPC_TBool_FALSE;
}
