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
#ifndef NYLPC_CMODMIMICSETTING_H_
#define NYLPC_CMODMIMICSETTING_H_
#include "NyLPC_http.h"
#include "../NyLPC_cHttpdConnection.h"
#include "NyLPC_cModRomFiles.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * MiMicの動作設定CGIです。MiMicの設定値をオンチップフラッシュへ保存/読み出しします。
 * uipServiceはNyLPC_TcNetConfig_tを継承したインスタンスで初期化してください。
 *
 * JSONAPIとして、以下のAPIを提供します。
 * ./setup.api?c=[update|get]&p=[:param:]
 * MiMicの動作設定と取得を行います。
 * c=get 現在の状態をjson形式で返却する。
 * {
 *      application: [:string:],
 *      mac00010203: [:HEX32:],
 *      mac0405xxxx: [:HEX32:],
 *      ip: [:HEX32:],
 *      mask: [:HEX32:],
 *      droute: [:HEX32:],
 *
 *      port: [:HEX16:],
 *      access\":%u}",
 * c=update pパラメタ/hostの内容でFlashをアップデートする。
 *      pパラメタは32bitの16進数文字列。
 *      [ 0] emac_0123      ビックエンディアン48bit値+パディング16bit
 *      [ 1] emac_45xx      :
 *          [emac4][emac5][x][x]
 *      [ 2] ipv4_flags     IPV4設定フラグ
 *      [ 3] ipv4_ip        IPアドレス。32bit値。ビックエンディアン
 *      [ 4] ipv4_mask      サブネットマスク。32bit値。ビックエンディアン
 *      [ 5] ipv4_deoute    defaultrootアドレス。32bit値。ビックエンディアン
 *      [ 6] service_flag
 *          See NyLPC_cNetConfig.h
 *      [ 7] http_param     HTTPサービスポート番号。2桁のHEX値である。ビックエンディアン。値16bit、パディング16bit
 *          [port_h][port_l][x][x]
 */
typedef struct NyLPC_TcModMiMicSetting NyLPC_TcModMiMicSetting_t;


struct NyLPC_TcModMiMicSetting
{
    NyLPC_TcModRomFiles_t super;
};

/**
 * コンストラクタ。
 */
void NyLPC_cModMiMicSetting_initialize(NyLPC_TcModMiMicSetting_t* i_inst,const NyLPC_TChar* i_ref_root_path);
void NyLPC_cModMiMicSetting_finalize(NyLPC_TcModMiMicSetting_t* i_inst);

/**
 * モジュールがコネクションをハンドリングできるかを返します。
 */
NyLPC_TBool NyLPC_cModMiMicSetting_canHandle(NyLPC_TcModMiMicSetting_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection);
/**
 * モジュールを実行します。
 * @return 処理に成功したか
 */
NyLPC_TBool NyLPC_cModMiMicSetting_execute(NyLPC_TcModMiMicSetting_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CHTTPSHORTHTTPHEADERPARSER_H_ */
