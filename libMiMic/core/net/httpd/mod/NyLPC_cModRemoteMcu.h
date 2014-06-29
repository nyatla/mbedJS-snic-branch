/*
 * NyLPC_cModRemoteMcu.h
 *
 *  Created on: 2013/03/07
 *      Author: nyatla
 */

#ifndef NYLPC_CMODREMOTEMCU_H_
#define NYLPC_CMODREMOTEMCU_H_

#include "NyLPC_http.h"
#include "../NyLPC_cHttpdConnection.h"
#include "NyLPC_cModRomFiles.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * URLが最大31文字までの、短いHttpリクエストを処理します。
 * このクラスは、NyLPC_TBasicHttpHeader_tにキャストできます。
 *
 * JSONAPIとして、以下のAPIを提供します。
 * ./mvm.api?o=[json|xml]&v=1&bc=[:MIMICBC:]
 * MiMicBCを実行します。
 * oパラメータ省略時はjsonと解釈します。
 */
typedef struct NyLPC_TcModRemoteMcu NyLPC_TcModRemoteMcu_t;


struct NyLPC_TcModRemoteMcu
{
    NyLPC_TcModRomFiles_t super;
};

/**
 * コンストラクタ。
 */
void NyLPC_cModRemoteMcu_initialize(NyLPC_TcModRemoteMcu_t* i_inst,const NyLPC_TChar* i_ref_root_path);
void NyLPC_cModRemoteMcu_finalize(NyLPC_TcModRemoteMcu_t* i_inst);

/**
 * モジュールがコネクションをハンドリングできるかを返します。
 */
NyLPC_TBool NyLPC_cModRemoteMcu_canHandle(NyLPC_TcModRemoteMcu_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection);
/**
 * モジュールを実行します。
 * @return
 * 処理に成功したか
 */
NyLPC_TBool NyLPC_cModRemoteMcu_execute(NyLPC_TcModRemoteMcu_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CMODREMOTEMCU_H_ */
