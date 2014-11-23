/*
 * NyLPC_cHttpdConnection.h
 *
 *  Created on: 2013/02/07
 *      Author: nyatla
 */

#ifndef NYLPC_CHTTPDCONNECTION_H_
#define NYLPC_CHTTPDCONNECTION_H_
#include "NyLPC_stdlib.h"
#include "NyLPC_cHttpRequestPrefixParser.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef DEFINE_NyLPC_TcHttpd_t
    typedef struct NyLPC_TcHttpd NyLPC_TcHttpd_t;
    #define DEFINE_NyLPC_TcHttpd_t
#endif


typedef NyLPC_TUInt8 NyLPC_TcHttpdConnection_Status;

/**　リクエストプレフィクスを読み出した状態*/
#define NyLPC_cHttpdConnection_ReqStatus_LISTEN         0
/** ACCEPT待ち*/
#define NyLPC_cHttpdConnection_ReqStatus_ACCEPT         1
/**　コネクションは接続済*/
#define NyLPC_cHttpdConnection_ReqStatus_PREFETCH       2
/**　リクエストパース待ち*/
#define NyLPC_cHttpdConnection_ReqStatus_REQPARSE       3
/**　リクエストパース済*/
#define NyLPC_cHttpdConnection_ReqStatus_END            4
#define NyLPC_cHttpdConnection_ReqStatus_BODYPARSE      5




/**　レスポンスヘッダ送付済*/
#define NyLPC_cHttpdConnection_ResStatus_HEAD   1
/**　レスポンスヘッダ送付済*/
#define NyLPC_cHttpdConnection_ResStatus_BODY   2
/**　レスポンスBODY送付済*/
#define NyLPC_cHttpdConnection_ResStatus_CLOSED 3
/** エラーが発生した。*/
#define NyLPC_cHttpdConnection_ResStatus_ERROR  4



/**
 * Httpdのハンドラが引き渡す、HTTPDコネクションクラス。
 *
 */
#ifndef DEFINE_NyLPC_TcHttpdConnection_t
    typedef struct NyLPC_TcHttpdConnection NyLPC_TcHttpdConnection_t;
    #define DEFINE_NyLPC_TcHttpdConnection_t
#endif
struct NyLPC_TcHttpdConnection
{
    NyLPC_TUInt8 _req_status;//リクエストステータス
    NyLPC_TUInt8 _res_status;//レスポンスステータス
    NyLPC_TUInt8 _connection_message_mode;//COnnection:closeをヘッダに書き込むかのフラグ
    NyLPC_TcHttpd_t* _parent_httpd; //NyLPC_cHttpd
    NyLPC_TiTcpSocket_t* _socket;
    NyLPC_TcHttpStream_t _in_stream;
    NyLPC_TcHttpRequestPrefixParser_t _pparser;
    union{
        NyLPC_TcHttpBodyWriter_t _body_writer;
        NyLPC_TcHttpHeaderWriter_t _head_writer;
    };
};

/**
 * @param i_parent_httpd
 *
 */
NyLPC_TBool NyLPC_cHttpdConnection_initialize(NyLPC_TcHttpdConnection_t* i_inst,NyLPC_TcHttpd_t* i_parent_httpd);
void NyLPC_cHttpdConnection_finalize(NyLPC_TcHttpdConnection_t* i_inst);

/**
 * 100 Continueをストリームに送信する。
 * HTTP/1.1でPOSTメッセージを受け付けた場合にコールすること。
 * この関数はステータスがNyLPC_cHttpdConnection_ResStatus_HEADの時だけ実行できる。
 */
NyLPC_TBool NyLPC_cHttpdConnection_send100Continue(NyLPC_TcHttpdConnection_t* i_inst);


/**
 * レスポンスヘッダを送信します。
 * BodyはChunkedエンコーディングで送信します。
 * @param i_additional_header
 * メッセージフィールドに追加する文字列です。
 * \r\nで終端下文字列を指定して下さい。
 */
NyLPC_TBool NyLPC_cHttpdConnection_sendResponseHeader(NyLPC_TcHttpdConnection_t* i_inst,NyLPC_TUInt16 i_response_code,const NyLPC_TChar* i_content_type,const NyLPC_TChar* i_additional_header);
/**
 * レスポンスヘッダを送信します。
 * BodyはContentLengthを伴って送信します。Body送信時にサイズチェックは行いません。
 * @param i_content_length
 * 最大で0x0fffffffを指定できます。
 * @param i_additional_header
 * メッセージフィールドに追加する文字列です。
 * \r\nで終端下文字列を指定して下さい。
 */
NyLPC_TBool NyLPC_cHttpdConnection_sendResponseHeader2(NyLPC_TcHttpdConnection_t* i_inst,NyLPC_TUInt16 i_response_code,const NyLPC_TChar* i_content_type,NyLPC_TUInt32 i_content_length,const NyLPC_TChar* i_additional_header);
/**
 * レスポンスBodyを送信します。
 * 関数を実行後、_res_statusはBODYかERRORに遷移します。
 */
NyLPC_TBool NyLPC_cHttpdConnection_sendResponseBody(NyLPC_TcHttpdConnection_t* i_inst,const void* i_data,NyLPC_TUInt32 i_size);

/**
 * レスポンスBodyを書式出力して送信します。
 * 関数を実行後、_res_statusはBODYかERRORに遷移します。
 */
NyLPC_TBool NyLPC_cHttpdConnection_sendResponseBodyF(NyLPC_TcHttpdConnection_t* i_inst,const char* i_fmt,...);


/**
 * Httpd全体で唯一のロックを取得する。
 */
void NyLPC_cHttpdConnection_lock(NyLPC_TcHttpdConnection_t* i_inst);

/**
 * Httpd全体で唯一のロックを開放する。
 */
void NyLPC_cHttpdConnection_unlock(NyLPC_TcHttpdConnection_t* i_inst);


/**
 * コネクションのStreamを返します。
 */
#define NyLPC_cHttpdConnection_refStream(i_inst) (&(i_inst->_in_stream.super))

#define NyLPC_cHttpdConnection_getMethod(i_inst) ((i_inst)->_pparser.method)
#define NyLPC_cHttpdConnection_getReqStatus(i_inst) ((i_inst)->_req_status)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CHTTPDCONNECTION_H_ */
