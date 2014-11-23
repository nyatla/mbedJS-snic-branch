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
#ifndef NYLPC_CNETCONFIG_H_
#define NYLPC_CNETCONFIG_H_

#include "NyLPC_stdlib.h"
#include "NyLPC_netif.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NyLPC_TcNetConfig_HOSTNAME_LEN 32
/**
 * クラス型を定義します。
 * NyLPC_cNetConfigクラスは、NyLPC_NetConfigの初期化データを保持します。
 * 初期化データのロード・セーブ機能を提供します。
 */
typedef struct NyLPC_TcNetConfig NyLPC_TcNetConfig_t;


/**
 * NyLPC_TcIPv4Configクラスの継承クラスです。
 * IPv4設定にネットワーク設定項目を加えます。
 * 加えられる項目は全て[RECOMMEND]パラメータです。
 */
struct NyLPC_TcNetConfig
{
    NyLPC_TcIPv4Config_t super;
    /**
     * ホスト名
     */
    NyLPC_TChar hostname[NyLPC_TcNetConfig_HOSTNAME_LEN];
    /**
     * tcp初期設定モードのフラグ値
    　* NyLPC_TcNetConfig_IPV4_FLAG_Xの組み合わせ
     * bit 01:IP初期設定のモード. 0:Manual指定,1:DHCP指定,2:AutoIP指定,3:APIPA指定
     */
    NyLPC_TUInt32 tcp_mode;
    struct{
        /**
         * サービスのフラグセット。
         * NyLPC_TcNetConfig_SERVICE_FLAG_xの組み合わせ
         */
        NyLPC_TUInt32 flags;
        NyLPC_TUInt16 http_port;
        NyLPC_TUInt16 padding;
    }services;

    /** インタフェイス層の設定 */
};
#define NyLPC_TcNetConfig_IPV4_FLAG_MODE_MASK   0x00000003
#define NyLPC_TcNetConfig_IPV4_FLAG_MODE_MANUAL 0x00000000
#define NyLPC_TcNetConfig_IPV4_FLAG_MODE_DHCP   0x00000001
#define NyLPC_TcNetConfig_IPV4_FLAG_MODE_AUTOIP 0x00000002
#define NyLPC_TcNetConfig_IPV4_FLAG_MODE_APIPA (NyLPC_TcNetConfig_IPV4_FLAG_MODE_DHCP|NyLPC_TcNetConfig_IPV4_FLAG_MODE_AUTOIP)
/*--------------------------------------------------
 * NyLPC_TcNetConfig.services.flags
 --------------------------------------------------*/

/**
 * MDNSサービスの有効・無効(いまのところビットパターンなので注意！)
 */
#define NyLPC_TcNetConfig_SERVICE_FLAG_MDNS 0x00000001
#define NyLPC_TcNetConfig_SERVICE_FLAG_UPNP 0x00000002


/**
 * フラッシュメモリから設定値を読み出して、インスタンスを初期化します。
 * @param i_is_factory_default
 * 出荷時設定を読み出すかを設定します。
 */
void NyLPC_cNetConfig_initialize(NyLPC_TcNetConfig_t* i_inst,NyLPC_TBool i_is_factory_default);


/**
 * インスタンスを終期化します。
 */
#define NyLPC_cNetConfig_finalize(i_inst);

#define NyLPC_cNetConfig_setIpAddr(i_inst,ip1,ip2,ip3,ip4) NyLPC_TIPv4Addr_set(&((i_inst)->super.ip_addr),(ip1),(ip2),(ip3),(ip4));

/**
 * Set IPv4 network mask value to instance.
 */
#define NyLPC_cNetConfig_setNetMask(i_inst,ip1,ip2,ip3,ip4) NyLPC_TIPv4Addr_set(&((i_inst)->super.netmask),(ip1),(ip2),(ip3),(ip4));

/**
 * Set IPv4 default gateway address to instance.
 */
#define NyLPC_cNetConfig_setGateway(i_inst,ip1,ip2,ip3,ip4) NyLPC_TIPv4Addr_set(&((i_inst)->super.dr_addr),(ip1),(ip2),(ip3),(ip4));

/**
 * Set ethernet mac address to instance.
 */
#define NyLPC_cNetConfig_setEmac(i_inst,a1,a2,a3,a4,a5,a6) NyLPC_TEthAddr_set(&((i_inst)->interface_setting.ethernet.eth_mac),(a1),(a2),(a3),(a4),(a5),(a6));



#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* NYLPC_CNETCONFIG_H_ */
