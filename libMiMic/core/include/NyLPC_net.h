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
/**
 * @file
 * このファイルは、mimicVmディレクトリ以下に宣言されるヘッダファイルを集積します。
 */
#ifndef NYLPC_NET_H_
#define NYLPC_NET_H_

#include "../net/mdns/NyLPC_cMDnsServer.h"

#include "../net/httpd/NyLPC_cHttpdConnection.h"
#include "../net/httpd/NyLPC_cHttpd.h"
#include "../net/httpd/NyLPC_cHttpdUtils.h"

#include "../net/httpd/mod/NyLPC_cModFileIoBaseClass.h"
#include "../net/httpd/mod/NyLPC_cModRemoteMcu.h"
#include "../net/httpd/mod/NyLPC_cModMiMicSetting.h"
#include "../net/httpd/mod/NyLPC_cModWebSocket.h"
#include "../net/httpd/mod/NyLPC_cModRomFiles.h"
#include "../net/httpd/mod/NyLPC_cModUrl.h"
#include "../net/httpd/mod/NyLPC_cModUPnPDevice.h"
#include "../net/httpd/mod/NyLPC_cModJsonRpc.h"

#include "../net/httpcl/NyLPC_cHttpClient.h"


#include "../net/upnp/NyLPC_cSsdpSocket.h"
#include "../net/upnp/NyLPC_cUPnP.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
