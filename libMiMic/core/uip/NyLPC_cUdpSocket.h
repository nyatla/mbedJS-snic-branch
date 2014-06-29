/*
 * NyLPC_cUdpSocket.h
 *
 *  Created on: 2013/05/20
 *      Author: nyatla
 */

#ifndef NYLPC_CUDPSOCKET_H_
#define NYLPC_CUDPSOCKET_H_
#include "NyLPC_uip.h"
#include "NyLPC_os.h"
#include "NyLPC_cBaseSocket.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct NyLPC_TcUdpSocket NyLPC_TcUdpSocket_t;
/**
 * 受信情報を格納する構造体
 */
struct NyLPC_TIPv4RxInfo
{
    NyLPC_TUInt16 size;//パケットサイズ
    NyLPC_TUInt16 port;//受信ポート
    NyLPC_TUInt16 peer_port;//PEERポート
    struct NyLPC_TIPv4Addr ip;//受信IP
    struct NyLPC_TIPv4Addr peer_ip;//PEERIP
};

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

/**
 * 受信時に非同期にコールされるハンドラ
 * UIPサービスタスクが実行する。
 * @return
 * TRUEならパケットを受信キューへ追加する。FALSEならパケットを受信キューへ追加しない。
 */
typedef NyLPC_TBool (*NyLPC_TcUdpSocket_onRxHandler)(NyLPC_TcUdpSocket_t* i_inst,const void* i_buf,const struct NyLPC_TIPv4RxInfo* i_info);
/**
 * 非同期にコールされるハンドラ。
 * UIPサービスタスクが実行する。
 */
typedef void (*NyLPC_TcUdpSocket_onPeriodic)(NyLPC_TcUdpSocket_t* i_inst);

struct NyLPC_TcUdpSocket
{
    NyLPC_TcBaseSocket_t _super;
    //この変数は、uipタスクの実行する関数のみが変更する。
    struct uip_udp_conn uip_udp_conn;
    NyLPC_TcFifoBuffer_t rxbuf;
    NyLPC_TcMutex_t* _smutex;
    struct{
        /**　受信ハンドラ。サービス実装に使用する。*/
        NyLPC_TcUdpSocket_onRxHandler rx;
        /** 定期実行ハンドラ。サービス実装に使用する。最低保障周期は1s*/
        NyLPC_TcUdpSocket_onPeriodic periodic;
    }as_handler;
};





/**
 * @param i_rbuf
 * 受信バッファアアドレス。サイズは、(最大受信サイズ-4バイト)*キュー数で計算します。
 * @param i_rbuf_len
 * 受信バッファのサイズ。
 */
NyLPC_TBool NyLPC_cUdpSocket_initialize(NyLPC_TcUdpSocket_t* i_inst,NyLPC_TUInt16 i_port,void* i_rbuf,NyLPC_TUInt16 i_rbuf_len);
void NyLPC_cUdpSocket_finalize(NyLPC_TcUdpSocket_t* i_inst);

/**
 * マルチキャストアドレスに参加する。
 * @param i_addr
 * 参加するマルチキャストグループを指定する。
 * 同じマルチキャスとグループに参加できるのは、システムの中で１つに限られます。
 * 0を指定した場合、マルチキャスとグループから離脱します。
 */
void NyLPC_cUdpSocket_joinMulticast(NyLPC_TcUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr);

/**
 * ブロードキャストに参加する。
 */
void NyLPC_cUdpSocket_setBroadcast(NyLPC_TcUdpSocket_t* i_inst);


/**
 * この関数は、ソケットの受信バッファの読み取り位置と、読み出せるデータサイズを返却します。
 * 関数はポインターを返却するだけで、バッファの読み取り位置をシークしません。
 * シークするにはNyLPC_cTcpSocket_pseekを使います。
 */
NyLPC_TInt32 NyLPC_cUdpSocket_precv(NyLPC_TcUdpSocket_t* i_inst,const void** o_buf_ptr,const struct NyLPC_TIPv4RxInfo** o_info,NyLPC_TUInt32 i_wait_msec);
/**
 * 受信バッファを次のバッファまでシークします。
 */
void NyLPC_cUdpSocket_pseek(NyLPC_TcUdpSocket_t* i_inst);

/**
 * 送信バッファを割り当てて返します。
 * 割り当てたメモリは、releaseSendBuf関数か、psend関数を成功させて開放する必要があります。
 * @param i_hint
 * 取得したいメモリサイズをセットします。
 * 関数は要求サイズより小さいメモリを返すことがあります。
 */
void* NyLPC_cUdpSocket_allocSendBuf(NyLPC_TcUdpSocket_t* i_inst,NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_buf_size,NyLPC_TUInt32 i_wait_in_msec);

void NyLPC_cUdpSocket_releaseSendBuf(NyLPC_TcUdpSocket_t* i_inst,void* i_buf_ptr);

/**
 * 事前にAllocしたTxパケットを送信します。
 * このAPIはゼロコピー送信をサポートするためのものです。
 * @param i_buf_ptr
 * allocSendBufで取得したメモリを指定します。
 * @return
 * 関数が失敗した場合、i_buf_ptrは「開放されません。」
 */
NyLPC_TBool NyLPC_cUdpSocket_psend(NyLPC_TcUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_port,void* i_buf_ptr,int i_len);

/**
 * 最大送信サイズは1200バイトです。
 */
NyLPC_TInt32 NyLPC_cUdpSocket_send(NyLPC_TcUdpSocket_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_port,const void* i_buf_ptr,NyLPC_TInt32 i_len,NyLPC_TUInt32 i_wait_in_msec);

/**
 * 非同期パケットハンドラを設定する。
 */
#define NyLPC_cUdpSocket_setOnRxHandler(i_inst,i_handler) (i_inst)->as_handler.rx=i_handler;

/**
 * 非同期タイマ呼び出しハンドラを設定する。
 */
#define NyLPC_cUdpSocket_setOnPeriodicHandler(i_inst,i_handler) (i_inst)->as_handler.periodic=i_handler;

/**
 * ソケットのローカルIPのアドレスを返す。
 * 値はuipが動作中のみ有効。
 */
#define NyLPC_cUdpSocket_getSockIP(i_inst) (&(i_inst)->uip_udp_conn.lipaddr)

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* NYLPC_CUDPSOCKET_H_ */
