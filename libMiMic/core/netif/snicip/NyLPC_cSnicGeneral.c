/*
 * NyLPC_cSnicGeneralCmdProc.c
 *
 *  Created on: 2014/11/22
 *      Author: nyatla
 */
#include "NyLPC_cSnicNetIf_protected.h"
#include "NyLPC_cSnicGeneral.h"
#include "NyLPC_iSnicDevice.h"
#include "NyLPC_Snic_types.h"
#include "NyLPC_os.h"


#define UART_CMD_SID_RESPONSE_BIT_MASK	0x80
#define UART_CMD_SID_RESPONSE_CMD_MASK	(~UART_CMD_SID_RESPONSE_BIT_MASK)

#define UART_CMD_SID_GEN_PWR_UP_IND			0x00   //Power up indication
#define UART_CMD_SID_GEN_FW_VER_GET_REQ		0x08   //Get firmware version string
#define UART_CMD_SID_GEN_RESTORE_REQ		0x09


#define UART_CMD_RES_GEN_SUCCESS           0x00
#define UART_CMD_RES_GEN_FAIL              0x01


NyLPC_TBool NyLPC_cSnicGeneral_initialize(NyLPC_TcSnicGeneral_t* i_inst)
{
	return NyLPC_TBool_TRUE;
}
void NyLPC_cSnicGeneral_finalize(NyLPC_TcSnicGeneral_t* i_inst)
{

}


NyLPC_TBool NyLPC_cSnicGeneral_getFirmwareVersionInfo(NyLPC_TcSnicGeneral_t* i_inst,NyLPC_TChar* i_buf,NyLPC_TUInt8 i_buf_len)
{
	NyLPC_TUInt8 data[3];
	i_inst->last_cmd=UART_CMD_SID_GEN_FW_VER_GET_REQ;
	i_inst->_response.get_firmware_version_info.buf=i_buf;
	i_inst->_response.get_firmware_version_info.buf_len=i_buf_len;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	i_inst->_signal=NyLPC_TUInt8_FALSE;
	NyLPC_cSnicNetIf_sendShortPayload(data,SNIC_UART_CMD_ID_GEN,2);
	//シグナル待ち
	do{
		NyLPC_cThread_yield();
	}while(!i_inst->_signal);
	NyLPC_cSnicNetIf_unregieterObject(i_inst);
	return i_inst->last_status==UART_CMD_RES_GEN_SUCCESS;
}



/**
 * FALSEを返却したら次のSOMまでまつよ
 */
NyLPC_TBool NyLPC_cSnicGeneral_rxHandler(struct NyLPC_TiSnicDevice_Interface* i_dev)
{
	NyLPC_TcSnicGeneral_t* inst;
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
	case (UART_CMD_SID_GEN_FW_VER_GET_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
		if(NyLPC_cSnicNetIf_readPayload(buf,1)){
			if(inst->last_status==UART_CMD_RES_GEN_SUCCESS){
				//サイズチェック
				if(buf[0]<inst->_response.get_firmware_version_info.buf_len-1){
					inst->_response.get_firmware_version_info.buf_len=buf[0];
					inst->_response.get_firmware_version_info.buf[buf[0]-1]=0;
					NyLPC_cSnicNetIf_readPayload(inst->_response.get_firmware_version_info.buf,buf[0]);
					inst->_signal=NyLPC_TUInt8_TRUE;//シグナル設定
					NyLPC_cSnicNetIf_unlockObject(inst);
					return NyLPC_TBool_TRUE;
				}else{
					//メモリ不足だからデータ捨てる
					NyLPC_cSnicNetIf_seekPayload(buf[0]);
					inst->_signal=NyLPC_TUInt8_TRUE;//シグナル設定
					NyLPC_cSnicNetIf_unlockObject(inst);
					return NyLPC_TBool_TRUE;
				}
			}
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
