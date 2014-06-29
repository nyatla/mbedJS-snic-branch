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
 *  http://nyatla.jp/
 *  <airmail(at)ebony.plala.or.jp> or <nyatla(at)nyatla.jp>
 *
 *********************************************************************************/

#ifndef NYLPC_CTCPSOCKET_PROTECTED_H_
#define NYLPC_CTCPSOCKET_PROTECTED_H_
#include "NyLPC_cIPv4.h"
#include "NyLPC_cTcpSocket.h"
#include "NyLPC_cIPv4Config.h"
#include "NyLPC_cIPv4Payload_protected.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************
 * ステータス値の定義
 **********************************************************************/
#define UIP_CLOSED       0
#define UIP_SYN_RCVD     1
#define UIP_SYN_SENT     2
#define UIP_ESTABLISHED  3
#define UIP_FIN_WAIT_1   4
#define UIP_FIN_WAIT_2   5
#define UIP_CLOSING      6
#define UIP_TIME_WAIT    7
#define UIP_CLOSE_WAIT   8
#define UIP_LAST_ACK     9








/**
 * パース結果をもとに、ソケットのuipconnectionを初期化します。
 * この関数は、cUipServiceからのみコールできます。
 */
void NyLPC_cTcpSocket_initConnection(NyLPC_TcTcpSocket_t* i_inst,const NyLPC_TcIPv4Config_t* i_config,const NyLPC_TcIPv4Payload_t* i_ipp);

/**
 * TCPペイロードを処理して、応答パケットをペイロードに返します。
 * uipサービスタスクが実行する関数です。
 * @return
 * 応答パケットを格納したメモリブロックを返します。
 * このメモリは、NyLPC_cUipService_allocSysTxBuf関数で確保されたメモリです。
 */
void* NyLPC_cTcpSocket_parseRx(
    NyLPC_TcTcpSocket_t* i_inst,
    const NyLPC_TcIPv4Payload_t* i_ipp);

/**
 * 定期的に実行する関数。最低でも1s単位で実行してください。
 * uipサービスタスクが実行する関数です。
 */
void NyLPC_cTcpSocket_periodic(
    NyLPC_TcTcpSocket_t* i_inst);

/**
 * CLOSEステータスのソケットを、SYN情報の内容でSYNRECV状態にします。
 * この関数は、NyLPC_TcTcpListenerクラスからコールされます。
 * @return
 * 遷移に成功すると、TRUEを返します。
 */
NyLPC_TBool NyLPC_cTcpSocket_listenSyn(NyLPC_TcTcpSocket_t* i_inst,const struct NyLPC_TTcpSocketSynParam* i_lq,NyLPC_TUInt16 i_lport);


/**
 * uipサービスタスクが実行する関数です。
 * サービスの開始を通知します。
 * この関数は他のAPIが非同期に実行されないことが保証される状況で使用する必要があります。
 */
void NyLPC_cTcpSocket_startService(NyLPC_TcTcpSocket_t* i_inst,const NyLPC_TcIPv4Config_t* i_config);

/**
 * uipサービスタスクが実行する関数です。
 * サービスの停止を通知します。
 * この関数は他のAPIが非同期に実行されないことが保証される状況で使用する必要があります。
 */
void NyLPC_cTcpSocket_stopService(NyLPC_TcTcpSocket_t* i_inst);


void* NyLPC_cTcpSocket_allocTcpReverseRstAck(
    const NyLPC_TcIPv4Payload_t* i_src);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CTCPSOCKET_PROTECTED_H_ */
