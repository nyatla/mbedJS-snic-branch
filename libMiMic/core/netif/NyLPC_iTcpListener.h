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
#ifndef NYLPC_ITCPLISTENER_H_
#define NYLPC_ITCPLISTENER_H_


#include "NyLPC_stdlib.h"
#include "NyLPC_os.h"
#include "NyLPC_NetIf_ip_types.h"
#include "NyLPC_iTcpSocket.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NyLPC_TiTcpListener NyLPC_TiTcpListener_t;

/**********************************************************************
 * Function
 **********************************************************************/


/**
 * 制限時間付きでソケットに接続を待ちます。
 */
typedef NyLPC_TBool (*NyLPC_TiTcpListener_listen)(NyLPC_TiTcpListener_t* i_inst,NyLPC_TiTcpSocket_t* i_sock,NyLPC_TUInt32 i_wait_msec);
typedef void (*NyLPC_TiTcpListener_finaize)(NyLPC_TiTcpListener_t* i_inst);
/**********************************************************************
 * Interface
 **********************************************************************/

/**
 */
struct NyLPC_TiTcpListener_Interface
{
	NyLPC_TiTcpListener_listen listen;
	NyLPC_TiTcpListener_finaize finalize;
};

struct NyLPC_TiTcpListener
{
	const struct NyLPC_TiTcpListener_Interface* _interface;
};

#define NyLPC_iTcpListener_listen(i_inst,i_sock,i_wait_msec)	((i_inst)->_interface->listen((i_inst),(i_sock),(i_wait_msec)))
#define NyLPC_iTcpListener_finaize(i_inst)						((i_inst)->_interface->finalize((i_inst)))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_ITCPLISTENER_H_ */
