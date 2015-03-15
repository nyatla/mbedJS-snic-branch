/*
 * NyLPC_cSnicGeneralCmdProc.c
 *
 *  Created on: 2014/11/22
 *      Author: nyatla
 */
#include "NyLPC_cSnicNetIf_protected.h"
#include "NyLPC_cSnicApi.h"
#include "NyLPC_Snic_types.h"


#define UART_CMD_SID_RESPONSE_BIT_MASK	0x80
#define UART_CMD_SID_RESPONSE_CMD_MASK	(~UART_CMD_SID_RESPONSE_BIT_MASK)

#define UART_CMD_SID_SNIC_INIT_REQ                      0x00    // SNIC API initialization
#define UART_CMD_SID_SNIC_CLEANUP_REQ                   0x01    // SNIC API cleanup
#define UART_CMD_SID_SNIC_SEND_FROM_SOCKET_REQ          0x02    // Send from socket
#define UART_CMD_SID_SNIC_CLOSE_SOCKET_REQ              0x03    // Close socket
#define UART_CMD_SID_SNIC_SOCKET_PARTIAL_CLOSE_REQ		0x04    // Socket partial close
#define UART_CMD_SID_SNIC_GETSOCKOPT_REQ                0x05    // Get socket option
#define UART_CMD_SID_SNIC_SETSOCKOPT_REQ                0x06    // Set socket option
#define UART_CMD_SID_SNIC_SOCKET_GETNAME_REQ            0x07    // Get name or peer name
#define UART_CMD_SID_SNIC_SEND_ARP_REQ                  0x08    // Send ARP request
#define UART_CMD_SID_SNIC_GET_DHCP_INFO_REQ             0x09    // Get DHCP info
#define UART_CMD_SID_SNIC_RESOLVE_NAME_REQ              0x0A    // Resolve a host name to IP address
#define UART_CMD_SID_SNIC_IP_CONFIG_REQ                 0x0B    // Configure DHCP or static IP
#define UART_CMD_SID_SNIC_DATA_IND_ACK_CONFIG_REQ       0x0C    // ACK configuration for data indications
#define UART_CMD_SID_SNIC_TCP_CREATE_SOCKET_REQ         0x10    // Create TCP socket
#define UART_CMD_SID_SNIC_TCP_CREATE_CONNECTION_REQ     0x11    // Create TCP connection server
#define UART_CMD_SID_SNIC_TCP_CONNECT_TO_SERVER_REQ     0x12    // Connect to TCP server
#define UART_CMD_SID_SNIC_UDP_CREATE_SOCKET_REQ         0x13    // Create UDP socket
#define UART_CMD_SID_SNIC_UDP_START_RECV_REQ            0x14    // Start UDP receive on socket
#define UART_CMD_SID_SNIC_UDP_SIMPLE_SEND_REQ           0x15    // Send UDP packet
#define UART_CMD_SID_SNIC_UDP_SEND_FROM_SOCKET_REQ      0x16    // Send UDP packet from socket
#define UART_CMD_SID_SNIC_HTTP_REQ                      0x17    // Send HTTP request
#define UART_CMD_SID_SNIC_HTTP_MORE_REQ                 0x18    // Send HTTP more data request
#define UART_CMD_SID_SNIC_HTTPS_REQ                     0x19    // Send HTTPS request
#define UART_CMD_SID_SNIC_TCP_CREATE_ADV_TLS_SOCKET_REQ 0x1A    // Create advanced TLS TCP socket
#define UART_CMD_SID_SNIC_TCP_CREAET_SIMPLE_TLS_SOCKET_REQ  0x1B    // Create simple TLS TCP socket
#define UART_CMD_SID_SNIC_TCP_CONNECTION_STATUS_IND     0x20    // Connection status indication
#define UART_CMD_SID_SNIC_TCP_CLIENT_SOCKET_IND         0x21    // TCP client socket indication
#define UART_CMD_SID_SNIC_CONNECTION_RECV_IND           0x22    // TCP or connected UDP packet received indication
#define UART_CMD_SID_SNIC_UDP_RECV_IND                  0x23    // UCP packet received indication
#define UART_CMD_SID_SNIC_ARP_REPLY_IND                 0x24    // ARP reply indication
#define UART_CMD_SID_SNIC_HTTP_RSP_IND                  0x25    // HTTP response indication



#define UART_CMD_RES_SNIC_SUCCESS           0x00
#define UART_CMD_RES_SNIC_FAIL              0x01

#define LOCK_RESOURCE(i)
#define UNLOCK_RESOURCE(i)

NyLPC_TBool NyLPC_cSnicApi_initialize(NyLPC_TcSnicApi_t* i_inst)
{
	if(!NyLPC_cMutex_initialize(&i_inst->sock_res_mutex)){
		return NyLPC_TBool_FALSE;
	}

	return NyLPC_TBool_TRUE;
}
void NyLPC_cSnicApi_finalize(NyLPC_TcSnicApi_t* i_inst)
{
	NyLPC_cMutex_finalize(i_inst->sock_res_mutex);
}

/**ポインタテーブルからポインタを探す*/
NyLPC_TInt16 findPtr(void** i_tbl,NyLPC_TUInt16 i_size,const void* i_find)
{
	NyLPC_TUInt16 i;
	for(i=0;i<i_size;i++){
		if(i_tbl[i]==i_find){
			return i;
		}
	}
	return -1;
}








static void sendCmd(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8* i_data,NyLPC_TUInt16 i_data_len)
{
	i_inst->_signal=NyLPC_TUInt8_FALSE;
	NyLPC_cSnicNetIf_sendShortPayload(i_data,SNIC_UART_CMD_ID_SNIC,i_data_len);
	//シグナル待ち
	do{
		NyLPC_cThread_yield();
	}while(!i_inst->_signal);
	NyLPC_cSnicNetIf_unregieterObject(i_inst);
}

NyLPC_TBool NyLPC_cSnicApi_apiInitalization(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt16 i_buf_len,struct NyLPC_TSnicApi_ApiInitializeResult* i_result)
{
	NyLPC_TUInt8 data[4];
	i_inst->last_cmd=UART_CMD_SID_SNIC_INIT_REQ;
	i_inst->_response.api_inisialization=i_result;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	data[2]=(NyLPC_TUInt8)((i_buf_len>>8)&0xff);
	data[3]=(NyLPC_TUInt8)((i_buf_len>>0)&0xff);
	sendCmd(i_inst,data,4);
	return i_inst->last_status==UART_CMD_RES_SNIC_SUCCESS;
}

NyLPC_TBool NyLPC_cSnicApi_cleanUp(NyLPC_TcSnicApi_t* i_inst)
{
	NyLPC_TUInt8 data[2];
	i_inst->last_cmd=UART_CMD_SID_SNIC_CLEANUP_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	sendCmd(i_inst,data,2);
	return i_inst->last_status==UART_CMD_RES_SNIC_SUCCESS;
}
NyLPC_TBool NyLPC_cSnicApi_configure(NyLPC_TcSnicApi_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,const struct NyLPC_TIPv4Addr* i_netmask,const struct NyLPC_TIPv4Addr* i_gateway)
{
	NyLPC_TUInt8 data[2+2+4*3];
	i_inst->last_cmd=UART_CMD_SID_SNIC_IP_CONFIG_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	data[2]=0;//STA
	if(i_addr==NULL){
		data[3]=1;
		sendCmd(i_inst,data,4);
	}else{
		data[3]=0;
		memcpy(&data[4],&(i_addr->v),4);
		memcpy(&data[8],&(i_netmask->v),4);
		memcpy(&data[12],&(i_gateway->v),4);
		sendCmd(i_inst,data,16);
	}
	return i_inst->last_status==UART_CMD_RES_SNIC_SUCCESS;

}
static inline NyLPC_TBool _createSocketX(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8 i_cmd,NyLPC_TUInt8 i_bind_option,const struct NyLPC_TIPv4Addr* i_local_ip,NyLPC_TUInt16 i_local_port,NyLPC_TUInt8* i_socket)
{
	NyLPC_TUInt8 data[2+1+4+2];
	i_inst->last_cmd=i_cmd;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	data[2]=i_bind_option;
	if(i_bind_option==0){
		memcpy(&data[3],&(i_local_ip->v),4);
		data[7]=(NyLPC_TUInt8)((i_local_port>>8)&0xff);
		data[8]=(NyLPC_TUInt8)((i_local_port>>0)&0xff);
	}
	sendCmd(i_inst,data,(i_bind_option==0)?9:3);
	*i_socket=i_inst->_response.rawbuf.u8[0];
	return i_inst->last_status==UART_CMD_RES_SNIC_SUCCESS;
}
NyLPC_TBool NyLPC_cSnicApi_createTcpSocket(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8 i_bind_option,const struct NyLPC_TIPv4Addr* i_local_ip,NyLPC_TUInt16 i_local_port,NyLPC_TUInt8* i_socket)
{
	return _createSocketX(i_inst,UART_CMD_SID_SNIC_TCP_CREATE_SOCKET_REQ,i_bind_option,i_local_ip,i_local_port,i_socket);
}
NyLPC_TBool NyLPC_cSnicApi_createUdpSocket(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8 i_bind_option,const struct NyLPC_TIPv4Addr* i_local_ip,NyLPC_TUInt16 i_local_port,NyLPC_TUInt8* i_socket)
{
	return _createSocketX(i_inst,UART_CMD_SID_SNIC_UDP_CREATE_SOCKET_REQ,i_bind_option,i_local_ip,i_local_port,i_socket);
}
NyLPC_TBool NyLPC_cSnicApi_connectToServer(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8 i_socket,const struct NyLPC_TIPv4Addr* i_local_ip,NyLPC_TUInt16 i_local_port,NyLPC_TUInt16 i_recv_buf_size,NyLPC_TUInt16 i_timeout,struct NyLPC_TSnicApi_ConnectToServerResult* i_result)
{
	NyLPC_TUInt8 data[12];
	i_inst->last_cmd=UART_CMD_SID_SNIC_TCP_CONNECT_TO_SERVER_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	data[2]=i_socket;
	memcpy(&data[3],i_local_ip,4);
	data[7]=(NyLPC_TUInt8)((i_recv_buf_size>>8)&0xff);
	data[8]=(NyLPC_TUInt8)((i_recv_buf_size>>0)&0xff);
	data[9]=(NyLPC_TUInt8)((i_recv_buf_size>>8)&0xff);
	data[10]=(NyLPC_TUInt8)((i_recv_buf_size>>8)&0xff);
	data[11]=(NyLPC_TUInt8)((i_timeout<1000)?1:(i_timeout/1000));
	sendCmd(i_inst,data,12);
	if(i_result!=NULL){
		i_result->recv_buf_size=i_inst->_response.rawbuf.u16[0];
	}
	return i_inst->last_status==UART_CMD_RES_SNIC_SUCCESS;

}


NyLPC_TBool NyLPC_cSnicApi_createTcpConnectionServer(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8 i_socket,NyLPC_TUInt16 i_recv_buf_size,NyLPC_TUInt8 i_maximum_connection,struct NyLPC_TSnicApi_CreateTcpConnectionServerResult* i_result)
{
	NyLPC_TUInt8 data[6];
	i_inst->last_cmd=UART_CMD_SID_SNIC_TCP_CREATE_CONNECTION_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	i_inst->_response.create_tcp_connection_server=i_result;
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	data[2]=i_socket;
	data[3]=(NyLPC_TUInt8)((i_recv_buf_size>>8)&0xff);
	data[4]=(NyLPC_TUInt8)((i_recv_buf_size>>0)&0xff);
	data[5]=i_maximum_connection;
	sendCmd(i_inst,data,6);
	return i_inst->last_status==UART_CMD_RES_SNIC_SUCCESS;
}
NyLPC_TBool NyLPC_cSnicApi_sendFromSocket(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8 i_socket,NyLPC_TUInt8 i_option,const void* i_payload,NyLPC_TUInt16 i_payload_length,struct NyLPC_TSnicApi_SendFromSocketResult* i_result)
{
	NyLPC_TUInt8 data[2+1+2];
	i_inst->last_cmd=UART_CMD_SID_SNIC_SEND_FROM_SOCKET_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	data[2]=i_option;
	data[3]=(NyLPC_TUInt8)((i_payload_length>>8)&0xff);
	data[4]=(NyLPC_TUInt8)((i_payload_length>>0)&0xff);
	NyLPC_cSnicNetIf_startPayload(SNIC_UART_CMD_ID_SNIC,5+NyLPC_cSnicNetIf_getPayloadLength(i_payload,i_payload_length));
	NyLPC_cSnicNetIf_sendPayload(data,5);
	NyLPC_cSnicNetIf_sendPayload(i_payload,i_payload_length);
	NyLPC_cSnicNetIf_endPayload();
	//シグナル待ち
	do{
		NyLPC_cThread_yield();
	}while(!i_inst->_signal);
	NyLPC_cSnicNetIf_unregieterObject(i_inst);
	if(i_result!=NULL){
		i_result->number_of_byte_sent=i_inst->_response.rawbuf.u16[0];
	}
	return i_inst->last_status==UART_CMD_RES_SNIC_SUCCESS;
}
NyLPC_TBool NyLPC_cSnicApi_closeSocket(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8 i_socket)
{
	NyLPC_TUInt8 data[2+1];
	i_inst->last_cmd=UART_CMD_SID_SNIC_CLOSE_SOCKET_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	data[2]=i_socket;
	sendCmd(i_inst,data,3);
	return i_inst->last_status==UART_CMD_RES_SNIC_SUCCESS;
}


NyLPC_TBool NyLPC_cSnicApi_sendUdpPacketFromSocket(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8 i_socket,NyLPC_TUInt8 i_connection_mode,const struct NyLPC_TIPv4Addr* i_remote_ip,NyLPC_TUInt16 i_remote_port,const void* i_payload,NyLPC_TUInt16 i_payload_length,struct NyLPC_TSnicApi_SendUdpPacketFromSocketResult* i_result)
{
	NyLPC_TUInt8 data[2+4+6];
	i_inst->last_cmd=UART_CMD_SID_SNIC_UDP_SEND_FROM_SOCKET_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	memcpy(&data[2],&(i_remote_ip->v),4);
	data[6]=(NyLPC_TUInt8)((i_remote_port>>8)&0xff);
	data[7]=(NyLPC_TUInt8)((i_remote_port>>0)&0xff);
	data[8]=i_socket;
	data[9]=i_connection_mode;
	data[10]=(NyLPC_TUInt8)((i_payload_length>>8)&0xff);
	data[11]=(NyLPC_TUInt8)((i_payload_length>>0)&0xff);
	NyLPC_cSnicNetIf_startPayload(SNIC_UART_CMD_ID_SNIC,12+NyLPC_cSnicNetIf_getPayloadLength(i_payload,i_payload_length));
	NyLPC_cSnicNetIf_sendPayload(data,12);
	NyLPC_cSnicNetIf_sendPayload(i_payload,i_payload_length);
	NyLPC_cSnicNetIf_endPayload();
	//シグナル待ち
	do{
		NyLPC_cThread_yield();
	}while(!i_inst->_signal);
	if(i_result!=NULL){
		i_result->number_of_byte_sent=i_inst->_response.rawbuf.u16[0];
	}
	NyLPC_cSnicNetIf_unregieterObject(i_inst);
	return i_inst->last_status==UART_CMD_RES_SNIC_SUCCESS;
}
NyLPC_TBool NyLPC_cSnicApi_startUdpReceiveOnSocket(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8 i_socket,NyLPC_TUInt32 i_recv_buffer_size,struct NyLPC_TSnicApi_StartUdpReceiveOnSocketResult* i_result)
{
	NyLPC_TUInt8 data[2+3];
	i_inst->last_cmd=UART_CMD_SID_SNIC_UDP_START_RECV_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	data[2]=i_socket;
	data[3]=(NyLPC_TUInt8)((i_recv_buffer_size>>8)&0xff);
	data[4]=(NyLPC_TUInt8)((i_recv_buffer_size>>0)&0xff);
	sendCmd(i_inst,data,5);
	if(i_result!=NULL){
		i_result->recv_buf_size=i_inst->_response.rawbuf.u16[0];
	}
	return i_inst->last_status==UART_CMD_RES_SNIC_SUCCESS;
}
NyLPC_TBool NyLPC_cSnicApi_setSocketOption(NyLPC_TcSnicApi_t* i_inst,NyLPC_TUInt8 i_socket,NyLPC_TUInt16 i_level,NyLPC_TUInt16 i_option_name,NyLPC_TUInt16 i_option_value_length,const void* i_option_value)
{
	NyLPC_TUInt8 data[2+7];
	i_inst->last_cmd=UART_CMD_SID_SNIC_SETSOCKOPT_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	data[2]=i_socket;
	data[3]=(NyLPC_TUInt8)((i_level>>8)&0xff);
	data[4]=(NyLPC_TUInt8)((i_level>>0)&0xff);
	data[5]=(NyLPC_TUInt8)((i_option_name>>8)&0xff);
	data[6]=(NyLPC_TUInt8)((i_option_name>>0)&0xff);
	data[7]=(NyLPC_TUInt8)((i_option_value_length>>8)&0xff);
	data[8]=(NyLPC_TUInt8)((i_option_value_length>>0)&0xff);
	NyLPC_cSnicNetIf_startPayload(SNIC_UART_CMD_ID_SNIC,9+NyLPC_cSnicNetIf_getPayloadLength(i_option_value,i_option_value_length));
	NyLPC_cSnicNetIf_sendPayload(data,9);
	NyLPC_cSnicNetIf_sendPayload(i_option_value,i_option_value_length);
	NyLPC_cSnicNetIf_endPayload();
	//シグナル待ち
	do{
		NyLPC_cThread_yield();
	}while(!i_inst->_signal);
	NyLPC_cSnicNetIf_unregieterObject(i_inst);
	return i_inst->last_status==UART_CMD_RES_SNIC_SUCCESS;
}

NyLPC_TBool NyLPC_cSnicApi_getDhcpInfo(NyLPC_TcSnicApi_t* i_inst,struct NyLPC_TSnicApi_GetDhcpInfoResult* i_result)
{
	NyLPC_TUInt8 data[3];
	i_inst->last_cmd=UART_CMD_SID_SNIC_GET_DHCP_INFO_REQ;
	i_inst->last_seq=NyLPC_cSnicNetIf_regieterObject(i_inst);
	i_inst->_response.get_dhcp_info=i_result;
	data[0]=i_inst->last_cmd;
	data[1]=i_inst->last_seq;
	data[2]=0;
	sendCmd(i_inst,data,3);
	return i_inst->last_status==UART_CMD_RES_SNIC_SUCCESS;
}

static NyLPC_TBool indicationHandler(NyLPC_TcSnicApi_t* i_inst,struct NyLPC_TiSnicDevice_Interface* i_dev,NyLPC_TUInt8 i_cmd)
{
	NyLPC_TcSnicTcpSocket_t* tcp_sock;
	NyLPC_TcSnicUdpSocket_t* udp_sock;
	NyLPC_TcSnicTcpListener_t* tcp_listener;
	NyLPC_TUInt8 buf[10];
	buf[0]=i_cmd;
	switch(buf[0]){
	case UART_CMD_SID_SNIC_TCP_CLIENT_SOCKET_IND:
		//ソケットきたよ通知
		if(!NyLPC_cSnicNetIf_readPayload(&buf[1],2)){
			goto DROP;
		}
		tcp_listener=NyLPC_cSnicNetIf_getTcpListenerBySocket(buf[2]);
		if(tcp_listener!=NULL){
			if(NyLPC_cSnicTcpListener_onClientSocketInd(tcp_listener,i_dev)){
				break;
			}
		}else{
			//読み飛ばし
			NyLPC_cSnicNetIf_seekPayload(7);
			break;
		}
		goto DROP;
	case UART_CMD_SID_SNIC_TCP_CONNECTION_STATUS_IND:
		goto DROP;
	case UART_CMD_SID_SNIC_CONNECTION_RECV_IND:
		if(!NyLPC_cSnicNetIf_readPayload(&buf[1],2)){
			goto DROP;
		}
		tcp_sock=NyLPC_cSnicNetIf_getTcpSocketBySocket(buf[2]);
		if(tcp_sock!=NULL){
			if(NyLPC_cSnicTcpSocket_onConnectionRecvInd(tcp_sock,i_dev)){
				break;
			}
		}else{
			//華麗にスルーする。
			if(NyLPC_cSnicNetIf_readPayload(&buf[0],2)){
				//要らないパケットの吸出し
				NyLPC_cSnicNetIf_seekPayload((((NyLPC_TUInt16)buf[0])<<8)|buf[1]);
				break;
			}
		}
		goto DROP;
	case UART_CMD_SID_SNIC_UDP_RECV_IND:
		if(!NyLPC_cSnicNetIf_readPayload(&buf[1],2)){
			goto DROP;
		}
		udp_sock=NyLPC_cSnicNetIf_getUdpSocketBySocket(buf[2]);
		if(udp_sock!=NULL){
			if(NyLPC_cSnicUdpSocket_onConnectionRecvInd(udp_sock,i_dev)){
				break;
			}
		}else{
			//華麗にスルーする。
			if(NyLPC_cSnicNetIf_readPayload(&buf[0],2)){
				//要らないパケットの吸出し
				NyLPC_cSnicNetIf_seekPayload((((NyLPC_TUInt16)buf[0])<<8)|buf[1]);
				break;
			}
		}
		goto DROP;
	case UART_CMD_SID_SNIC_ARP_REPLY_IND:
		goto DROP;
	case UART_CMD_SID_SNIC_HTTP_RSP_IND:
		goto DROP;
	}
	return NyLPC_TBool_TRUE;
DROP:
	return NyLPC_TBool_FALSE;
}
static NyLPC_TBool requestHandler(struct NyLPC_TiSnicDevice_Interface* i_dev,NyLPC_TUInt8 i_cmd)
{
	NyLPC_TcSnicApi_t* inst;
	NyLPC_TUInt8 buf[24];//CMD,SEQ,RES
	buf[0]=i_cmd;
	if(!NyLPC_cSnicNetIf_readPayload(&buf[1],2)){
		goto DROP;
	}
	//SEQ->INST変換
	inst=NyLPC_cSnicNetIf_lockObjectById(buf[1]);
	if(inst==NULL){
		goto DROP;
	}
	//SEQがインスタンスの要求したものに一致するか?
	if(inst->last_seq!=buf[1]){
		goto DROP_UNLOCK;
	}
	//最後に実行したコマンドのレスポンスか?
	if(inst->last_cmd!=(buf[0]&UART_CMD_SID_RESPONSE_CMD_MASK)){
		goto DROP_UNLOCK;
	}
	//コマンドコードの保存
	inst->last_status=buf[2];
	//リクエストタイプごとの処理
	switch(buf[0]){
	case (UART_CMD_SID_SNIC_GET_DHCP_INFO_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
		if(inst->last_status==UART_CMD_RES_SNIC_SUCCESS){
			if(NyLPC_cSnicNetIf_readPayload(buf,6+4+4+4)){
				if(inst->_response.get_dhcp_info!=NULL){
					memcpy(&inst->_response.get_dhcp_info->macaddr,&buf[0],8);
					memcpy(&inst->_response.get_dhcp_info->ipaddr,&buf[6],4);
					memcpy(&inst->_response.get_dhcp_info->netmask,&buf[10],4);
					memcpy(&inst->_response.get_dhcp_info->gateway,&buf[14],4);
				}
				inst->_signal=NyLPC_TUInt8_TRUE;//シグナル設定
				NyLPC_cSnicNetIf_unlockObject(inst);
				return NyLPC_TBool_TRUE;
			}
		}
		goto DROP_UNLOCK;

	case (UART_CMD_SID_SNIC_UDP_START_RECV_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
	case (UART_CMD_SID_SNIC_UDP_SEND_FROM_SOCKET_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
	case (UART_CMD_SID_SNIC_SEND_FROM_SOCKET_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
	case UART_CMD_SID_SNIC_TCP_CONNECT_TO_SERVER_REQ|UART_CMD_SID_RESPONSE_BIT_MASK:
		if(inst->last_status==UART_CMD_RES_SNIC_SUCCESS){
			if(NyLPC_cSnicNetIf_readPayload(buf,2)){
				inst->_response.rawbuf.u16[0]=(((NyLPC_TUInt16)buf[0])<<8)|buf[1];
				inst->_signal=NyLPC_TUInt8_TRUE;//シグナル設定
				NyLPC_cSnicNetIf_unlockObject(inst);
				return NyLPC_TBool_TRUE;
			}
		}
		goto DROP_UNLOCK;
	case (UART_CMD_SID_SNIC_TCP_CREATE_CONNECTION_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
		if(inst->last_status==UART_CMD_RES_SNIC_SUCCESS){
			if(NyLPC_cSnicNetIf_readPayload(buf,3)){
				if(inst->_response.create_tcp_connection_server!=NULL){
					inst->_response.create_tcp_connection_server->buffer_size=(((NyLPC_TUInt16)buf[0])<<8)|buf[1];
					inst->_response.create_tcp_connection_server->maximum_client=buf[2];
				}
				inst->_signal=NyLPC_TUInt8_TRUE;//シグナル設定
				NyLPC_cSnicNetIf_unlockObject(inst);
				return NyLPC_TBool_TRUE;
			}
		}
		goto DROP_UNLOCK;
	case (UART_CMD_SID_SNIC_UDP_CREATE_SOCKET_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
	case (UART_CMD_SID_SNIC_TCP_CREATE_SOCKET_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
		if(inst->last_status==UART_CMD_RES_SNIC_SUCCESS){
			if(NyLPC_cSnicNetIf_readPayload(buf,1)){
				inst->_response.rawbuf.u8[0]=buf[0];
				inst->_signal=NyLPC_TUInt8_TRUE;//シグナル設定
				NyLPC_cSnicNetIf_unlockObject(inst);
				return NyLPC_TBool_TRUE;
			}
		}
		goto DROP_UNLOCK;
	case (UART_CMD_SID_SNIC_SETSOCKOPT_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
	case (UART_CMD_SID_SNIC_CLOSE_SOCKET_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
	case (UART_CMD_SID_SNIC_IP_CONFIG_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
	case (UART_CMD_SID_SNIC_CLEANUP_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
		if(inst->last_status==UART_CMD_RES_SNIC_SUCCESS){
			inst->_signal=NyLPC_TUInt8_TRUE;//シグナル設定
			NyLPC_cSnicNetIf_unlockObject(inst);
			return NyLPC_TBool_TRUE;
		}
		goto DROP_UNLOCK;
	case (UART_CMD_SID_SNIC_INIT_REQ|UART_CMD_SID_RESPONSE_BIT_MASK):
		if(NyLPC_cSnicNetIf_readPayload(buf,4)){
			if(inst->last_status==UART_CMD_RES_SNIC_SUCCESS){
				if(inst->_response.api_inisialization!=NULL){
					inst->_response.api_inisialization->default_rx_size=(((NyLPC_TUInt16)buf[0])<<8)|buf[1];
					inst->_response.api_inisialization->max_udp=(((NyLPC_TUInt16)buf[2])<<8)|buf[3];
				}
				inst->_signal=NyLPC_TUInt8_TRUE;//シグナル設定
				NyLPC_cSnicNetIf_unlockObject(inst);
				return NyLPC_TBool_TRUE;
			}
		}
		goto DROP_UNLOCK;
	default:
		goto DROP_UNLOCK;
	}
DROP_UNLOCK:
	inst->_signal=NyLPC_TUInt8_TRUE;//シグナル設定
	NyLPC_cSnicNetIf_unlockObject(inst);
DROP:
	return NyLPC_TBool_FALSE;

}
/**
 * FALSEを返却したら次のSOMまでまつよ
 */
NyLPC_TBool NyLPC_cSnicApi_rxHandler(NyLPC_TcSnicApi_t* i_inst,struct NyLPC_TiSnicDevice_Interface* i_dev)
{
	NyLPC_TUInt8 buf[1];
	if(!NyLPC_cSnicNetIf_readPayload(buf,1)){
		return NyLPC_TBool_FALSE;
	}
	if(buf[0]&0x20){
		return indicationHandler(i_inst,i_dev,buf[0]);
	}else{
		return requestHandler(i_dev,buf[0]);
	}
}

/**
 * メッセージハンドラで無視されるSocketClose
 */
void NyLPC_cSnicApi_sysCloseSocket(NyLPC_TUInt8 i_sock)
{
	NyLPC_TUInt8 buf[3];
	buf[0]=UART_CMD_SID_SNIC_CLOSE_SOCKET_REQ;
	buf[1]=0;//コマンド待機しないスペシャル値
	buf[2]=i_sock;
	NyLPC_cSnicNetIf_sendShortPayload(buf,SNIC_UART_CMD_ID_SNIC,3);
}
void NyLPC_cSnicApi_sendIndicateConfirm(NyLPC_TUInt8 i_cmd,NyLPC_TUInt8 i_seq)
{
	NyLPC_TUInt8 buf[3];
	buf[0]=UART_CMD_SID_SNIC_CLOSE_SOCKET_REQ;
	buf[1]=i_seq;//コマンド待機しないスペシャル値
	NyLPC_cSnicNetIf_sendShortPayload(buf,SNIC_UART_CMD_ID_SNIC,2);
}
