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
#ifndef NYLPC_ITCPSOCKET_H_
#define NYLPC_ITCPSOCKET_H_


#include "NyLPC_stdlib.h"
#include "NyLPC_os.h"
#include "NyLPC_NetIf_ip_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NyLPC_TiTcpSocket NyLPC_TiTcpSocket_t;

/**********************************************************************
 * Function
 **********************************************************************/


typedef const struct NyLPC_TIPv4Addr* (*NyLPC_TiTcpSocket_getPeerAddr)(const NyLPC_TiTcpSocket_t* i_inst);
typedef NyLPC_TUInt16 (*NyLPC_TiTcpSocket_getPeerPort)(const NyLPC_TiTcpSocket_t* i_inst);


typedef NyLPC_TBool (*NyLPC_TiTcpSocket_accept)(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt32 i_wait_in_msec);
/**
 * @return
 *  1 - 以上:受信に成功した。
 *  0 - タイムアウト
 * -1 - ソケットがクローズしている
 */
typedef NyLPC_TInt32 (*NyLPC_TiTcpSocket_precv)(NyLPC_TiTcpSocket_t* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_wait_msec);
typedef void (*NyLPC_TiTcpSocket_pseek)(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt16 i_seek);
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
typedef NyLPC_TInt32(*NyLPC_TiTcpSocket_send)(NyLPC_TiTcpSocket_t* i_inst,const void* i_buf_ptr,NyLPC_TInt32 i_len,NyLPC_TUInt32 i_wait_in_msec);
typedef void(*NyLPC_TiTcpSocket_close)(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt32 i_wait_in_msec);


/**
 * NyLPC_iTcpSocket_psendで送信するための送信バッファ準備します。
 * @param i_hint
 * 送信を希望するデータサイズを指定します。
 * アロケータは出来る限り希望に沿ってメモリを返します。
 * @param o_buf_size
 * 取得できたバッファサイズを返します。
 * @return
 * 成功した場合、送信バッファを返します。
 * アプリケーションは、可能な限り速やかにデータを書き込んで、NyLPC_iTcpSocket_psendをコールしてください。
 * @note
 * Optionフィールドを持つパケットを送信する場合は、オプションデータサイズの合計をデータサイズに指定して、payloadwriterで調整すること。
 */
typedef void* (*NyLPC_TiTcpSocket_allocSendBuf)(NyLPC_TiTcpSocket_t* i_inst,NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_buf_size,NyLPC_TUInt32 i_wait_in_msec);

/**
 * NyLPC_iTcpSocket_allocSendBufで確保したメモリを開放します。
 */
typedef void (*NyLPC_TiTcpSocket_releaseSendBuf)(NyLPC_TiTcpSocket_t* i_inst,void* i_buf_ptr);

/**
 * 事前にAllocしたTxパケットを送信します。
 * このAPIはゼロコピー送信をサポートするためのものです。
 * @param i_buf_ptr
 * allocSendBufで取得したメモリを指定します。
 * @return
 * 失敗した場合、メモリは開放されません。
 */
typedef NyLPC_TBool (*NyLPC_TiTcpSocket_psend)(NyLPC_TiTcpSocket_t* i_inst,void* i_buf_ptr,int i_len,NyLPC_TUInt32 i_wait_in_msec);

/**
 * TCPソケットをクライアントとしてサーバへ接続します。
 */
typedef NyLPC_TBool (*NyLPC_TiTcpSocket_connect)(NyLPC_TiTcpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_peer_port,NyLPC_TUInt32 i_wait_in_msec);

typedef void (*NyLPC_TiTcpSocket_finalize)(NyLPC_TiTcpSocket_t* i_inst);

/**********************************************************************
 * Interface
 **********************************************************************/

/**
 */
struct NyLPC_TiTcpSocket_Interface
{
	NyLPC_TiTcpSocket_getPeerAddr getpeeraddr;
	NyLPC_TiTcpSocket_getPeerPort getpeerport;
	NyLPC_TiTcpSocket_accept accept;
	NyLPC_TiTcpSocket_precv precv;
	NyLPC_TiTcpSocket_pseek pseek;
	NyLPC_TiTcpSocket_send send;
	NyLPC_TiTcpSocket_close close;
	NyLPC_TiTcpSocket_allocSendBuf allocSendBuf;
	NyLPC_TiTcpSocket_releaseSendBuf releaseSendBuf;
	NyLPC_TiTcpSocket_psend psend;
	NyLPC_TiTcpSocket_connect connect;
	NyLPC_TiTcpSocket_finalize finalize;
};

struct NyLPC_TiTcpSocket
{
	const struct NyLPC_TiTcpSocket_Interface* _interface;
};


#define NyLPC_iTcpSocket_getPeerAddr(i_inst)									((i_inst)->_interface->getpeeraddr((i_inst)))
#define NyLPC_iTcpSocket_getPeerPort(i_inst)									((i_inst)->_interface->getpeerport((i_inst)))
#define NyLPC_iTcpSocket_accept(i_inst,i_wait_in_msec)							((i_inst)->_interface->accept((i_inst),(i_wait_in_msec)))
#define NyLPC_iTcpSocket_precv(i_inst,o_buf_ptr,i_wait_msec)					((i_inst)->_interface->precv((i_inst),(o_buf_ptr),(i_wait_msec)))
#define NyLPC_iTcpSocket_pseek(i_inst,i_seek)									((i_inst)->_interface->pseek((i_inst),(i_seek)))
#define NyLPC_iTcpSocket_send(i_inst,i_buf_ptr,i_len,i_wait_in_msec)			((i_inst)->_interface->send((i_inst),(i_buf_ptr),(i_len),(i_wait_in_msec)))
#define NyLPC_iTcpSocket_close(i_inst,i_wait_in_msec)							((i_inst)->_interface->close((i_inst),(i_wait_in_msec)))
#define NyLPC_iTcpSocket_allocSendBuf(i_inst,i_hint,o_buf_size,i_wait_in_msec)	((i_inst)->_interface->allocSendBuf((i_inst),(i_hint),(o_buf_size),(i_wait_in_msec)))
#define NyLPC_iTcpSocket_releaseSendBuf(i_inst,i_buf_ptr)						((i_inst)->_interface->releaseSendBuf((i_inst),(i_buf_ptr)))
#define NyLPC_iTcpSocket_psend(i_inst,i_buf_ptr,i_len,i_wait_in_msec)			((i_inst)->_interface->psend((i_inst),(i_buf_ptr),(i_len),(i_wait_in_msec)))
#define NyLPC_iTcpSocket_connect(i_inst,i_addr,i_peer_port,i_wait_in_msec)		((i_inst)->_interface->connect((i_inst),(i_addr),(i_peer_port),(i_wait_in_msec)))
#define NyLPC_iTcpSocket_finalize(i_inst)										((i_inst)->_interface->finalize((i_inst)))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_ITCPSOCKET_H_ */
