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
/**
 * @file
 * このファイルは、NyLPC_cIPv4Configクラスを定義します。
 */
#ifndef NYLPC_CIPV4CONFIG_H_
#define NYLPC_CIPV4CONFIG_H_


#include "NyLPC_uip.h"

/**
 * クラス型を定義します。
 * NyLPC_cIPv4Configクラスは、IPと、下位のネットワーク層の設定を保持します。
 * 関連するオブジェクトが、ネットワーク設定を問い合わせる為に使います。
 */
typedef struct NyLPC_TcIPv4Config NyLPC_TcIPv4Config_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**********************************************************************
 *
 * class NyLPC_TcIPv4Config
 *
 **********************************************************************/




/**
 * NyLPC_TcIPv4Configクラスの構造体です。
 */
struct NyLPC_TcIPv4Config
{
    /** イーサネットアドレスを格納します。 */
    struct NyLPC_TEthAddr eth_mac;
    /** IPアドレスを格納します。Network orderです。 */
    struct NyLPC_TIPv4Addr ip_addr;
    /** ネットマスクを格納します。Network orderです。 */
    struct NyLPC_TIPv4Addr netmask;
    /** デフォルトゲートウェイアドレスを格納します。Network orderです。 */
    struct NyLPC_TIPv4Addr dr_addr;
    /** デフォルトMMSサイズです。送信パケットのMSS値、受信パケットのデフォルトMSS値として使います。 */
    NyLPC_TUInt16 default_mss;
};

#define NyLPC_TcIPv4Config_getEtherMac000120203(v)(((v)->eth_mac.addr[0]<<24)|((v)->eth_mac.addr[1]<<16)|((v)->eth_mac.addr[2]<<8)|((v)->eth_mac.addr[3]<<0))
#define NyLPC_TcIPv4Config_getEtherMac0405xxxx(v) (((v)->eth_mac.addr[4]<<24)|((v)->eth_mac.addr[5]<<16))

/**
 * コンストラクタです。
 * イーサネット用にコンフィギュレーションを初期化します。
 * @param i_inst
 * 初期化するインスタンスです。
 * @param i_ether_frame_len
 * イーサネットフレームのサイズ。この数値から、MSSのデフォルト値を計算します。
 */
void NyLPC_cIPv4Config_initialzeForEthernet(NyLPC_TcIPv4Config_t* i_inst,const struct NyLPC_TEthAddr* i_ether_addr,NyLPC_TUInt16 i_ether_frame_len);

/**
 * コピーコンストラクタ
 */
void NyLPC_cIPv4Config_initialzeCopy(NyLPC_TcIPv4Config_t* i_inst,const NyLPC_TcIPv4Config_t* i_src);


/**
 * デストラクタです。インスタンスを破棄して、確保している動的リソースを元に戻します。
 * @param i_inst
 * 破棄するインスタンスです。
 * initializeに成功したインスタンスだけが指定できます。
 */
#define NyLPC_cIPv4Config_finalize(i_inst)

/**
 * この関数は、IPのデフォルトゲートウェイを設定します。dr_addrの値を更新します。
 * @param i_inst
 * 操作するインスタンスです。
 * @param i_dr_addr
 * 設定するIPアドレスを格納したアドレスです。
 */
void NyLPC_cIPv4Config_setDefaultRoute(NyLPC_TcIPv4Config_t* i_inst,const struct NyLPC_TIPv4Addr* i_dr_addr);

/**
 * この関数は、ローカルIPアドレスとネットマスクを設定します。
 * @param i_inst
 * 操作するインスタンスです。
 * @param i_ipaddr
 * 設定するIPアドレスを格納したアドレスです。
 * @param i_netmask
 * 設定するネットマスクを格納したアドレスです。
 */
void NyLPC_cIPv4Config_setIp(NyLPC_TcIPv4Config_t* i_inst,const struct NyLPC_TIPv4Addr* i_ipaddr,const struct NyLPC_TIPv4Addr* i_netmask);

/**
 * この関数は、i_target_ipが、現在のIPアドレスに対するローカルアドレスであるかを返します。
 * @param i_inst
 * 操作するインスタンスです。
 * @param i_target_ip
 * 確認するIPアドレスです。
 * @return
 * i_target_ipがローカルIPアドレスなら、TRUEを返します。
 */
NyLPC_TBool NyLPC_cIPv4Config_isLocalIP(const NyLPC_TcIPv4Config_t* i_inst,const struct NyLPC_TIPv4Addr* i_target_ip);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
