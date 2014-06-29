/*
 * NyLPC_cHttpdConnection.h
 *
 *  Created on: 2013/02/07
 *      Author: nyatla
 */

#ifndef NYLPC_CHTTPDCONNECTION_PROTECTED_H_
#define NYLPC_CHTTPDCONNECTION_PROTECTED_H_
#include "NyLPC_uipService.h"
#include "NyLPC_cHttpdConnection.h"
#include "NyLPC_stdlib.h"
#include "NyLPC_cHttpdUtils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * コネクションモード
 */
typedef NyLPC_TUInt8 NyLPC_TcHttpdConnection_CONNECTION_MODE;
/**
 * 持続性接続
 */
#define NyLPC_TcHttpdConnection_CONNECTION_MODE_CONTINUE 1
/**
 * 持続性接続キャンセル
 */
#define NyLPC_TcHttpdConnection_CONNECTION_MODE_CLOSE    2

/**
 * URL Prefixを返却します。
 */
const NyLPC_TChar* NyLPC_cHttpdConnection_getUrlPrefix(const NyLPC_TcHttpdConnection_t* i_inst);

/**
 * コネクションステータスをパース済みに設定します。
 * HttpModで処理を行った後にコールします。
 */
void NyLPC_cHttpdConnection_setReqStatusParsed(NyLPC_TcHttpdConnection_t* i_inst);


/**
 * コネクションをHTTPレベルで閉じます。
 * ResponseステータスはCLOSEDになります。Requestステータスは変化しません。
 */
NyLPC_TBool NyLPC_cHttpdConnection_closeResponse(NyLPC_TcHttpdConnection_t* i_inst);


/**
 * ソケットをlistenします。LISTEN状態のソケットに使えます。
 */
NyLPC_TBool NyLPC_cHttpdConnection_listenSocket(NyLPC_TcHttpdConnection_t* i_inst,NyLPC_TcTcpListener_t* i_listener);

/**
 * コネクションのソケットをacceptします。
 * listenSocketが成功したソケットにだけ使えます。
 * 関数が成功すると、REQStatusがHEADに遷移します。
 */
NyLPC_TBool NyLPC_cHttpdConnection_acceptSocket(NyLPC_TcHttpdConnection_t* i_inst);

/**
 * コネクションのソケットを閉じます。
 * ResponseステータスはCLOSEDになり、RequestステータスはLISTENになります。
 */
void NyLPC_cHttpdConnection_closeSocket(NyLPC_TcHttpdConnection_t* i_inst);

/**
 * ヘッダをprefetchします。
 */
NyLPC_TBool NyLPC_cHttpdConnection_prefetch(NyLPC_TcHttpdConnection_t* i_inst);

/**
 * コネクションのプリフェッチデータをヘッダパーサへpushします。
 */
NyLPC_TBool NyLPC_cHttpdConnection_pushPrefetchInfo(NyLPC_TcHttpdConnection_t* i_inst,NyLPC_TcHttpBasicHeaderParser_t* i_header_parser,struct NyLPC_THttpBasicHeader* o_out);


/**
 * 次のプリフェッチを準備する。
 */
NyLPC_TBool NyLPC_cHttpdConnection_prevNextPrefetch(NyLPC_TcHttpdConnection_t* i_inst);

/**
 * リクエストのパースを完了する。
 */
void NyLPC_cHttpdConnection_requestParsed(NyLPC_TcHttpdConnection_t* i_inst);


/**
 * コネクションのモードをセットする。
 * この関数はhttpdハンドラのコールするモジュールが呼び出すことが有る。
 * HTTPリクエストのバージョンが1.1かつconnectionがCLOSEで無い場合、CONTINUEを指定することで持続性接続に設定できる。
 * 関数は、NyLPC_cHttpdConnection_sendResponseHeaderをコールする前に実行すること。
 */
#define NyLPC_cHttpdConnection_setConnectionMode(i_inst,i_mode)     (i_inst)->_connection_message_mode=(i_mode)

NyLPC_TUInt16 NyLPC_cHttpd_incNumOfConnection(NyLPC_TcHttpd_t* i_inst);
NyLPC_TUInt16 NyLPC_cHttpd_decNumOfConnection(NyLPC_TcHttpd_t* i_inst);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* NYLPC_CHTTPDCONNECTION_H_ */
