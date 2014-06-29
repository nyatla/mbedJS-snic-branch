/*
 * cConfiglationStorage.h
 *
 *  Created on: 2011/10/18
 *      Author: nyatla
 */

#include "NyLPC_stdlib.h"
#include "NyLPC_net.h"
#ifndef NYLPC_CCONFIGLATIONSTORAGE_H_
#define NYLPC_CCONFIGLATIONSTORAGE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * IPアドレスはネットワークオーダーで格納する。
 * 構造体は4バイトアライメントであること。
 */
struct NyLPC_TMiMicConfigulation{
    /** ROM焼検出用。0xFFFFFFFFを書く　*/
    NyLPC_TUInt32   fast_boot;
    /** ホスト名*/
    NyLPC_TChar     hostname[NyLPC_TcNetConfig_HOSTNAME_LEN];
    /** MACアドレスの下位4bit*/
    NyLPC_TUInt32   mac_00_01_02_03;
    /** MACアドレスの上位2bit*/
    NyLPC_TUInt32   mac_04_05_xx_xx;
    /*
     * IPv4設定
     */

    /**
     * 0-1bit 起動モード
     *  0:default,1:DHCP,2:AUTOIP,3:APIPA
     */
    NyLPC_TUInt32   ipv4_flags;
    /** IPV4アドレス*/
    NyLPC_TUInt32   ipv4_addr_net;
    NyLPC_TUInt32   ipv4_mask_net;
    NyLPC_TUInt32   ipv4_drut_net;

    /*
     * Service setting
     */

    /**
     * Service flags
     * 0:mdns ON/OFF
     */
    NyLPC_TUInt32   srv_flags;
    /** HTTPポート番号*/
    NyLPC_TUInt16   http_port;
    NyLPC_TUInt16   padding;

};

/**
 * ユーザー定義コンフィギュレーションを持つ場合にtrue
 * falseなら初期設定と捉えることも出来ます。
 */
NyLPC_TBool NyLPC_cMiMicConfiglation_hasUserConfigulation(void);

/**
 * ユーザコンフィギュレーションを更新する。
 * この関数は、RTOSが停止中に実行すること。
 * この関数は384バイト程度のスタックが必要です。
 */
NyLPC_TBool NyLPC_cMiMicConfiglation_updateConfigulation(const struct NyLPC_TMiMicConfigulation* i_congfiglation);
/**
 * コンフィギュレーション値を返す。
 * この関数は、RTOSが停止中に実行すること。
 */
const struct NyLPC_TMiMicConfigulation* NyLPC_cMiMicConfiglation_loadFromFlash(void);
const struct NyLPC_TMiMicConfigulation* NyLPC_cMiMicConfiglation_loadFactoryDefault(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CCONFIGLATIONSTORAGE_H_ */
