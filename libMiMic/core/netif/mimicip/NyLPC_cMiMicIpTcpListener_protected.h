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
#ifndef NYLPC_CTCPLISTENER_PROTECTED_H_
#define NYLPC_CTCPLISTENER_PROTECTED_H_
#include "NyLPC_cMiMicIpTcpListener.h"
#include "NyLPC_cMiMicIpNetIf_protected.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**
 * ListenerへSYNパケットを通知します。
 * @return
 * 常にTRUE
 */
NyLPC_TBool NyLPC_cMiMicIpTcpListener_synPacket(NyLPC_TcMiMicIpTcpListener_t* i_inst,const NyLPC_TcIPv4Payload_t* i_payload);
#define NyLPC_cMiMicIpTcpListener_getLocalPort(i) ((i)->_port)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CTCPLISTENER_PROTECTED_H_ */
