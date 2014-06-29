/*
 * NyLPC_httpd_utils.h
 *
 *  Created on: 2013/03/05
 *      Author: nyatla
 */

#ifndef NYLPC_HTTPD_UTILS_H_
#define NYLPC_HTTPD_UTILS_H_

#include "NyLPC_stdlib.h"
#include "NyLPC_http.h"
#include "NyLPC_cHttpdConnection.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * 固定長コンテンツを送信する。
 */
NyLPC_TBool NyLPC_cHttpdUtils_sendFixedContentBatch(NyLPC_TcHttpdConnection_t* i_connection,const NyLPC_TChar* i_content_type,const NyLPC_TChar* i_content,NyLPC_TUInt32 i_size);
/**
 * 標準的なJsonヘッダを送信する。
 */
NyLPC_TBool NyLPC_cHttpdUtils_sendJsonHeader(NyLPC_TcHttpdConnection_t* i_connection);

/**
 * エラーコードを送信する。HEAD以外リクエストに対しては簡単なbodyを返却する。
 */
void NyLPC_cHttpdUtils_sendErrorResponse(NyLPC_TcHttpdConnection_t* i_connection,int i_status);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*  */
