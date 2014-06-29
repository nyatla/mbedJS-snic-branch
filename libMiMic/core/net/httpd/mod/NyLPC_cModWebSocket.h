#ifndef NYLPC_CMODWEBSOCKET_H_
#define NYLPC_CMODWEBSOCKET_H_

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
 *	http://nyatla.jp/
 *	<airmail(at)ebony.plala.or.jp> or <nyatla(at)nyatla.jp>
 *
 *********************************************************************************/
#include "NyLPC_stdlib.h"
#include "NyLPC_http.h"
#include "../NyLPC_cHttpdConnection_protected.h"
#include "../NyLPC_cHttpdUtils.h"
#include "NyLPC_cModRomFiles.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * ファイルアップロードの為の基本シーケンスを提供する抽象クラスです。
 * 継承クラスで_abstruct_function以下の関数に実体を設定して使います。
 */
typedef struct NyLPC_TcModWebSocket NyLPC_TcModWebSocket_t;


#define NyLPC_TcModWebSocket_FRAME_TYPE_BIN 0x01
#define NyLPC_TcModWebSocket_FRAME_TYPE_TXT 0x02

typedef NyLPC_TUInt8 NyLPC_TcModWebSocket_ST;
/**
 * パケットヘッダの受信待ちである。
 */
#define NyLPC_TcModWebSocket_ST_START_PAYLOAD	0x02
/**
 * ペイロードの読み取り中である。
 */
#define NyLPC_TcModWebSocket_ST_READ_PAYLOAD	0x03
/**
 * WebSocketは閉じられた。
 */
#define NyLPC_TcModWebSocket_ST_CLOSED			0x04

/**
 * クラス構造体
 */
struct NyLPC_TcModWebSocket
{
	NyLPC_TcModRomFiles_t super;
	/**
	 * サブプロトコル。NULLの場合0
	 */
	const NyLPC_TChar* _ref_sub_protocol;
	/**ペイロードの解析ステータス*/
	NyLPC_TcModWebSocket_ST _payload_st;
	NyLPC_TUInt8 _frame_type;
	/**
	 * BIT0: MASK value
	 */
	NyLPC_TUInt8 _frame_flags_bits;
	NyLPC_TUInt8 _frame_mask[4];
	/** ペイロードサイズ*/
	NyLPC_TUInt16 payload_size;
	/** ペイロード位置*/
	NyLPC_TUInt16 payload_ptr;
	NyLPC_TcHttpdConnection_t* _ref_connection;
};


/**
 * NyLPC_cModWebSocket_readCB関数のコールバックハンドラです。
 * @return
 * -1　　　　:エラーが発生したことを通知します。NyLPC_cModWebSocket_readCBは負数を返してクローズします。
 *  0　　　　:読出しの中断を通知します。NyLPC_cModWebSocket_readCBは読み取ったデータ数を記録して正常終了します。
 *  1　　　　:読出しの継続を通知します。NyLPC_cModWebSocket_readCBは継続してデータを通知します。
 */
typedef NyLPC_TInt32 (*NyLPC_TcModWebSocket_onRreadCB)(void* i_param,NyLPC_TChar i_c);

/**
 * コンストラクタ。
 */
void NyLPC_cModWebSocket_initialize(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_ref_root_path);

void NyLPC_cModWebSocket_finalize(NyLPC_TcModWebSocket_t* i_inst);

/**
 * モジュールがコネクションをハンドリングできるかを返します。
 */
NyLPC_TBool NyLPC_cModWebSocket_canHandle(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection);

/**
 * モジュールを実行します。
 */
NyLPC_TBool NyLPC_cModWebSocket_execute(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection);

/**
 * NyLPC_cModWebSocket_read関数がブロック無しに完了できるかを返します。
 * NyLPC_cModWebSocket_readで処理をブロックしたくない場合に使います。
 * アプリケーションはこの関数を頻繁にチェックして、trueの場合は速やかにNyLPC_cModWebSocket_readを実行してください。
 */
NyLPC_TBool NyLPC_cModWebSocket_canRead(const NyLPC_TcModWebSocket_t* i_inst);


/**
 * 受信データをコールバック関数に通知するNyLPC_cModWebSocket_readです。
 * @param i_cb
 * NyLPC_TcModWebSocket_onRreadCBのi_paramに設定する数値です。
 * @return
 * n>0:データ受信
 * 0  :タイムアウト。コネクションの状態は変化しない。
 * -1 :エラー コネクションはNyLPC_TcModWebSocket_ST_CLOSEDへ遷移する。
 */
NyLPC_TInt16 NyLPC_cModWebSocket_readCB(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TcModWebSocket_onRreadCB i_cb,void* i_cb_param);


/**
 * ストリームからデータを受信して、可能ならi_bufに最大i_buf_lenバイトのデータを受信します。
 * この関数はデータ以外のパケットも処理します。
 * @return
 * n>0:受信成功。nバイトのデータを受信した。
 * 0  :タイムアウト。コネクションの状態は変化しない。
 * -1 :エラー。 コネクションはNyLPC_TcModWebSocket_ST_CLOSEDへ遷移する。
 */
NyLPC_TInt16 NyLPC_cModWebSocket_read(NyLPC_TcModWebSocket_t* i_inst,void* i_buf,NyLPC_TInt16 i_buf_len);
/**
 * i_bufからi_lenバイトのデータを送信します。データは１ペーロードとしてクライアントへ送信されます。
 * この関数はデータ以外のパケットも処理します。
 * @return
 * true: 送信に成功した。
 * false:送信に失敗した。 コネクションはNyLPC_TcModWebSocket_ST_CLOSEDへ遷移する。
 */
NyLPC_TBool NyLPC_cModWebSocket_write(NyLPC_TcModWebSocket_t* i_inst,const void* i_buf,NyLPC_TInt16 i_len);
/**
 * 書式文字列を出力します。
 */
NyLPC_TBool NyLPC_cModWebSocket_writeFormat(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_fmt,...);
NyLPC_TBool NyLPC_cModWebSocket_writeFormatV(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_fmt,va_list args);


/**
 * 書式文字列を出力した場合の文字数を返します。
 * この関数は、startBulkWrite関数のi_lenパラメータに渡す値を計算するときに使います。
 */
NyLPC_TInt16 NyLPC_cModWebSocket_testFormatV(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_fmt,va_list args);
NyLPC_TInt16 NyLPC_cModWebSocket_testFormat(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_fmt,...);

/**
 * バルク書き込みを開始します。
 * バルク書き込みは、endBulkWrite関数をコールするまでにwriteBulk関数で入力されたデータを、１つのi_lenサイズのWebsocketパケットとして送信します。
 * バルク書き込み中は、通常のwrite関数を使用することができません。
 * @param i_len
 * NyLPC_cModWebSocket_endBulkFormatでバルク書き込みを終了するまでに入力するデータサイズを指定します。
 */
NyLPC_TBool NyLPC_cModWebSocket_startBulkWrite(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TInt16 i_len);
/**
 * バルク書き込みを終了します。
 * この関数をコールする前に、startBulkWrite関数のi_lenで指定した大きさのデータを入力し終えている必要があります。
 * 過不足があった場合、WebSocketセッションが破壊されます。
 */
NyLPC_TBool NyLPC_cModWebSocket_endBulkWrite(NyLPC_TcModWebSocket_t* i_inst);
NyLPC_TBool NyLPC_cModWebSocket_writeBulkFormatV(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_fmt,va_list args);
NyLPC_TBool NyLPC_cModWebSocket_writeBulkFormat(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_fmt,...);

/**
 * CLOSEパケットを送信してコネクションを閉じます。
 * この関数はデータ以外のパケットも処理します。
 * i_codeにはWebsocketのコード
 */
void NyLPC_cModWebSocket_close(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TUInt16 i_code);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CMODWEBSOCKET_H_ */
