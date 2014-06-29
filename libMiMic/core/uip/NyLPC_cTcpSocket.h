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
#ifndef NYLPC_CTCPSOCKET_H_
#define NYLPC_CTCPSOCKET_H_



#include "NyLPC_uip.h"
#include "NyLPC_os.h"
#include "NyLPC_cIPv4Payload.h"
#include "NyLPC_cBaseSocket.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct NyLPC_TcTcpSocket NyLPC_TcTcpSocket_t;


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

struct NyLPC_TcTcpSocket
{
    /** Base class*/
    NyLPC_TcBaseSocket_t _super;
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


#define NyLPC_cTcpSocket_getPeerAddr(i_inst) (&((i_inst)->uip_connr.ripaddr))
#define NyLPC_cTcpSocket_getPeerPort(i_inst) (((i_inst)->uip_connr.rport))

/**
 * 初期化関数です。
 * uipserviceは初期化済である必要があります。
 * また、暫定条件として、サービスが停止中である必要もあります。
 * @param i_recv_buf
 * 受信バッファを指定します。
 */
NyLPC_TBool NyLPC_cTcpSocket_initialize(NyLPC_TcTcpSocket_t* i_inst,void* i_rbuf,NyLPC_TUInt16 i_rbuf_len);
/**
 * 終期化関数です。
 * uipserviceは初期化済である必要があります。
 * また、暫定条件として、サービスが停止中である必要もあります。
 */
void NyLPC_cTcpSocket_finalize(NyLPC_TcTcpSocket_t* i_inst);

NyLPC_TBool NyLPC_cTcpSocket_accept(NyLPC_TcTcpSocket_t* i_inst,NyLPC_TUInt32 i_wait_in_msec);
/**
 * @return
 *  1 - 以上:受信に成功した。
 *  0 - タイムアウト
 * -1 - ソケットがクローズしている
 */
NyLPC_TInt32 NyLPC_cTcpSocket_precv(NyLPC_TcTcpSocket_t* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_wait_msec);
void NyLPC_cTcpSocket_pseek(NyLPC_TcTcpSocket_t* i_inst,NyLPC_TUInt16 i_seek);
/**
 * 送信未達は保障されません。
 * エラーを検出したら、基本的にはソケットをクローズしてください。
 * @param i_wait_msec
 * 送信失敗までの待ち時間を指定します。現在は、
 * RTT推定ができないため、TCPの再送を考慮して、最低でも10秒(10000)程度を指定してください。
 * @return
 * 送信したバイト数を返します。エラーならば0未満の数を返します。
 *
 */
NyLPC_TInt32 NyLPC_cTcpSocket_send(NyLPC_TcTcpSocket_t* i_inst,const void* i_buf_ptr,NyLPC_TInt32 i_len,NyLPC_TUInt32 i_wait_in_msec);
void NyLPC_cTcpSocket_close(NyLPC_TcTcpSocket_t* i_inst,NyLPC_TUInt32 i_wait_in_msec);


/**
 * NyLPC_cTcpSocket_psendで送信するための送信バッファ準備します。
 * @param i_hint
 * 送信を希望するデータサイズを指定します。
 * アロケータは出来る限り希望に沿ってメモリを返します。
 * @param o_buf_size
 * 取得できたバッファサイズを返します。
 * @return
 * 成功した場合、送信バッファを返します。
 * アプリケーションは、可能な限り速やかにデータを書き込んで、NyLPC_cTcpSocket_psendをコールしてください。
 * @note
 * Optionフィールドを持つパケットを送信する場合は、オプションデータサイズの合計をデータサイズに指定して、payloadwriterで調整すること。
 */
void* NyLPC_cTcpSocket_allocSendBuf(NyLPC_TcTcpSocket_t* i_inst,NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_buf_size,NyLPC_TUInt32 i_wait_in_msec);

/**
 * NyLPC_cTcpSocket_allocSendBufで確保したメモリを開放します。
 */
void NyLPC_cTcpSocket_releaseSendBuf(NyLPC_TcTcpSocket_t* i_inst,void* i_buf_ptr);

/**
 * 事前にAllocしたTxパケットを送信します。
 * このAPIはゼロコピー送信をサポートするためのものです。
 * @param i_buf_ptr
 * allocSendBufで取得したメモリを指定します。
 * @return
 * 失敗した場合、メモリは開放されません。
 */
NyLPC_TBool NyLPC_cTcpSocket_psend(NyLPC_TcTcpSocket_t* i_inst,void* i_buf_ptr,int i_len,NyLPC_TUInt32 i_wait_in_msec);

/**
 * TCPソケットをクライアントとしてサーバへ接続します。
 */
NyLPC_TBool NyLPC_cTcpSocket_connect(NyLPC_TcTcpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_peer_port,NyLPC_TUInt32 i_wait_in_msec);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CTCPSOCKET_H_ */
