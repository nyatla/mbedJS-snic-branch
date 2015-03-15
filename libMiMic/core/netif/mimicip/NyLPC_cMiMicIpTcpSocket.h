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
#ifndef NYLPC_CMIMICIPTCPSOCKET_H_
#define NYLPC_CMIMICIPTCPSOCKET_H_



#include "NyLPC_os.h"
#include "../NyLPC_NetIf_ip_types.h"
#include "NyLPC_cIPv4Payload.h"
#include "../NyLPC_iTcpSocket.h"
#include "NyLPC_cIPv4.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct NyLPC_TcMiMicIpTcpSocket NyLPC_TcMiMicIpTcpSocket_t;


/**********************************************************************
 *
 * NyLPC_TTcpListenerSynInfo　struct
 *
 **********************************************************************/


struct NyLPC_TTcpSocketSynParam
{
    struct NyLPC_TIPv4Addr srcaddr;
    NyLPC_TUInt16 rport;
    NyLPC_TUInt16 mss;
    NyLPC_TUInt32 rcv_nxt32;
};


/**********************************************************************
 *
 * NyLPC_TcTcpSocket_TxQItem　struct
 *
 **********************************************************************/

/**
 * TXキューの数。この値は8未満にしてください。
 */
#define NyLPC_TcTcpSocket_NUMBER_OF_TXQ 4

struct NyLPC_TcTcpSocket_TxQItem
{
    //最終更新時刻
    NyLPC_TUInt32 tick_of_sent;
    //このパケットのRTO(秒間隔)
    NyLPC_TUInt32 rto32;
    void* packet;
    //パケットのACK番号。この番号を受信できれば、再送パケットは消去可能である。
    NyLPC_TUInt32 ackno;
};

/**********************************************************************
 *
 * uip_conn　struct
 *
 **********************************************************************/

struct uip_conn
{
    struct NyLPC_TIPv4Addr        ripaddr;  /**< The IP address of the remote host. */
    const struct NyLPC_TIPv4Addr* lipaddr;  /**< ローカルIP*/
    NyLPC_TUInt16                lport;     /**< The local TCP port, in network byte order. */
    NyLPC_TUInt16                rport;     /**< The local remote TCP port, in network byte order. */
    NyLPC_TUInt32 rcv_nxt32;    /**< The sequence number that we expect to receive next. */
    NyLPC_TUInt32 snd_nxt32;    /**< 送信用sqカウンター*/
    NyLPC_TUInt16 peer_mss;     /**< PeerのMSS*/
    NyLPC_TUInt16 default_mss;  /**< Peerの初期MMS*/
    /**Peerのウインドウサイズ*/
    NyLPC_TUInt16 peer_win;
    NyLPC_TUInt16 _padding;
    /**現在ソケットのRTO*/
    NyLPC_TUInt32 current_rto32;
};








/**********************************************************************
 *
 * NyLPC_TcTcpSocket class
 *
 **********************************************************************/

/**
 * uipサービスを使用したTCPソケットクラスです。
 * この関数は2つのタスクから呼び出されます。
 * [uipTask]  ->  [cTcpSocket] <- [Application]
 * ApplicationとuipTaskとの間での排他処理はインスタンスで制御されています。
 * Application側からのコールは内部でuipTaskとの間で排他処理を実行します。
 * Application側からのコールはリエントラントではありません。
 */

struct NyLPC_TcMiMicIpTcpSocket
{
	struct NyLPC_TiTcpSocket _super;
	NyLPC_TcIPv4_t* _parent_ipv4;
	//この変数は、uipタスクの実行する関数のみが変更する。
    struct uip_conn uip_connr;
    NyLPC_TcFifoBuffer_t rxbuf;
    struct{
        NyLPC_TUInt8 rp;
        NyLPC_TUInt8 wp;
        //送信キュー
        struct NyLPC_TcTcpSocket_TxQItem txq[NyLPC_TcTcpSocket_NUMBER_OF_TXQ];
    }txbuf;
    volatile NyLPC_TUInt8 tcpstateflags; /**< TCP state and flags. */
};

NyLPC_TBool NyLPC_cMiMicIpTcpSocket_initialize(NyLPC_TcMiMicIpTcpSocket_t* i_inst,void* i_rbuf,NyLPC_TUInt16 i_rbuf_len);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CTCPSOCKET_H_ */
