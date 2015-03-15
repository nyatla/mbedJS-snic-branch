/*
 * NyLPC_cUdpSocket.h
 *
 *  Created on: 2013/05/20
 *      Author: nyatla
 */

#ifndef NYLPC_CMIMICIPUDPSOCKET_H_
#define NYLPC_CMIMICIPUDPSOCKET_H_

#include "NyLPC_os.h"
#include "../NyLPC_iUdpSocket.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NyLPC_TcMiMicIpUdpSocket_MAX_UDP_SOCKET 1

/**
 * Class struct
 */
typedef struct NyLPC_TcMiMicIpUdpSocket NyLPC_TcMiMicIpUdpSocket_t;



/**
 * Representation of a uIP UDP connection.
 */
struct uip_udp_conn{
    struct NyLPC_TIPv4Addr lipaddr;   /**< The IP address of the remote peer. */
    /** マルチキャスとアドレス(ZEROで無効)*/
    struct NyLPC_TIPv4Addr mcastaddr;
    NyLPC_TUInt16 lport;        /**< The local port number in network byte order. */
    NyLPC_TUInt8  flags;        /**フラグ*/
    NyLPC_TUInt8  padding;      /***/
};




struct NyLPC_TcMiMicIpUdpSocket
{
	struct NyLPC_TiUdpSocket _super;
    //この変数は、uipタスクの実行する関数のみが変更する。
    struct uip_udp_conn uip_udp_conn;
    NyLPC_TcFifoBuffer_t rxbuf;
    NyLPC_TcMutex_t* _smutex;
    struct{
        /**　受信ハンドラ。サービス実装に使用する。*/
    	NyLPC_TiUdpSocket_onRxHandler rx;
        /** 定期実行ハンドラ。サービス実装に使用する。最低保障周期は1s*/
    	NyLPC_TiUdpSocket_onPeriodicHandler periodic;
    }as_handler;
};






/**
 * @param i_rbuf
 * 受信バッファアアドレス。サイズは、(最大受信サイズ-4バイト)*キュー数で計算します。
 * @param i_rbuf_len
 * 受信バッファのサイズ。
 */
NyLPC_TBool NyLPC_cMiMicIpUdpSocket_initialize(NyLPC_TcMiMicIpUdpSocket_t* i_inst,NyLPC_TUInt16 i_port,void* i_rbuf,NyLPC_TUInt16 i_rbuf_len);
void NyLPC_cMiMicIpUdpSocket_finalize(NyLPC_TcMiMicIpUdpSocket_t* i_inst);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* NYLPC_CUDPSOCKET_H_ */
