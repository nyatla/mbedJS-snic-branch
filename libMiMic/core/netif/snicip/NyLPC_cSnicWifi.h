/*
 * NyLPC_cSnicGeneral.h
 *
 *  Created on: 2014/11/23
 *      Author: nyatla
 */

#ifndef NYLPC_CSNICWIFI_H_
#define NYLPC_CSNICWIFI_H_

#include "nyLPC_stdlib.h"
#include "NyLPC_Snic_types.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct NyLPC_TcSnicWifi NyLPC_TcSnicWifi_t;

struct NyLPC_TcSnicWifi
{
	volatile NyLPC_TUInt8 _signal;		//シグナルの代用品
	NyLPC_TUInt8 last_cmd;				//最後に送信したコマンド
	NyLPC_TUInt8 last_status;			//最後に実行したコマンドのステータス
	NyLPC_TUInt8 last_seq;
	/** 関数との通信用*/
};


NyLPC_TBool NyLPC_cSnicWifi_initialize(NyLPC_TcSnicWifi_t* i_inst);
void NyLPC_cSnicWifi_finalize(NyLPC_TcSnicWifi_t* i_inst);
NyLPC_TBool NyLPC_cSnicWifi_on(NyLPC_TcSnicWifi_t* i_inst,NyLPC_Snic_TCuntryCode i_country_code);
NyLPC_TBool NyLPC_cSnicWifi_off(NyLPC_TcSnicWifi_t* i_inst);
NyLPC_TBool NyLPC_cSnicWifi_join(NyLPC_TcSnicWifi_t* i_inst,const void* i_ssid,NyLPC_TUInt16 i_ssid_len,NyLPC_TUInt8 i_smode,const void* i_seq_key,NyLPC_TUInt16 i_seq_key_len);
NyLPC_TBool NyLPC_cSnicWifi_disconnect(NyLPC_TcSnicWifi_t* i_inst);
#ifdef __cplusplus
}
#endif





#endif /* NYLPC_CSNICWIFI_H_ */
