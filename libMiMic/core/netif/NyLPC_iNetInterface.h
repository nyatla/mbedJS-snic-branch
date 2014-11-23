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
#ifndef NYLPC_INETINTERFACE_H_
#define NYLPC_INETINTERFACE_H_


#include "NyLPC_stdlib.h"
#include "NyLPC_os.h"
#include "NyLPC_iTcpSocket.h"
#include "NyLPC_iUdpSocket.h"
#include "NyLPC_iTcpListener.h"
#include "NyLPC_cIPv4Config.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct NyLPC_TNetInterfaceInfo{
	const char* device_name;
	const NyLPC_TcIPv4Config_t* current_config;
};

/**
 * ソケット生成のヒント値
 */
typedef NyLPC_TUInt16 NyLPC_TSocketType;
#define NyLPC_TSocketType_TCP_NORMAL	0x0100
#define NyLPC_TSocketType_TCP_HTTP		0x0101
#define NyLPC_TSocketType_UDP_NORMAL	0x0200
#define NyLPC_TSocketType_UDP_NOBUF		0x0202
/**********************************************************************
 * Function
 **********************************************************************/

/**
 * 制限時間付きでソケットに接続を待ちます。
 */
typedef NyLPC_TiTcpSocket_t* (*NyLPC_TiNetInterface_createTcpSocetEx)(NyLPC_TSocketType i_socktype);
typedef NyLPC_TiUdpSocket_t* (*NyLPC_TiNetInterface_createUdpSocetEx)(NyLPC_TUInt16 i_port,NyLPC_TSocketType i_socktype);
typedef NyLPC_TiTcpListener_t* (*NyLPC_TiNetInterface_createTcpListener)(NyLPC_TUInt16 i_port);
/**
 * Start関数はイベント関数の定期コールが開始される前に呼び出されます。
 */
typedef void (*NyLPC_TiNetInterface_start)(const NyLPC_TcIPv4Config_t* i_cfg);
typedef void (*NyLPC_TiNetInterface_stop)(void);

typedef void(*NyLPC_TiNetInterface_sendArpRequest)(const struct NyLPC_TIPv4Addr* i_addr);
typedef NyLPC_TBool(*NyLPC_TiNetInterface_hasArpInfo)(const struct NyLPC_TIPv4Addr* i_addr);
typedef NyLPC_TBool(*NyLPC_TiNetInterface_isInitService)(void);

typedef const struct NyLPC_TNetInterfaceInfo* (*NyLPC_TiNetInterface_getInterfaceInfo)(void);

/**********************************************************************
 * Interface
 **********************************************************************/
struct NyLPC_TiNetInterface_Interface
{
	NyLPC_TiNetInterface_createTcpSocetEx createTcpSocketEx;
	NyLPC_TiNetInterface_createUdpSocetEx createUdpSocetEx;
	NyLPC_TiNetInterface_createTcpListener createTcpListener;
	NyLPC_TiNetInterface_start start;
	NyLPC_TiNetInterface_stop stop;
	NyLPC_TiNetInterface_sendArpRequest sendarprequest;
	NyLPC_TiNetInterface_hasArpInfo hasarpinfo;
	NyLPC_TiNetInterface_isInitService isinitservice;
	NyLPC_TiNetInterface_getInterfaceInfo getinterfaceinfo;
};



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CTCPLISTENER_H_ */
