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

#ifndef NyLPC_MiMicIpNetIf_protected_H
#define NyLPC_MiMicIpNetIf_protected_H
#include "NyLPC_cMiMicIpNetIf.h"
#include "NyLPC_os.h"
#include "NyLPC_cIPv4Arp.h"
#include "NyLPC_cIPv4.h"
#include "NyLPC_cIPv4IComp.h"
#include "../driver/ethernet/EthDev.h"
#include "NyLPC_cMiMicIpTcpSocket.h"
#include "NyLPC_cMiMicIpUdpSocket.h"
#include "NyLPC_cMiMicIpTcpListener.h"

/**********************************************************************
 *
 * NyLPC_TcUipService_t
 *
 **********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//#define NyLPC_TcUipService_SIZE_OF_REPLY_BUF 128



struct NyLPC_TcMiMicIpNetIf
{
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
    struct NyLPC_TNetInterfaceInfo _netinfo;
};


/**　唯一のサービスインスタンス - Single service instance*/
extern NyLPC_TcMiMicIpNetIf_t* _NyLPC_TcMiMicIpNetIf_inst;

/**
 * @param i_mac_addr
 * システムで唯一のUIPサービスを初期化します。1度だけ実行できます。
 */
NyLPC_TBool NyLPC_cMiMicIpNetIf_initialize(NyLPC_TcMiMicIpNetIf_t* i_inst);

/**
 * サービスが初期化済みならtrueです。 - true if service was initialized.
 */
#define NyLPC_cMiMicIpNetIf_isInitService() (_NyLPC_TcMiMicIpNetIf_inst!=NULL)
/**
 * サービスが稼働中か返します。
 */
#define NyLPC_cMiMicIpNetIf_isRun() NyLPC_TUInt16_isBitOn(_NyLPC_TcMiMicIpNetIf_inst->_status,NyLPC_TcMiMicIpNetIf_STATUSBIT_IS_RUNNING)


/**********************************************************************
 * コントロールビットの定義
 **********************************************************************/
//サービスが実行中の場合1
#define NyLPC_TcMiMicIpNetIf_STATUSBIT_IS_RUNNING 0
#define NyLPC_TcMiMicIpNetIf_ORDER_START          1
#define NyLPC_TcMiMicIpNetIf_ORDER_STOP           2



#define INST_TYPE_NyLPC_Unknown 0
#define INST_TYPE_NyLPC_TcTcpListener 1
#define INST_TYPE_NyLPC_TcTcpSocket 2


/**********************************************************************
 *
 * IPスタックからコールする関数群
 *
 **********************************************************************/

/**
 * NyLPC_cUipService_allocTxBufが返却するメモリサイズ。
 *
 */
#define NyLPC_cMiMicIpNetIf_SYS_TX_BUF_SIZE (64-sizeof(struct NyLPC_TEthernetIIHeader))


/**
 * IPv4パケットをネットワークに送信します。
 * この関数は、リエントラントを許容します。
 * @param i_eth_payload
 * NyLPC_cUipService_getTxFrame、または、NyLPC_cUipService_recvIPv4Rxで得たバッファに、
 * IPv4パケットを書きこんだものを指定してください。
 */
void NyLPC_cMiMicIpNetIf_sendIPv4Tx(void* i_eth_payload);

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
void* NyLPC_cMiMicIpNetIf_allocTxBuf(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size);

/**
 * システム用の小さな送信ペイロードメモリを返します。
 * この関数は、リエントラントを許容します。
 * 返却するメモリブロックのサイズが小さいこと、メモリが確実に返される点がNyLPC_cUipService_allocTxBufと異なります。
 * この関数が返すメモリのサイズは、NyLPC_cUipService_SYS_TX_BUF_SIZEの値です。
 * 関数を使用するコードでは開始時に１度だけNyLPC_cUipService_SYS_TX_BUF_SIZEの値を確認して下さい。
 */
void* NyLPC_cMiMicIpNetIf_allocSysTxBuf(void);

/**
 * allocTxbufで確保したメモリを開放します。
 */
void* NyLPC_cMiMicIpNetIf_releaseTxBuf(void* i_buf);

/**
 * 指定したIPアドレスを要求するARPリクエストを発行します。
 */
void NyLPC_cMiMicIpNetIf_sendArpRequest(const struct NyLPC_TIPv4Addr* i_addr);

/**
 * ARPテーブルに指定したIPがあるかを返します。
 */
NyLPC_TBool NyLPC_cMiMicIpNetIf_hasArpInfo(const struct NyLPC_TIPv4Addr* i_addr);


void NyLPC_cMiMicIpNetIf_releaseTcpSocketMemory(const NyLPC_TcMiMicIpTcpSocket_t* i_inst);
void NyLPC_cMiMicIpNetIf_releaseUdpSocketMemory(const NyLPC_TcMiMicIpUdpSocket_t* i_inst);
void NyLPC_cMiMicIpNetIf_releaseTcpListenerMemory(const NyLPC_TcMiMicIpTcpListener_t* i_inst);

NyLPC_TcMiMicIpTcpListener_t* NyLPC_cMiMicIpNetIf_getListenerByPeerPort(NyLPC_TUInt16 i_port);
NyLPC_TcMiMicIpUdpSocket_t* NyLPC_cMiMicIpNetIf_getMatchUdpSocket(NyLPC_TUInt16 i_lport);
NyLPC_TBool NyLPC_cMiMicIpNetIf_isClosedTcpPort(NyLPC_TUInt16 i_lport);
NyLPC_TcMiMicIpUdpSocket_t* NyLPC_cMiMicIpNetIf_getMatchMulticastUdpSocket(
    const struct NyLPC_TIPv4Addr* i_mcast_ip,NyLPC_TUInt16 i_lport);
NyLPC_TcMiMicIpTcpSocket_t* NyLPC_cMiMicIpNetIf_getMatchTcpSocket(
    NyLPC_TUInt16 i_lport,struct NyLPC_TIPv4Addr i_rip,NyLPC_TUInt16 i_rport);
void NyLPC_cMiMicIpNetIf_callPeriodic(void);
void NyLPC_cMiMicIpNetIf_callSocketStart(const NyLPC_TcIPv4Config_t* i_cfg);
void NyLPC_cMiMicIpNetIf_callSocketStop(void);





#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

