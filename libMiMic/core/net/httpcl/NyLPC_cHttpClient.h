/*
 * NyLPC_cHttpClient.h
 *
 *  Created on: 2013/08/24
 *      Author: nyatla
 */

#ifndef NYLPC_CHTTPCLIENT_H_
#define NYLPC_CHTTPCLIENT_H_

#include "NyLPC_stdlib.h"
#include "NyLPC_net.h"
#include "NyLPC_http.h"
#include "NyLPC_netif.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NyLPC_TcHttpClient NyLPC_TcHttpClient_t;

#define NyLPC_cHttpHeaderWriter_CONTENT_LENGTH_UNLIMITED 0xffffffff
struct NyLPC_TcHttpClient
{
	NyLPC_TUInt8 _state;
	NyLPC_TUInt8 _padding1;
	NyLPC_TiTcpSocket_t* _sock;
	NyLPC_TcHttpStream_t _stream;
	union{
		NyLPC_TcHttpHeaderWriter_t head_writer;
		NyLPC_TcHttpBodyWriter_t body_writer;
		NyLPC_TcHttpBasicHeaderParser_t head_parser;
		NyLPC_TcHttpBodyParser_t body_parser;
	}pw;
};


NyLPC_TBool NyLPC_cHttpClient_initialize(NyLPC_TcHttpClient_t* i_inst,void* i_rx_buf,NyLPC_TUInt16 i_rx_size);

void NyLPC_cHttpClient_finalize(NyLPC_TcHttpClient_t* i_inst);

/**
 * サーバとの接続を切断する。
 * ステータスはCLOSEDになる。
 */
void NyLPC_cHttpClient_close(NyLPC_TcHttpClient_t* i_inst);


/**
 * サーバに接続する。
 * 関数はステータスをIDLEへ遷移する。
 * インスタンスのステータスは何でも構わない。
 * @return
 * TRUE - ステータスはIDLEへ遷移する。
 * FALSE - ステータスはCLOSEDへ遷移する。
 */
NyLPC_TBool NyLPC_cHttpClient_connect(NyLPC_TcHttpClient_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_port);



/**
 * POSTリクエストを送信する。
 * ステータスはIDLEである必要がある。
 * @param i_content_length
 * 送信bodyのサイズ。最大 0xfffffffe
 * NyLPC_cHttpHeaderWriter_CONTENT_LENGTH_UNLIMITEDの場合はChunked転送になる。
 * @return
 * 引き続き処理が可能かを返す。
 * TRUE - 成功。ステータスはSEND_REQ_BODYになる。write/getStatusを呼び出せる。
 * FALSE - 失敗。ステータスはCLOSEDになる。
 */
NyLPC_TBool NyLPC_cHttpClient_sendMethod(
	NyLPC_TcHttpClient_t* i_inst,
	NyLPC_THttpMethodType i_method,
	const NyLPC_TChar* i_path,
	NyLPC_TUInt32 i_content_length,
	const NyLPC_TChar* i_mime_type,
	const NyLPC_TChar* i_additional_header);


/**
 * POSTリクエストのデータを送信する。
 * ステータスはSEND_REQ_BODYである必要がある。
 * @return
 * TRUE - 成功。
 * FALSE - 失敗。ステータスはCLOSEDになる。
 */
NyLPC_TBool NyLPC_cHttpClient_write(NyLPC_TcHttpClient_t* i_inst,const void* i_buf,NyLPC_TUInt32 i_buf_size);

/**
 * 書式文字列としてPOSTリクエストのデータを送信する。
 * ステータスはSEND_REQ_BODYである必要がある。
 * @param i_fmt
 * printfライクなフォーマット文字列
 * @return
 * TRUE - 成功。
 * FALSE - 失敗。ステータスはCLOSEDになる。
 */
NyLPC_TBool NyLPC_cHttpClient_writeFormat(NyLPC_TcHttpClient_t* i_inst,const NyLPC_TChar* i_fmt,...);
NyLPC_TBool NyLPC_cHttpClient_writeFormatV(NyLPC_TcHttpClient_t* i_inst,const NyLPC_TChar* i_fmt,va_list i_args);

/**
 * ステータスコードを返す。
 * ステータスはSEND_REQ_BODYである必要がある。
 * @return
 * 0　- 失敗。ステータスはCLOSEDになる。
 * その他 - ステータスコード。ステータスはRECV_RES_BODYになる。
 */
NyLPC_TUInt16 NyLPC_cHttpClient_getStatus(NyLPC_TcHttpClient_t* i_inst);


/**
 * GET/POSTリクエストで受信したデータを読み出す。
 * ステータスはRECV_RES_BODYである必要がある。
 * @param o_read_len
 * 戻り値TRUEの場合のみ有効。データ終端に達した場合は0になる。
 * @return
 * TRUE:正常読み出し。o_read_lenの値で終端判定
 * FALSE:失敗。コネクションはクローズされる。
 */
NyLPC_TBool NyLPC_cHttpClient_read(NyLPC_TcHttpClient_t* i_inst,void* i_buf,NyLPC_TUInt32 i_buf_size,NyLPC_TInt16* o_read_len);






#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CHTTPCLIENT_H_ */

