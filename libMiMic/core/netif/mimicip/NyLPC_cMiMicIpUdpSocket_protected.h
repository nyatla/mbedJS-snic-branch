/*
 * NyLPC_cUdpSocket.h
 *
 *  Created on: 2013/05/20
 *      Author: nyatla
 */

#ifndef NYLPC_CUDPSOCKET_PROTECTED_H_
#define NYLPC_CUDPSOCKET_PROTECTED_H_
#include "NyLPC_cMiMicIpUdpSocket.h"
#include "NyLPC_cIPv4Payload.h"
#include "../NyLPC_cIPv4Config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */





/**
 * この関数は、rxパケットを処理して、ソケットの状態を更新します。
 * uipサービスタスクが実行する関数です。
 */
NyLPC_TBool NyLPC_cMiMicIpUdpSocket_parseRx(
    NyLPC_TcMiMicIpUdpSocket_t* i_inst,
    const NyLPC_TcIPv4Payload_t* i_ipp);

/**
 * uipサービスタスクが実行する関数です。
 * サービスの開始を通知します。
 * この関数は他のAPIが非同期に実行されないことが保証される状況で使用する必要があります。
 */
void NyLPC_cMiMicIpUdpSocket_startService(NyLPC_TcMiMicIpUdpSocket_t* i_inst,const NyLPC_TcIPv4Config_t* i_config);

/**
 * uipサービスタスクが実行する関数です。
 * サービスの停止を通知します。
 * この関数は他のAPIが非同期に実行されないことが保証される状況で使用する必要があります。
 */
void NyLPC_cMiMicIpUdpSocket_stopService(NyLPC_TcMiMicIpUdpSocket_t* i_inst);


/**
 * 定期的に実行する関数。最低でも1s単位で実行してください。
 * uipサービスタスクが実行する関数です。
 */
#define NyLPC_cMiMicIpUdpSocket_periodic(i_inst) if((i_inst)->as_handler.periodic!=NULL){(i_inst)->as_handler.periodic((NyLPC_TiUdpSocket_t*)(i_inst));}
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* NYLPC_CUDPSOCKET_H_ */
