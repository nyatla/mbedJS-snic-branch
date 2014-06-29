/*
 * NyLPC_cModRemoteMcu.h
 *
 *  Created on: 2013/03/07
 *      Author: nyatla
 */

#ifndef NYLPC_cModFileIoBaseClass_H_
#define NYLPC_cModFileIoBaseClass_H_

#include "NyLPC_http.h"
#include "NyLPC_stdlib.h"
#include "../NyLPC_cHttpdConnection.h"
#include "NyLPC_cModRomFiles.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * ファイルアップロードの為の基本シーケンスを提供する抽象クラスです。
 * 継承クラスで_abstruct_function以下の関数に実体を設定して使います。
 */
typedef struct NyLPC_TcModFileIoBaseClass NyLPC_TcModFileIoBaseClass_t;

/*
 * Abstruct関数の定義
 */

/**
 * i_fnameの内容をi_body_parserの内容で更新します。
 * この関数はHTTPD_LOCKがかかった状態でコールされます。
 * 関数にはコネクションにHTTP応答を返却する実装をしてください。
 * @return
 * 処理が失敗した場合はFALSEを返します。FALSEを返すとCallerはHTTPセッションを切断します。
 */
typedef NyLPC_TBool (*NyLPC_cModFileIoBaseClass_uploadFunction)(NyLPC_TcHttpdConnection_t* i_connection,const NyLPC_TChar* i_fname,NyLPC_TcHttpBodyParser_t* i_body_parser);
/**
 * i_fnameのファイルを作成します。
 * この関数はHTTPD_LOCKがかかった状態でコールされます。
 * 関数にはコネクションにHTTP応答を返却する実装をしてください。
 * @return
 * 処理が失敗した場合はFALSEを返します。FALSEを返すとCallerはHTTPセッションを切断します。
 */
typedef NyLPC_TBool (*NyLPC_cModFileIoBaseClass_createFunction)(NyLPC_TcHttpdConnection_t* i_connection,const NyLPC_TChar* i_fname);
/**
 * i_fnameのファイルを削除します。
 * この関数はHTTPD_LOCKがかかった状態でコールされます。
 * 関数にはコネクションにHTTP応答を返却する実装をしてください。
 * @return
 * 処理が失敗した場合はFALSEを返します。FALSEを返すとCallerはHTTPセッションを切断します。
 */
typedef NyLPC_TBool (*NyLPC_cModFileIoBaseClass_removeFunction)(NyLPC_TcHttpdConnection_t* i_connection,const NyLPC_TChar* i_fname);

/**
 * Abstruct関数テーブル
 */
struct NyLPC_TcModFileIoBaseClass_AbstructFunction
{
    NyLPC_cModFileIoBaseClass_uploadFunction upload;
    NyLPC_cModFileIoBaseClass_createFunction create;
    NyLPC_cModFileIoBaseClass_removeFunction remove;
};

/**
 * クラス構造体
 */
struct NyLPC_TcModFileIoBaseClass
{
    NyLPC_TcModRomFiles_t super;
    /**
     * 処理関数へのポインタ構造体
     */
    struct NyLPC_TcModFileIoBaseClass_AbstructFunction _abstruct_function;
};

/**
 * コンストラクタ。
 */
void NyLPC_cModFileIoBaseClass_initialize(NyLPC_TcModFileIoBaseClass_t* i_inst,const NyLPC_TChar* i_ref_root_path);
void NyLPC_cModFileIoBaseClass_finalize(NyLPC_TcModFileIoBaseClass_t* i_inst);

/**
 * モジュールがコネクションをハンドリングできるかを返します。
 */
NyLPC_TBool NyLPC_cModFileIoBaseClass_canHandle(NyLPC_TcModFileIoBaseClass_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection);
/**
 * モジュールを実行します。
 * この関数は、リクエストを解析して、抽象関数upload,create,deleteの何れかに該当する場合はそれを呼び出します。
 * @return
 * 処理に成功したか
 */
NyLPC_TBool NyLPC_cModFileIoBaseClass_execute(NyLPC_TcModFileIoBaseClass_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CMODREMOTEMCU_H_ */
