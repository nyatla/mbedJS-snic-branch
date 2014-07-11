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
 * このファイルは、NyLPC全体に関わる、コンフィギュレーション定数を宣言します。
 */
#ifndef NyLPC_config_h
#define NyLPC_config_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

////////////////////////////////////////////////////////////////////////////////////////////////
// ENDIAN
////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * 定義値です。LITTLE ENDIANを表します。
 */
#define NyLPC_ENDIAN_LITTLE 0
/**
 * 定義値です。BIG ENDIANを表します。
 */
#define NyLPC_ENDIAN_BIG    1

/**
 * 定義値です。CPUのエンディアンを定義します。
 * この値は、NyLPClibに影響を及ぼします。
 */
#define NyLPC_ENDIAN NyLPC_ENDIAN_LITTLE
////////////////////////////////////////////////////////////////////////////////////////////////
// MCU
////////////////////////////////////////////////////////////////////////////////////////////////
#define NyLPC_MCU_UNKNOWN 1
#define NyLPC_MCU_LPC17xx 2
#define NyLPC_MCU_LPC4088 3

#define NyLPC_MCU NyLPC_MCU_LPC17xx


////////////////////////////////////////////////////////////////////////////////////////////////
// OS
////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *　環境定数です。WIN32環境でコンパイルすることを示します。
 */
#define NyLPC_ARCH_WIN32 1
/**
 *　環境定数です。FREERTOS環境でコンパイルすることを示します。
 */
#define NyLPC_ARCH_FREERTOS 2
/**
 * 環境定数です。MbedRTOR環境でコンパイルすることを示します。
 */
#define NyLPC_ARCH_MBEDRTOS 3

/**
 * 環境定数です。アーキテクチャを選択します。NyLPC_ARCH_WIN32は、デバック用の定数です。
 * 通常は、NyLPC_ARCH_FREERTOSを使用します。
 */
#ifdef WIN_DEBUG
    #define NyLPC_ARCH NyLPC_ARCH_WIN32
#else
    #define NyLPC_ARCH NyLPC_ARCH_FREERTOS
#endif


////////////////////////////////////////////////////////////////////////////////////////////////
//予約定義値の修正
////////////////////////////////////////////////////////////////////////////////////////////////

#if NyLPC_ARCH==NyLPC_ARCH_WIN32
    //PACKED STRUCTの無効化
    #define PACK_STRUCT_END
    //OSタイプによりMCUを修正
#   undef NyLPC_MCU
#   define NyLPC_MCU NyLPC_MCU_UNKNOWN
#endif



////////////////////////////////////////////////////////////////////////////////////////////////
//デバック情報
////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * デバック用の宣言。
 * 定数が1の場合、メモリブロックをソースにして、TCPSocketをエミュレートします。
 * ソケットを使わずにデバックをしたいときに使います。
 */
#define NyLPC_CONFIG_cHttpStream_DEBUG 0

////////////////////////////////////////////////////////////////////////////////////////////////
//デバック情報
////////////////////////////////////////////////////////////////////////////////////////////////


#define NyLPC_cHttpdThread_SIZE_OF_THREAD_STACK (1024+512)
# define NyLPC_cHttpd_MAX_PERSISTENT_CONNECTION 1

/*固有プラットフォーム設定はここに記述します。*/
#undef NyLPC_ARCH
#define NyLPC_ARCH NyLPC_ARCH_MBEDRTOS

#ifdef TARGET_LPC4088
#undef NyLPC_MCU
#define NyLPC_MCU NyLPC_MCU_LPC4088
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
