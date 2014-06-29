/*
 * NyLPC_cUdpSocket.h
 *
 *  Created on: 2013/05/20
 *      Author: nyatla
 */

#ifndef NYLPC_CUDPSOCKET_PROTECTED_H_
#define NYLPC_CUDPSOCKET_PROTECTED_H_
#include "NyLPC_cUdpSocket.h"
#include "NyLPC_cIPv4Payload.h"
#include "NyLPC_cIPv4Config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * この関数は、rxパケットを処理して、ソケットの状態を更新します。
 * uipサービスタスクが実行する関数です。
 */
NyLPC_TBool NyLPC_cUdpSocket_parseRx(
    NyLPC_TcUdpSocket_t* i_inst,
    const NyLPC_TcIPv4Payload_t* i_ipp);

/**
 * uipサービスタスクが実行する関数です。
 * サービスの開始を通知します。
 * この関数は他のAPIが非同期に実行されないことが保証される状況で使用する必要があります。
 */
void NyLPC_cUdpSocket_startService(NyLPC_TcUdpSocket_t* i_inst,const NyLPC_TcIPv4Config_t* i_config);

/**
 * uipサービスタスクが実行する関数です。
 * サービスの停止を通知します。
 * この関数は他のAPIが非同期に実行されないことが保証される状況で使用する必要があります。
 */
void NyLPC_cUdpSocket_stopService(NyLPC_TcUdpSocket_t* i_inst);


/**
 * 定期的に実行する関数。最低でも1s単位で実行してください。
 * uipサービスタスクが実行する関数です。
 */
#define NyLPC_cUdpSocket_periodic(i_inst) if((i_inst)->as_handler.periodic!=NULL){(i_inst)->as_handler.periodic(i_inst);}

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* NYLPC_CUDPSOCKET_H_ */
