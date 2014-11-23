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
#ifndef NYLPC_CMODUPNPDEVICE_H_
#define NYLPC_CMODUPNPDEVICE_H_

#include "../../upnp/NyLPC_cUPnP.h"
#include "NyLPC_cModRomFiles.h"

#include "NyLPC_stdlib.h"
#include "NyLPC_http.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * NyLPC_cUPnPクラスと協調して、UPnPDeviceのHTTP接続を処理します。
 * 提供するサービスは以下の通りです。
 * <ul>
 * <li>DeviceDescription - /[:i_path:]/ddesc</li>
 * <li>CONTROL - /[:i_path:]/control/[:serviceIndex:]</li>
 * <li>EVENT - /[:i_path:]/control/[:serviceIndex:]</li>
 * </ul>
 * [:serviceIndex:]はdeviceDescriptionを直列化して生成する通し番号です。
 * サービスを示す一意のIDです。16BITのHEX値です。
 * 上位8bitがデバイスインデックス、下位8bitがサービスインデックスです。
 * 例えば以下の構造のデバイスでは、サービス番号は以下のようになります。
 * <pre>
 * dev:a        0x1n (0<=n<=0xf)
 *  +-dev:b     0x2n (0<=n<=0xf)
 *  |+-dev:c    0x3n (0<=n<=0xf)
 *  +-dev:d     0x4n (0<=n<=0xf)
 * </pre>
 */
typedef struct NyLPC_TcModUPnPDevice NyLPC_TcModUPnPDevice_t;

struct NyLPC_TcModUPnPDevice
{
    NyLPC_TcModRomFiles_t super;
    const NyLPC_TcUPnP_t* _ref_upnp;
};

/**
 * コンストラクタ
 * @param i_inst
 * 初期化するインスタンスのポインタ
 * @param i_ref_upnp
 * 参照するUPnPデバイスインスタンス
 *
 */
void NyLPC_cModUPnPDevice_initialize(NyLPC_TcModUPnPDevice_t* i_inst,const NyLPC_TcUPnP_t* i_ref_upnp);

/**
 *
 */
void NyLPC_cModUPnPDevice_finalize(NyLPC_TcModUPnPDevice_t* i_inst);

/**
 * モジュールがコネクションをハンドリングできるかを返します。
 */
NyLPC_TBool NyLPC_cModUPnPDevice_canHandle(NyLPC_TcModUPnPDevice_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection);



/**
 * モジュールを実行します。
 */
NyLPC_TBool NyLPC_cModUPnPDevice_execute(NyLPC_TcModUPnPDevice_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
