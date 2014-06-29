#pragma once
////////////////////////////////////////////////////////////////////////////////
// mimic.h
////////////////////////////////////////////////////////////////////////////////
#include "Net.h"
#include "NetConfig.h"
#include "Httpd.h"
#include "Http.h"
#include "UrlReader.h"
#include "HttpdConnection.h"
#include "IpAddr.h"
#include "TcpSocket.h"
#include "UdpSocket.h"
#include "HttpClient.h"
#include "jsonrpc/MbedJsApi.h"

#include "mod/ModUrl.h"
#include "mod/ModRomFiles.h"
#include "mod/ModRemoteMcu.h"
#include "mod/ModMiMicSetting.h"
#include "mod/ModLocalFileSystem.h"
#include "mod/ModFileIo.h"
#include "mod/ModUPnPDevice.h"
#include "mod/ModWebSocket.h"
#include "mod/ModJsonRpc.h"
#include "LocalFileSystem2.h"

using namespace MiMic;