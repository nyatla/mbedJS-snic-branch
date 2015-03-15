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
 *
 * Parts of this file were leveraged from uIP:
 *
 * Copyright (c) 2001-2003, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef NyLPC_TcMiMicIpNetIf_H_
#define NyLPC_TcMiMicIpNetIf_H_
#include "NyLPC_config.h"
#include "NyLPC_stdlib.h"
#include "../NyLPC_iNetInterface.h"
#include "../NyLPC_cIPv4Config.h"
/**********************************************************************
 * Heder files
 **********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************
 *
 * NyLPC_TcUipService class
 *
 **********************************************************************/
/**
 * uipタスクを管理する、サービスクラスです。
 */
typedef struct NyLPC_TcMiMicIpNetIf NyLPC_TcMiMicIpNetIf_t;


/** サービスタスクスタックサイズ
 *  mdns 256+96(MIN)
 *  upnp 256+160(MIN)
 *  mbedのExportしたのをLPCXpressoでコンパイルするときは+192しないと落ちる。
 */
#ifndef NyLPC_cMiMicIpNetIf_config_STACK_SIZE
#	define NyLPC_cMiMicIpNetIf_config_STACK_SIZE (256+256+192)
#endif

/**
 * SOCKETリソースの最大数とRXメモリサイズ
 */

#ifndef NyLPC_cMiMicIpNetIf_config_TCPLISTENER_MAX
#	define NyLPC_cMiMicIpNetIf_config_TCPLISTENER_MAX			2
#endif
#ifndef NyLPC_cMiMicIpNetIf_config_TCPSOCKET_MAX
#	define NyLPC_cMiMicIpNetIf_config_TCPSOCKET_MAX				3
#endif
#ifndef NyLPC_cMiMicIpNetIf_config_UDPSOCKET_MAX
#	define NyLPC_cMiMicIpNetIf_config_UDPSOCKET_MAX				1
#endif
#ifndef NyLPC_cMiMicIpNetIf_config_NB_UDPSOCKET_MAX
#	define NyLPC_cMiMicIpNetIf_config_NB_UDPSOCKET_MAX 			2
#endif
#ifndef NyLPC_cMiMicIpNetIf_config_TCPSOCKET_RX_BUFFER_SIZE
#	define NyLPC_cMiMicIpNetIf_config_TCPSOCKET_RX_BUFFER_SIZE	512
#endif
#ifndef NyLPC_cMiMicIpNetIf_config_UDPSOCKET_RX_BUFFER_SIZE
#	define NyLPC_cMiMicIpNetIf_config_UDPSOCKET_RX_BUFFER_SIZE	512
#endif






//const NyLPC_TcIPv4Config_t* NyLPC_cUipService_refCurrentConfig(void);
const struct NyLPC_TiNetInterface_Interface* NyLPC_cMiMicIpNetIf_getNetInterface(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

