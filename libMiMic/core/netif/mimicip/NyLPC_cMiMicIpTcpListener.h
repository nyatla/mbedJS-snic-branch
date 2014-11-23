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
#ifndef NYLPC_CTCPLISTENER_H_
#define NYLPC_CTCPLISTENER_H_


#include "NyLPC_stdlib.h"
#include "NyLPC_os.h"
#include "NyLPC_cMiMicIpBaseSocket.h"
#include "NyLPC_cMiMicIpTcpSocket.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NyLPC_TcMiMicIpTcpListener NyLPC_TcMiMicIpTcpListener_t;

/**********************************************************************
 *
 * NyLPC_TTcpListenerListenQ struct
 *
 **********************************************************************/

#define NyLPC_TcMiMicIpTcpListener_NUMBER_OF_Q 10

struct NyLPC_TTcpListenerListenQ
{
    struct NyLPC_TTcpSocketSynParam item[NyLPC_TcMiMicIpTcpListener_NUMBER_OF_Q];
    NyLPC_TUInt16 wp;
};


/**********************************************************************
 *
 * NyLPC_TcTcpListener class
 *
 **********************************************************************/

/**
 * TCP listenerクラス型です。
 */
struct NyLPC_TcMiMicIpTcpListener
{
    NyLPC_TcMiMicIpBaseSocket_t _super;
    NyLPC_TUInt16 _port;                /**<ネットワークオーダーのポート番号*/
//  /**
//   * タスク間の調停用Mutex
//   * Listener用の共通Mutexポインタ
//   */
//  NyLPC_TcMutex_t* _mutex;
    /**
     * SYNパケットのキュー
     */
    struct NyLPC_TTcpListenerListenQ _listen_q;
};
/**
 * この関数は、TCPのリスナーを初期化します。
 * 初期化したリスナーをサービスに登録することにより、listen関数を使用できるようになります。
 * サービスへの登録は、NyLPC_cUipService_addListenerを使います。
 * @param i_port
 * ポート番号。host orderです。
 */
NyLPC_TBool NyLPC_cMiMicIpTcpListener_initialize(NyLPC_TcMiMicIpTcpListener_t* i_inst,NyLPC_TUInt16 i_port);

/**
 * この関数は、一定時間i_sockに接続を受け付けます。
 */
NyLPC_TBool NyLPC_cMiMicIpTcpListener_listen(NyLPC_TcMiMicIpTcpListener_t* i_inst,NyLPC_TcMiMicIpTcpSocket_t* i_sock,NyLPC_TUInt32 i_wait_msec);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CTCPLISTENER_H_ */
