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

#ifndef NyLPC_uipService_protected_H
#define NyLPC_uipService_protected_H
#include "NyLPC_cUipService.h"
#include "../driver/ethernet/EthDev.h"

/**********************************************************************
 *
 * NyLPC_TcUipService_t
 *
 **********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//#define NyLPC_TcUipService_SIZE_OF_REPLY_BUF 128



struct NyLPC_TcUipService
{
    const NyLPC_TcIPv4Config_t* _ref_config;
    volatile NyLPC_TUInt16  _status;            /**< ステータスビット*/
    NyLPC_TcSemaphore_t _emac_semapho;  /** EMACの制御用セマフォです。*/
    NyLPC_TcStopwatch_t _arp_sw;        /**<ARP用のストップウォッチ*/
    NyLPC_TcStopwatch_t _periodic_sw;   /**<周期実行用のストップウォッチ*/
    /** ARP処理インスタンス*/
    NyLPC_TcIPv4Arp_t _arp;
    /** TCPv4処理インスタンス*/
    NyLPC_TcIPv4_t _tcpv4;
    /** ICOMP処理インスタンス*/
    NyLPC_TcIPv4IComp_t _icomp;
    /** (Ethernetメモリ排他制御用)*/
    NyLPC_TcMutex_t _mutex;
    const struct TiEthernetDevice* _ethif;
};


/**　唯一のサービスインスタンス - Single service instance*/
extern NyLPC_TcUipService_t* _NyLPC_TcUipService_inst;

/**
 * サービスが初期化済みならtrueです。 - true if service was initialized.
 */
#define NyLPC_TcUipService_isInitService() (_NyLPC_TcUipService_inst!=NULL)
/**
 * サービスが稼働中か返します。
 */
#define NyLPC_cUipService_isRun() NyLPC_TUInt16_isBitOn(_NyLPC_TcUipService_inst->_status,NyLPC_TcUipService_STATUSBIT_IS_RUNNING)


/**********************************************************************
 * コントロールビットの定義
 **********************************************************************/
//サービスが実行中の場合1
#define NyLPC_TcUipService_STATUSBIT_IS_RUNNING 0
#define NyLPC_TcUipService_ORDER_START          1
#define NyLPC_TcUipService_ORDER_STOP           2



#define INST_TYPE_NyLPC_Unknown 0
#define INST_TYPE_NyLPC_TcTcpListener 1
#define INST_TYPE_NyLPC_TcTcpSocket 2


/**********************************************************************
 * cTcpSocketからコールする関数
 **********************************************************************/

/**
 * NyLPC_cUipService_allocTxBufが返却するメモリサイズ。
 *
 */
#define NyLPC_cUipService_SYS_TX_BUF_SIZE (64-sizeof(struct NyLPC_TEthernetIIHeader))


/**
 * IPv4パケットをネットワークに送信します。
 * この関数は、リエントラントを許容します。
 * @param i_eth_payload
 * NyLPC_cUipService_getTxFrame、または、NyLPC_cUipService_recvIPv4Rxで得たバッファに、
 * IPv4パケットを書きこんだものを指定してください。
 */
void NyLPC_cUipService_sendIPv4Tx(void* i_eth_payload);

/**
 * 送信ペイロードメモリを返します。
 * この関数は、リエントラントを許容します。
 * @param i_hint
 * 取得したいメモリサイズを指定します。(このサイズは、イーサネットヘッダのサイズを含みません。)
 * このサイズよりも小さなサイズが割り当てられることがあります。
 * @param o_size
 * 取得メモリのイーサネットヘッダを除いたペイロード部分の長さ
 * @return
 * 成功:IPペイロードのためのメモリブロックを返します。/失敗:NULL
 * メモリは、[TEthPacket][payload]の構造で返されます。
 */
void* NyLPC_cUipService_allocTxBuf(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size);

/**
 * システム用の小さな送信ペイロードメモリを返します。
 * この関数は、リエントラントを許容します。
 * 返却するメモリブロックのサイズが小さいこと、メモリが確実に返される点がNyLPC_cUipService_allocTxBufと異なります。
 * この関数が返すメモリのサイズは、NyLPC_cUipService_SYS_TX_BUF_SIZEの値です。
 * 関数を使用するコードでは開始時に１度だけNyLPC_cUipService_SYS_TX_BUF_SIZEの値を確認して下さい。
 */
void* NyLPC_cUipService_allocSysTxBuf(void);

/**
 * allocTxbufで確保したメモリを開放します。
 */
void* NyLPC_cUipService_releaseTxBuf(void* i_buf);

/**
 * 指定したIPアドレスを要求するARPリクエストを発行します。
 */
void NyLPC_cUipService_sendArpRequest(const struct NyLPC_TIPv4Addr* i_addr);

/**
 * ARPテーブルに指定したIPがあるかを返します。
 */
NyLPC_TBool NyLPC_cUipService_hasArpInfo(const struct NyLPC_TIPv4Addr* i_addr);






#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
