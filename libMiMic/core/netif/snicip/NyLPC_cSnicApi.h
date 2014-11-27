/*
 * NyLPC_cSnicApi.h
 *
 *  Created on: 2014/11/25
 *      Author: nyatla
 */

#ifndef NYLPC_CSNICAPI_H_
#define NYLPC_CSNICAPI_H_
#include "nyLPC_stdlib.h"
#include "nyLPC_netif.h"
#include "NyLPC_Snic_types.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct NyLPC_TcSnicApi NyLPC_TcSnicApi_t;

struct NyLPC_TSnicApi_ApiInitializeResult{
	NyLPC_TUInt16 default_rx_size;
	NyLPC_TUInt16 max_udp;
};

struct NyLPC_TSnicApi_CreateTcpConnectionServerResult{
	NyLPC_TUInt16 buffer_size;
	NyLPC_TUInt8 maximum_client;
};
struct NyLPC_TSnicApi_SendFromSocketResult{
	NyLPC_TUInt16 number_of_byte_sent;
};
struct NyLPC_TSnicApi_SendUdpPacketResult{
	NyLPC_TUInt16 number_of_byte_sent;
};
struct NyLPC_TSnicApi_StartUdpReceiveOnSocketResult{
	NyLPC_TUInt16 recv_buf_size;
};

struct NyLPC_TcSnicApi
{
	volatile NyLPC_TUInt8 _signal;		//シグナルの代用品
	NyLPC_TUInt8 last_cmd;				//最後に送信したコマンド
	NyLPC_TUInt8 last_status;			//最後に実行したコマンドのステータス
	NyLPC_TUInt8 last_seq;
	/** 関数との通信用*/
	union{
		union{
			NyLPC_TUInt16 u16[2];
			NyLPC_TUInt8 u8[4];
		}rawbuf;
		struct NyLPC_TSnicApi_ApiInitializeResult* api_inisialization;
		struct NyLPC_TSnicApi_CreateTcpConnectionServerResult* create_tcp_connection_server;
		struct NyLPC_TSnicApi_SendFromSocketResult* send_from_socket;
	}_response;
};


NyLPC_TBool NyLPC_cSnicApi_initialize(NyLPC_TcSnicApi_t* i_inst);
void NyLPC_cSnicApi_finalize(NyLPC_TcSnicApi_t* i_inst);


NyLPC_TBool NyLPC_cSnicApi_apiInitalization(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt16 i_buf_len,struct NyLPC_TSnicApi_ApiInitializeResult* i_result);
NyLPC_TBool NyLPC_cSnicApi_cleanUp(NyLPC_TcSnicApi_t* i_inst);
NyLPC_TBool NyLPC_cSnicApi_configure(NyLPC_TcSnicApi_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,const struct NyLPC_TIPv4Addr* i_netmask,const struct NyLPC_TIPv4Addr* i_gateway);
NyLPC_TBool NyLPC_cSnicApi_createTcpSocket(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8 i_cmd,NyLPC_TUInt8 i_bind_option,const struct NyLPC_TIPv4Addr* i_local_ip,NyLPC_TUInt16 i_local_port,NyLPC_TUInt8* i_socket);
NyLPC_TBool NyLPC_cSnicApi_createUdpSocket(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8 i_cmd,NyLPC_TUInt8 i_bind_option,const struct NyLPC_TIPv4Addr* i_local_ip,NyLPC_TUInt16 i_local_port,NyLPC_TUInt8* i_socket);

NyLPC_TBool NyLPC_cSnicApi_createTcpConnectionServer(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8 i_socket,NyLPC_TUInt16 i_recv_buf_size,NyLPC_TUInt8 i_maximum_connection,struct NyLPC_TSnicApi_CreateTcpConnectionServerResult* i_result);




#ifdef __cplusplus
}
#endif

#endif /* NYLPC_CSNICAPI_H_ */
