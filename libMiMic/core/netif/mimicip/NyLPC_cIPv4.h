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
 *
 * Parts of this file were leveraged from uIP:
 *
 * Copyright (c) 2001-2003, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * @file
 * このファイルは、NyLPC_cIPv4IPv4クラスを定義します。
 */
#ifndef NYLPC_CIPV4TCP_H_
#define NYLPC_CIPV4TCP_H_



#include "NyLPC_os.h"
#include "../NyLPC_NetIf_ip_types.h"
#include "../NyLPC_cIPv4Config.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**********************************************************************
 *
 * Closs reference
 *
 **********************************************************************/

typedef struct NyLPC_TcMiMicIpBaseSocket NyLPC_TcMiMicIpBaseSocket_t;


/**********************************************************************
 *
 * class NyLPC_TcIPv4
 *
 **********************************************************************/
typedef struct NyLPC_TcIPv4 NyLPC_TcIPv4_t;




/**
 * Socketの最大生成数
 * この値は、NyLPC_cTcpSocketクラス,NyLPC_cTcpListener,NyLPC_cUdpの最大生成数になります。
 */
#define NyLPC_cIPv4_MAX_SOCKET 10



/**
 * NyLPC_TcIPv4クラスの構造体です。
 */
struct NyLPC_TcIPv4
{
    /** 参照しているIPスタックの環境値です。この値は、start関数が設定します。*/
    const NyLPC_TcIPv4Config_t* _ref_config;
    /** ソケットリソースの保護用。コールバック関数から呼び出されるソケット内部のリソース保護に使用する共通MUTEX*/
    NyLPC_TcMutex_t _sock_mutex;
    /** リスナリソースの保護用。コールバック関数から呼び出されるソケット内部のリソース保護に使用する共通MUTEX*/
    NyLPC_TcMutex_t _listener_mutex;
    /** NyLPC_cTcpSocketを管理するポインタリストです。*/
    NyLPC_TcPtrTbl_t _socket_tbl;
    /** _socket_tblが使用するメモリ領域です。*/
    NyLPC_TcMiMicIpBaseSocket_t* _socket_array_buf[NyLPC_cIPv4_MAX_SOCKET];
    /** 0-0xfffまでを巡回するカウンタ*/
    NyLPC_TUInt16 tcp_port_counter;
};

/**
 * コンストラクタです。インスタンスを初期化します。
 * @param i_inst
 * 初期化するインスタンス
 */
void NyLPC_cIPv4_initialize(
    NyLPC_TcIPv4_t* i_inst);

/**
 * デストラクタです。インスタンスを破棄して、確保している動的リソースを元に戻します。
 * @param i_inst
 * 破棄するインスタンス
 * initializeが成功したインスタンスだけが指定できます。
 */
void NyLPC_cIPv4_finalize(
    NyLPC_TcIPv4_t* i_inst);

/**
 * この関数は、インスタンスにTCP/IP処理の準備をするように伝えます。
 * @param i_inst
 * 操作するインスタンス
 * @param i_ref_configlation
 * IPの環境値をセットしたオブジェクトを指定します。
 * この値は、stop関数を実行するまでの間、維持してください。
 */
void NyLPC_cIPv4_start(
    NyLPC_TcIPv4_t* i_inst,
    const NyLPC_TcIPv4Config_t* i_ref_configlation);

/**
 * この関数はTCP/IP処理を停止することを伝えます。
 * @param i_inst
 * 操作するインスタンス。
 * startで開始済みで無ければなりません。
 * @note
 * 現在、接続中の接続に対する保障は未実装です。安全に使用することが出来ません。
 */
void NyLPC_cIPv4_stop(
    NyLPC_TcIPv4_t* i_inst);

/**
 * この関数は、NyLPC_TcBaseSocketオブジェクトを管理リストへ追加します。
 * @param i_inst
 * 操作するインスタンス。
 * @param i_sock
 * 追加するインスタンスのポインタ
 * @return
 * 追加が成功するとTRUEを返します。
 */
NyLPC_TBool NyLPC_cIPv4_addSocket(
    NyLPC_TcIPv4_t* i_inst,
    NyLPC_TcMiMicIpBaseSocket_t* i_sock);

/**
 * この関数は、NyLPC_cTcpSocketオブジェクトを管理リストから除外します。
 * NyLPC_TcBaseSocketが使います。
 * @param i_inst
 * 操作するインスタンス。
 * @param i_sock
 * 削除するインスタンスのポインタ
 * @return
 * 削除が成功するとTRUEを返します。
 */
NyLPC_TBool NyLPC_cIPv4_removeSocket(
    NyLPC_TcIPv4_t* i_inst,
    NyLPC_TcMiMicIpBaseSocket_t* i_sock);


/**
 * この関数は、RxIPパケットを処理して、管理下のインスタンスに処理を依頼します。
 * 現在の関数は、i_rxに最大64バイトの応答パケットのイメージを格納することがあります。
 * 応答パケットは、RXに対するACKパケットです。
 * 格納の有無は戻り値を確認することで判ります。
 * この関数はstart-stopの間だけコールすることが出来ます。start,stopと非同期に実行しないでください。
 * @param i_inst
 * 操作するインスタンスです。
 * @param i_rx
 * RXパケットを格納したメモリアドレスです。
 * 最低でも、64バイト以上のサイズが必要です。
 * @param i_rx_size
 * i_rxに格納したデータのサイズです。
 * @return
 * 応答パケットを格納したメモリです。
 */
void* NyLPC_cIPv4_rx(NyLPC_TcIPv4_t* i_inst,const void* i_rx,NyLPC_TUInt16 i_rx_size);
/**
 * この関数は、定期的にインスタンスへ実行機会を与える関数です。
 * TCPの再送、無通信タイムアウトなどを処理します。
 * 約1秒おきに呼び出してください。
 * @param i_inst
 * 操作するインスタンスです。
 */
void NyLPC_cIPv4_periodec(NyLPC_TcIPv4_t* i_inst);

/**
 * ソケットリソースとコールバックの排他処理に使う共通MUTEXを返します。
 * このMutexはソケット同士の干渉が起こらない処理にだけ使ってください。
 */
#define NyLPC_cIPv4_getSockMutex(i_inst) (&((i_inst)->_sock_mutex))
/**
 * リスナーリソースとコールバックの排他処理に使う共通MUTEXを返します。
 */
#define NyLPC_cIPv4_getListenerMutex(i_inst) (&((i_inst)->_listener_mutex))

/**
 * ポート0で使用するポート番号を返します。
 * @return
 * 49152 - (49152+0x0ffff)番までのポートのうち、使用中でないポート番号を返します。
 * エラー時は0です。
 */
NyLPC_TUInt16 NyLPC_cIPv4_getNewPortNumber(NyLPC_TcIPv4_t* i_inst);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
