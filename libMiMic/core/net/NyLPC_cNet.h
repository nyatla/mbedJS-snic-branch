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
#ifndef NYLPC_CNET_H_
#define NYLPC_CNET_H_

#include "NyLPC_stdlib.h"
#include "NyLPC_uipService.h"
#include "NyLPC_cNetConfig.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * クラス型を定義します。
 */
typedef struct NyLPC_TcNet NyLPC_TcNet_t;




struct NyLPC_TcNet
{
    int dummy;
};

/**
 * ネットワークを初期化する。
 */
void NyLPC_cNet_initialize(NyLPC_TcNet_t* i_inst);

#define NyLPC_cNet_finalize(inst)
/**
 * ネットワークサービスを開始します。
 * サービスは停止中でなければなりません。
 * 関数は、ネットワークアダプタの値を元にNyLPC_cMiMicEnv_PlatformName変数の値を更新します。
 * @param i_ref_config
 * Networkコンフィギュレーション変数。このオブジェクトはcNetをstopするまで維持すること。
 */
void NyLPC_cNet_start(NyLPC_TcNet_t* i_inst,const NyLPC_TcNetConfig_t* i_ref_config);
/**
 * ネットワークスタックを停止します。
 * サービスは開始中でなければなりません。
 * start関数で開始済である必要があります。
 * この関数をコールする前に、全てのTCPソケットを閉じ、非同期なソケット操作を停止してください。
 */
void NyLPC_cNet_stop(NyLPC_TcNet_t* i_inst);

/**
 * NyLPC_TcIPv4Config_tをDHCPで更新します。
 * この関数をコールする時は、サービスは停止中でなければなりません。
 * @param i_cfg
 * 更新するi_cfg構造体。
 * emac,default_mssは設定済である必要があります。他のフィールド値は不定で構いません。
 * 更新されるフィールドは、ip,netmast,default_rootの3つです。
 * @return
 * 更新に成功した場合TRUE
 */
NyLPC_TBool NyLPC_cNet_requestAddrDhcp(NyLPC_TcNet_t* i_net,NyLPC_TcIPv4Config_t* i_cfg,NyLPC_TInt16 i_repeat);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* NYLPC_CNET_H_ */
