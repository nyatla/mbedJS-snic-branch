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
#ifndef NYLPC_CMODJSONRPC_H_
#define NYLPC_CMODJSONRPC_H_
#include "NyLPC_http.h"
#include "../NyLPC_cHttpdConnection.h"
#include "NyLPC_cModWebSocket.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * WebSocketストリームからJSONRPC電文を取り込むモジュールです。
 */
typedef struct NyLPC_TcModJsonRpc NyLPC_TcModJsonRpc_t;


/**
 * クラス構造体
 */
struct NyLPC_TcModJsonRpc
{
	NyLPC_TcModWebSocket_t super;
	NyLPC_TcJsonRpcParser_t _rpc_parser;
	union NyLPC_TJsonRpcParserResult _result;
};


void NyLPC_cModJsonRpc_initialize(NyLPC_TcModJsonRpc_t* i_inst,const NyLPC_TChar* i_ref_root_path,const struct NyLPC_TJsonRpcClassDef** i_class_tbl);
void NyLPC_cModJsonRpc_finalize(NyLPC_TcModJsonRpc_t* i_inst);
#define NyLPC_cModJsonRpc_canHandle(i,c) NyLPC_cModWebSocket_canHandle(&((i)->super),(c))
#define NyLPC_cModJsonRpc_close(i,t) NyLPC_cModWebSocket_close(&((i)->super),(t))


NyLPC_TBool NyLPC_cModJsonRpc_execute(NyLPC_TcModJsonRpc_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection);

/**
 * Execute実行後に繰り返し実行できます。
 * trueを返却した場合は、NyLPC_cModJsonRpc_getRpcCall関数で結果を取得できるか調べてください。
 * @return
 * JSONRPCの処理を継続して行えるかを返します。
 * falseの場合、JSONRPCの構文解析は失敗し、Websocketは閉じられます。Websocketの受信ループを終了してください。
 */
NyLPC_TBool NyLPC_cModJsonRpc_processRpcMessage(NyLPC_TcModJsonRpc_t* i_inst);

/**
 * JSONRPCの構文解析結果を返します。
 * @return
 * JSONRPC電文が確定した場合、結果を格納した構造体を返します。構造体の有効期限は、次回にNyLPC_cModJsonRpc_processRpcMessageを実行するまでです。
 * NULLを返した場合は、引き続きNyLPC_cModJsonRpc_processRpcMessageを実行する必要があります。
 */
const union NyLPC_TJsonRpcParserResult* NyLPC_cModJsonRpc_getMessage(NyLPC_TcModJsonRpc_t* i_inst);

NyLPC_TBool NyLPC_cModJsonRpc_putResult(NyLPC_TcModJsonRpc_t* i_inst,NyLPC_TUInt32 i_id,const NyLPC_TChar* i_params_fmt,...);
NyLPC_TBool NyLPC_cModJsonRpc_putResultV(NyLPC_TcModJsonRpc_t* i_inst,NyLPC_TUInt32 i_id,const NyLPC_TChar* i_params_fmt,va_list i_a);
NyLPC_TBool NyLPC_cModJsonRpc_putError(NyLPC_TcModJsonRpc_t* i_inst,NyLPC_TUInt32 i_id,NyLPC_TInt32 i_code);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CMODJSONRPC_H_ */
