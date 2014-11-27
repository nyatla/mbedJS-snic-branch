/*
 * NyLPC_cSnicGeneral.h
 *
 *  Created on: 2014/11/23
 *      Author: nyatla
 */

#ifndef NYLPC_CSNICGENERAL_H_
#define NYLPC_CSNICGENERAL_H_

#include "nyLPC_stdlib.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct NyLPC_TcSnicGeneral NyLPC_TcSnicGeneral_t;

struct NyLPC_TcSnicGeneral
{
	volatile NyLPC_TUInt8 _signal;		//シグナルの代用品
	NyLPC_TUInt8 last_cmd;				//最後に送信したコマンド
	NyLPC_TUInt8 last_status;			//最後に実行したコマンドのステータス
	NyLPC_TUInt8 last_seq;
	/** 関数との通信用*/
	union{
		struct{
			NyLPC_TUInt16 buf_len;
			NyLPC_TChar* buf;
		}get_firmware_version_info;
	}_response;
};


NyLPC_TBool NyLPC_cSnicGeneral_initialize(NyLPC_TcSnicGeneral_t* i_inst);
void NyLPC_cSnicGeneral_finalize(NyLPC_TcSnicGeneral_t* i_inst);
NyLPC_TBool NyLPC_cSnicGeneral_getFirmwareVersionInfo(NyLPC_TcSnicGeneral_t* i_inst,NyLPC_TChar* i_buf,NyLPC_TUInt8 i_buf_len);

#ifdef __cplusplus
}
#endif
#endif /* NYLPC_CSNICGENERAL_H_ */
