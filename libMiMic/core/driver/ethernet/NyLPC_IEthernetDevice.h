/*
 * NyLPC_IEthernetDevice.h
 *
 *  Created on: 2011/12/06
 * MiMicのイーサネットドライバインタフェイスを定義する。
 */
#ifndef NyLPC_IEthernetDevice_h
#define NyLPC_IEthernetDevice_h
#include "NyLPC_stdlib.h"
#include "NyLPC_uipService.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NyLPC_TiEthernetDevice NyLPC_TiEthernetDevice_t;

/**非同期イベントのメッセージタイプ*/
typedef unsigned int NyLPC_TiEthernetDevice_EVENT;
/**TXが到達した*/
#define NyLPC_TiEthernetDevice_EVENT_ON_TX 1
/**RXが到達した*/
#define NyLPC_TiEthernetDevice_EVENT_ON_RX 2

/**
 * ヒント値。NyLPC_TcEthernetMM_HINT_CTRL_PACKETと同じ。
 */
#define NyLPC_TcEthernetMM_HINT_CTRL_PACKET 0

typedef void (*NyLPC_TiEthernetDevice_onEvent)(void* i_param,NyLPC_TiEthernetDevice_EVENT i_type);

/**
 * 送信バッフメモリのヘッダ。
 * この構造体は、TXバッファメモリブロックのヘッダーです。
 * TXバッファメモリブロックは、この構造体の後ろに、sizeに一致したメモリを連結したもので表現します。
 * <pre>
 * buffer=[struct NyLPC_TTxBufferHeader][n]
 * </pre>
 */
struct NyLPC_TTxBufferHeader
{
	//メモリブロックの参照カウンタ。
	NyLPC_TInt8  ref;
	//送信用にロックしたかを示すフラグ
	NyLPC_TUInt8 is_lock;
	//32ビット境界に合わせるためのパディング。
	NyLPC_TUInt16 padding;
};



/**
 * 受信キューの先頭にあるデータを返す。
 * 関数は、受信キューのポインタを操作しない。続けて読み出したとしても、同じポインターを返す。
 * 次のデータを得るには、nextRxEthFrameを呼び出す。
 * #制限として、返却したポインタの内容は、一時的に書き換え可としてください。（この制限は将来削除します。）
 * @return
 * 成功した場合、受信データを格納したバッファポインタ。返却値は、nextRxEthFrameをコールするまで有効である。
 * 存在しない場合NULL。
 */
#define NyLPC_iEthernetDevice_getRxEthFrame(i,p) (i)->getRxEthFrame(p)
typedef void* (*NyLPC_TiEthernetDevice_getRxEthFrame)(unsigned short* o_len_of_data);



/**
 * getRxEthFrameで得たメモリを破棄して、次のRXデータを準備する。
 * getRxEthFrameで返したメモリの内容の有効期間はここで終了するので注意すること。
 */
#define NyLPC_iEthernetDevice_nextRxEthFrame(i) (i)->nextRxEthFrame()
typedef void (*NyLPC_TiEthernetDevice_nextRxEthFrame)(void);


/**
 * 送信バッファを得る。
 * 関数は、i_hintで示されるサイズのメモリブロックを確保しようとするが、実際にはそれよりも小さいことがある。
 * @param i_hint
 * 確保してほしいメモリサイズ
 * @param o_size
 * 実際に割り当てたメモリのサイズ
 * @return
 * 割り当てたメモリブロックのヘッダ。
 */
#define NyLPC_iEthernetDevice_allocTxBuf(i,h,s) (i)->allocTxBuf((h),(s))
typedef struct NyLPC_TTxBufferHeader* (*NyLPC_TiEthernetDevice_allocTxBuf)(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size);


/**
 * alloc_TxBufで得た送信バッファを開放する。
 * 関数は、メモリブロックの参照カウンタを1減算する。
 */
#define NyLPC_iEthernetDevice_releaseTxBuf(i,b) (i)->releaseTxBuf(b)
typedef void (*NyLPC_TiEthernetDevice_releaseTxBuf)(struct NyLPC_TTxBufferHeader* i_buf);



/**
 * イーサネットパケット構造体i_bufの内容を送信する。
 * @param i_buf
 * allocTxBufで得たメモリか、初期化したNyLPC_TTxBufferHeaderメモリブロックを指定する。
 * 送信が終わるまでの間、メモリを開放してはならない。
 * #外部で確保したメモリについては、利用不能なケースがあるかもしれない。現在のMiMicでは、使用できることを前提としている。
 * @oaram i_size
 * i_bufの後ろに連結されているデータメモリの長さ
 */
#define NyLPC_iEthernetDevice_sendTxEthFrame(i,b,s) (i)->sendTxEthFrame((b),(s))
typedef void (*NyLPC_TiEthernetDevice_sendTxEthFrame)(struct NyLPC_TTxBufferHeader* i_buf,unsigned short i_size);


/**
 * この関数は、送信キューの状態を進行させるタイミングを与える。
 * 外部関数が、送信キューにセットしたパケットの送信待ちをするときに呼び出す。
 */
#define NyLPC_iEthernetDevice_processTx(i) (i)->processTx()
typedef void (*NyLPC_TiEthernetDevice_processTx)(void);



/**
 * この関数は、ドライバを開始します。
 * @param i_eth_addr
 * イーサネットアドレス
 * @param i_handler
 * 通知ハンドラ
 * @param i_param
 * 通知ハンドラに渡るパラメータ
 */
#define NyLPC_iEthernetDevice_start(i,a,h,p) (i)->start((a),(h),(p))
typedef NyLPC_TBool(*NyLPC_TiEthernetDevice_start)(const struct NyLPC_TEthAddr* i_eth_addr,NyLPC_TiEthernetDevice_onEvent i_handler,void* i_param);


/**
 * この関数はドライバを停止します。
 */
#define NyLPC_iEthernetDevice_stop(i) (i)->stop()
typedef void(*NyLPC_TiEthernetDevice_stop)(void);


/**
 * デバイス名を返します。
 */
#define NyLPC_iEthernetDevice_getDevicName(i) ((i)->_device_name)

/**
 * イーサネットデバイスのアクセスインターフェイス
 * イーサネットデバイスの管理するメモリブロック、ペリフェラル制御のインタフェイスを提供する。
 *
 */
struct TiEthernetDevice
{
	/** NyLPC_iEthernetDevice_getDevicNameで返却する値*/
	const char* _device_name;//pointer to device name.
	NyLPC_TiEthernetDevice_start start;
	NyLPC_TiEthernetDevice_stop stop;
	NyLPC_TiEthernetDevice_getRxEthFrame getRxEthFrame;
	NyLPC_TiEthernetDevice_nextRxEthFrame nextRxEthFrame; //nextRxEthFrame;
	NyLPC_TiEthernetDevice_allocTxBuf allocTxBuf;
	NyLPC_TiEthernetDevice_releaseTxBuf releaseTxBuf;
	NyLPC_TiEthernetDevice_sendTxEthFrame sendTxEthFrame;
	NyLPC_TiEthernetDevice_processTx processTx;
};




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
