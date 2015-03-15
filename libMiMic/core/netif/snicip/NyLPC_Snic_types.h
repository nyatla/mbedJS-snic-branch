/*
 * NyLPC_Snic_types.h
 *
 *  Created on: 2014/11/22
 *      Author: nyatla
 */

#ifndef NYLPC_SNIC_TYPES_H_
#define NYLPC_SNIC_TYPES_H_

#include "NyLPC_stdlib.h"

typedef const char* NyLPC_Snic_TCuntryCode;
#define NyLPC_Snic_TCuntryCode_JAPAN ("JP")
#define NyLPC_Snic_TCuntryCode_UNITED_STATES ("US")

#define SNIC_UART_CMD_ID_GEN     0x01    //General command
#define SNIC_UART_CMD_ID_SNIC    0x70    //SNIC command
#define SNIC_UART_CMD_ID_WIFI    0x50    //Wi-Fi command

#define SNIC_WIFI_SECURITY_OPEN 			0x00
#define SNIC_WIFI_SECURITY_WEP 				0x01
#define SNIC_WIFI_SECURITY_WPA_TKIP_PSK 	0x02
#define SNIC_WIFI_SECURITY_WPA2_AES_PSK  	0x04
#define SNIC_WIFI_SECURITY_WPA2_MIXED_PSK 	0x06

#define SNIC_API_SOL_SOCKET (0x0fff)
#define SNIC_API_IPPROTO_IP (0x0000)
#define SNIC_API_IPPROTO_TCP (0x0006)

#define SNIC_API_SO_BROADCAST (0x0020) //BOOL Specify whether the socket can send broadcast message
#define SNIC_API_SO_DONTLINGER (0xFF7F) //BOOL Specify whether the socket should not block the socket close operation while waiting for data to be send.
#define SNIC_API_SO_LINGER (0x0080) //UINT16 Set socket close linger time in seconds.
#define SNIC_API_SO_RCVBUF (0x1002) //UINT16 The total per-socket buffer space reserved for receives.
#define SNIC_API_SO_REUSEADDR (0x0004) //BOOL The socket can be bound to an address which is already in use.
#define SNIC_API_SO_SNDBUF (0x1001) //UINT16 The total per-socket buffer space reserved for sends.
#define SNIC_API_SO_TYPE (0x1008) //UINT32

#define SNIC_API_IP_ADD_MEMBERSHIP (0x0003) //IP_MREQ Request to stop receiving from a multicast group
#define SNIC_API_IP_DROP_MEMBERSHIP (0x0004)//IP_MREQ
#define SNIC_API_IP_MULTICAST_TTL (0x0005) // UINT8 Specify the time-to-live for a multicast packet
#define SNIC_API_IP_MULTICAST_IF (0x0006) //UINT32 Specify the address of the multicast interface
#define SNIC_API_IP_TOS (0x0001) //UINT32 Specify the type of service parameter
#define SNIC_API_IP_TTL (0x0002) //UINT32 Specify the time-to-live for a non-multicast packet

#define SNIC_API_TCP_NODELAY (0x0001)	//BOOL


struct NyLPC_TUartFrameFormatHeader
{
	NyLPC_TUInt8 l0;
	NyLPC_TUInt8 l1;
	NyLPC_TUInt8 command_id;
}PACK_STRUCT_END;
#define NyLPC_TUartFrameFormatHeader_SIZE_OF_DATA 3
#define NyLPC_TUartFrameFormat_getLength(i_struct) 	((((NyLPC_TUInt16)(i_struct)->l1 & 0x03)<<7)|((i_struct)->l0 & 0x7f))
#define NyLPC_TUartFrameFormat_isAck(i_struct) (((i_struct)->l1&0x40)==0)
#define NyLPC_TUartFrameFormat_getCmd(i_struct) ((i_struct)->command_id&0x7f)








#endif /* NYLPC_SNIC_TYPES_H_ */
