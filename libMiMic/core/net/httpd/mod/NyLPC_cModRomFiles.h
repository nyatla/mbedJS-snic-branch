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
#ifndef NYLPC_CMODROMFILES_H_
#define NYLPC_CMODROMFILES_H_
#include "NyLPC_http.h"
#include "../NyLPC_cHttpdConnection.h"
#include "NyLPC_cModUrl.h"
#include "NyLPC_utils.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * ROMFileDataに格納したファイルデータを返すモジュールクラスです。
 * モジュールに登録したファイルデータは、initializeに登録したルートパスが一致したときに動作します。
 * 例えばrootpathが"/test/"の場合に、"/test/my.jpg"のリクエストが有る場合、
 * "my.jpg"が登録されていればそれを返します。登録されていなければ404エラーを返します。
 * プレフィクスを除いたURLの長さは32-1文字です。
 */
typedef struct NyLPC_TcModRomFiles NyLPC_TcModRomFiles_t;


struct NyLPC_TcModRomFiles
{
    NyLPC_TcModUrl_t super;
    /** ルートパス*/
    const NyLPC_TChar* _ref_root_path;
    /** ROMFileのデータ*/
    const struct NyLPC_TRomFileData* _data;
    /** ROMFILEデータの数*/
    int _num_of_data;
};

/**
 * コンストラクタ。
 * @param i_ref_path
 * 16文字以内のルートパスを指定します。文字列は外部参照です。インスタンスをfinalizeするまで維持してください。
 * path:= {.+}{/(.+)}?
 * @param i_ref_root_path
 * Must set static variable or hold value until instance is finished.
 * 静的に宣言された文字列を指定してください。
 */
void NyLPC_cModRomFiles_initialize(NyLPC_TcModRomFiles_t* i_inst,const NyLPC_TChar* i_ref_root_path,const struct NyLPC_TRomFileData* i_data,int i_num_of_data);
void NyLPC_cModRomFiles_finalize(NyLPC_TcModRomFiles_t* i_inst);

/**
 * モジュールがコネクションをハンドリングできるかを返します。
 */
NyLPC_TBool NyLPC_cModRomFiles_canHandle(NyLPC_TcModRomFiles_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection);
/**
 * モジュールを実行します。canHandleがtrueの時だけ実行できます。
 * @return
 * 処理が成功すればtrue。失敗したらfalse
 */
NyLPC_TBool NyLPC_cModRomFiles_execute(NyLPC_TcModRomFiles_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection);






#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CMODROMFILES_H_ */
