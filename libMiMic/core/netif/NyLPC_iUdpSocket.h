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
#ifndef NYLPC_IIUDPSOCKET_H_
#define NYLPC_IIUDPSOCKET_H_


#include "NyLPC_stdlib.h"
#include "NyLPC_os.h"
#include "NyLPC_NetIf_ip_types.h"
#include "NyLPC_iTcpSocket.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NyLPC_TiUdpSocket NyLPC_TiUdpSocket_t;
/**********************************************************************
 * Struct
 **********************************************************************/

/**
 * 受信情報を格納する構造体
 */
struct NyLPC_TIPv4RxInfo
{
    NyLPC_TUInt16 size;//パケットサイズ
    NyLPC_TUInt16 port;//受信ポート
    NyLPC_TUInt16 peer_port;//PEERポート
    struct NyLPC_TIPv4Addr ip;//受信IP
    struct NyLPC_TIPv4Addr peer_ip;//PEERIP
};


/**********************************************************************
 * Event
 **********************************************************************/

/**
 * 受信時に非同期にコールされるハンドラ
 * UIPサービスタスクが実行する。
 * @return
 * TRUEならパケットを受信キューへ追加する。FALSEならパケットを受信キューへ追加しない。
 */
typedef NyLPC_TBool (*NyLPC_TiUdpSocket_onRxHandler)(NyLPC_TiUdpSocket_t* i_inst,const void* i_buf,const struct NyLPC_TIPv4RxInfo* i_info);

/**
 * 非同期にコールされるハンドラ。
 * UIPサービスタスクが実行する。
 */
typedef void (*NyLPC_TiUdpSocket_onPeriodicHandler)(NyLPC_TiUdpSocket_t* i_inst);

/**********************************************************************
 * Function
 **********************************************************************/



/**
 * マルチキャストアドレスに参加する。
 * @param i_addr
 * 参加するマルチキャストグループを指定する。
 * 同じマルチキャスとグループに参加できるのは、システムの中で１つに限られます。
 * 0を指定した場合、マルチキャスとグループから離脱します。
 */
typedef void (*NyLPC_TiUdpSocket_joinMulticast)(NyLPC_TiUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr);

/**
 * ブロードキャストに参加する。
 */
typedef void (*NyLPC_TiUdpSocket_setBroadcast)(NyLPC_TiUdpSocket_t* i_inst);


/**
 * この関数は、ソケットの受信バッファの読み取り位置と、読み出せるデータサイズを返却します。
 * 関数はポインターを返却するだけで、バッファの読み取り位置をシークしません。
 * シークするにはNyLPC_cTcpSocket_pseekを使います。
 */
typedef NyLPC_TInt32 (*NyLPC_TiUdpSocket_precv)(NyLPC_TiUdpSocket_t* i_inst,const void** o_buf_ptr,const struct NyLPC_TIPv4RxInfo** o_info,NyLPC_TUInt32 i_wait_msec);
/**
 * 受信バッファを次のバッファまでシークします。
 */
typedef void (*NyLPC_TiUdpSocket_pseek)(NyLPC_TiUdpSocket_t* i_inst);

/**
 * 送信バッファを割り当てて返します。
 * 割り当てたメモリは、releaseSendBuf関数か、psend関数を成功させて開放する必要があります。
 * @param i_hint
 * 取得したいメモリサイズをセットします。
 * 関数は要求サイズより小さいメモリを返すことがあります。
 */
typedef void* (*NyLPC_TiUdpSocket_allocSendBuf)(NyLPC_TiUdpSocket_t* i_inst,NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_buf_size,NyLPC_TUInt32 i_wait_in_msec);

typedef void (*NyLPC_TiUdpSocket_releaseSendBuf)(NyLPC_TiUdpSocket_t* i_inst,void* i_buf_ptr);

/**
 * 事前にAllocしたTxパケットを送信します。
 * このAPIはゼロコピー送信をサポートするためのものです。
 * @param i_buf_ptr
 * allocSendBufで取得したメモリを指定します。
 * @return
 * 関数が失敗した場合、i_buf_ptrは「開放されません。」
 */
typedef NyLPC_TBool (*NyLPC_TiUdpSocket_psend)(NyLPC_TiUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_port,void* i_buf_ptr,int i_len);

/**
 * 最大送信サイズは1200バイトです。
 */
typedef NyLPC_TInt32 (*NyLPC_TiUdpSocket_send)(NyLPC_TiUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_port,const void* i_buf_ptr,NyLPC_TInt32 i_len,NyLPC_TUInt32 i_wait_in_msec);

/**
 * 非同期パケットハンドラを設定する。
 */
typedef void (*NyLPC_TiUdpSocket_setOnRxHandler)(NyLPC_TiUdpSocket_t* i_inst,NyLPC_TiUdpSocket_onRxHandler i_handler);

/**
 * 非同期タイマ呼び出しハンドラを設定する。
 */
typedef void (*NyLPC_TiUdpSocket_setOnPeriodicHandler)(NyLPC_TiUdpSocket_t* i_inst,NyLPC_TiUdpSocket_onPeriodicHandler i_handler);


/**
 * ソケットのローカルIPのアドレスを返す。
 * 値はuipが動作中のみ有効。
 */
typedef struct NyLPC_TIPv4Addr* (*NyLPC_TiUdpSocket_getSockIP)(const NyLPC_TiUdpSocket_t* i_inst);

typedef void (*NyLPC_TiUdpSocket_finalize)(NyLPC_TiUdpSocket_t* i_inst);


/**********************************************************************
 * Interface
 **********************************************************************/

/**
 */
struct NyLPC_TiUdpSocket_Interface
{
	NyLPC_TiUdpSocket_joinMulticast joinMulticast;
	NyLPC_TiUdpSocket_setBroadcast setBroadcast;
	NyLPC_TiUdpSocket_precv precv;
	NyLPC_TiUdpSocket_pseek pseek;
	NyLPC_TiUdpSocket_allocSendBuf allocSendBuf;
	NyLPC_TiUdpSocket_releaseSendBuf releaseSendBuf;
	NyLPC_TiUdpSocket_psend psend;
	NyLPC_TiUdpSocket_send send;
	NyLPC_TiUdpSocket_setOnRxHandler setOnRxHandler;
	NyLPC_TiUdpSocket_setOnPeriodicHandler setOnPeriodicHandler;
	NyLPC_TiUdpSocket_getSockIP getSockIP;
	NyLPC_TiUdpSocket_finalize finalize;
};

struct NyLPC_TiUdpSocket
{
	const struct NyLPC_TiUdpSocket_Interface* _interface;
	void* _tag;
};


#define NyLPC_iUdpSocket_joinMulticast(i_inst,i_addr)								((i_inst)->_interface->joinMulticast((i_inst),(i_addr)))
#define NyLPC_iUdpSocket_setBroadcast(i_inst)										((i_inst)->_interface->setBroadcast((i_inst)))
#define NyLPC_iUdpSocket_precv(i_inst,o_buf_ptr,o_info,i_wait_msec)					((i_inst)->_interface->precv((i_inst),(o_buf_ptr),(o_info),(i_wait_msec)))
#define NyLPC_iUdpSocket_pseek(i_inst)												((i_inst)->_interface->pseek((i_inst)))
#define NyLPC_iUdpSocket_allocSendBuf(i_inst,i_hint,o_buf_size,i_wait_in_msec)		((i_inst)->_interface->allocSendBuf((i_inst),(i_hint),(o_buf_size),(i_wait_in_msec)))
#define NyLPC_iUdpSocket_releaseSendBuf(i_inst,i_buf_ptr)							((i_inst)->_interface->releaseSendBuf((i_inst),(i_buf_ptr)))
#define NyLPC_iUdpSocket_psend(i_inst,i_addr,i_port,i_buf_ptr,i_len)				((i_inst)->_interface->psend((i_inst),(i_addr),(i_port),(i_buf_ptr),(i_len)))
#define NyLPC_iUdpSocket_send(i_inst,i_addr,i_port,i_buf_ptr,i_len,i_wait_in_msec)	((i_inst)->_interface->send((i_inst),(i_addr),(i_port),(i_buf_ptr),(i_len),(i_wait_in_msec)))
#define NyLPC_iUdpSocket_setOnRxHandler(i_inst,i_handler)							((i_inst)->_interface->setOnRxHandler((i_inst),(i_handler)))
#define NyLPC_iUdpSocket_setOnPeriodicHandler(i_inst,i_handler)						((i_inst)->_interface->setOnPeriodicHandler((i_inst),(i_handler)))
#define NyLPC_iUdpSocket_getSockIP(i_inst)											((i_inst)->_interface->getSockIP((i_inst)))
#define NyLPC_iUdpSocket_finalize(i_inst)											((i_inst)->_interface->finalize((i_inst)))


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_IIUDPSOCKET_H_ */
