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
