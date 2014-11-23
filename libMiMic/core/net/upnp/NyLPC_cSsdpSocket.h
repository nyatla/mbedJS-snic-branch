/*
 * NyLPC_cSsdpSocket.h
 *
 *  Created on: 2013/07/26
 *      Author: nyatla
 */

#ifndef NYLPC_CSSDPSOCKET_H_
#define NYLPC_CSSDPSOCKET_H_
#include "NyLPC_netif.h"
#include "NyLPC_UPnP_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * DeviceDescriptionに含まれる最大デバイス数
 */
#define NyLPC_TcSsdpSocket_MAX_DEVICES 5

/**
 * @bug
 * TTLが64(4であるべき)
 */
typedef struct NyLPC_TcSsdpSocket NyLPC_TcSsdpSocket_t;

struct NyLPC_TcSsdpSocket
{
	NyLPC_TiUdpSocket_t* _socket;
	NyLPC_TcStopwatch_t _periodic_sw;
	/**
	 * locationパス
	 */
	const NyLPC_TChar* location_path;
	/**
	 * DeviceDescriptionをホストするサーバアドレス
	 */
	NyLPC_TUInt16 location_port;
	NyLPC_TUInt8 number_of_device;
	/**
	 * bit0:SSDPサービスの開始要求
	 * bit1:SSDPサービスの停止要求
	 * bit2:SSDPサービスの実行状態(1:start,0:stop)
	 */
	 volatile NyLPC_TUInt8 _flags;
	/**
	 * デバイスレコードの配列へのポインタ。
	 * この値は、initialize関数でセットされたrootdeviceを水平展開したものです。
	 * 0番目のレコードはルートデバイスです。
	 */
	const struct NyLPC_TUPnPDevDescDevice* ref_device_record[NyLPC_TcSsdpSocket_MAX_DEVICES];
};

/**
 * DeviceRecordを参照したSSDPソケットインスタンスを生成します。
 * この関数はuipServiceが開始する前に実行してください。
 * @param　i_inst
 * @param i_ref_dev_record
 * 0番目のデバイスがルートデバイス、以降は embeddedデバイスになります。
 * このオブジェクトはインスタンスが破棄されるまでの間維持してください。
 * @param i_number_of_devices
 * @param i_server_port
 * @param i_ref_location_path
 * deviceDescriptionのホストパスを設定します。
 * 例えば"upnp"の場合、locationはhttp://[:ip:]:[:port:]/upnp/ddesc.xmlとなります。
 */
void NyLPC_cSsdpSocket_initialize(
		NyLPC_TcSsdpSocket_t* i_inst,
		const struct NyLPC_TUPnPDevDescDevice* i_ref_dev_record,
		NyLPC_TUInt16 i_server_port,const NyLPC_TChar* i_ref_location_path);

void NyLPC_cSsdpSocket_finalize(NyLPC_TcSsdpSocket_t* i_inst);
/**
 * SSDPサービスを開始します。
 */
void NyLPC_cSsdpSocket_start(NyLPC_TcSsdpSocket_t* i_inst);
/**
 * SSDPサービスを停止します。
 */
void NyLPC_cSsdpSocket_stop(NyLPC_TcSsdpSocket_t* i_inst);


#ifdef __cplusplus
}
#endif /* __cplusplus */













#endif /* NYLPC_CSSDPSOCKET_H_ */
