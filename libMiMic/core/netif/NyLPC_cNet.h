#ifndef NYLPC_CNET_H_
#define NYLPC_CNET_H_
#include "NyLPC_stdlib.h"
#include "NyLPC_cIPv4Config.h"
#include "NyLPC_iTcpListener.h"
#include "NyLPC_iTcpSocket.h"
#include "NyLPC_iUdpSocket.h"
#include "NyLPC_iNetInterface.h"
#include "NyLPC_NetIf_ip_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NyLPC_TcNet NyLPC_TcNet_t;

void NyLPC_cNet_initialize(const struct NyLPC_TiNetInterface_Interface* i_netif);
#define NyLPC_cNet_finalize()

NyLPC_TiTcpSocket_t* NyLPC_cNet_createTcpSocketEx(NyLPC_TSocketType i_socktype);
NyLPC_TiUdpSocket_t* NyLPC_cNet_createUdpSocketEx(NyLPC_TUInt16 i_port,NyLPC_TSocketType i_socktype);
NyLPC_TiTcpListener_t* NyLPC_cNet_createTcpListenerEx(NyLPC_TUInt16 i_port);

/**
 * 指定したIPアドレスを要求するARPリクエストを発行します。
 */
void NyLPC_cNet_sendArpRequest(const struct NyLPC_TIPv4Addr* i_addr);

/**
 * ARPテーブルに指定したIPがあるかを返します。
 */
NyLPC_TBool NyLPC_cNet_hasArpInfo(const struct NyLPC_TIPv4Addr* i_addr);

void NyLPC_cNet_start(const NyLPC_TcIPv4Config_t* i_ref_config);
void NyLPC_cNet_stop(void);
NyLPC_TBool NyLPC_cNet_isInitService(void);

/**
 * NyLPC_TcIPv4Config_tをDHCPで更新します。
 * この関数をコールする時は、サービスは停止中でなければなりません。
 * @param i_cfg
 * 更新するi_cfg構造体。
 * emac,default_mssは設定済である必要があります。他のフィールド値は不定で構いません。
 * 更新されるフィールドは、ip,netmast,default_rootの3つです。
 * @return
 * 更新に成功した場合TRUE
 */
NyLPC_TBool NyLPC_cNet_requestAddrDhcp(NyLPC_TcIPv4Config_t* i_cfg,NyLPC_TInt16 i_repeat);
/**
 * NyLPC_TcIPv4Config_tをAPIPAで更新します。
 * この関数をコールする時は、サービスは停止中でなければなりません。
 * @param i_cfg
 * 更新するi_cfg構造体。
 * emac,default_mssは設定済である必要があります。他のフィールド値は不定で構いません。
 * 更新されるフィールドは、ip,netmast,default_rootの3つです。
 * @return
 * 更新に成功した場合TRUE
 */
NyLPC_TBool NyLPC_cNet_requestAddrApipa(NyLPC_TcIPv4Config_t* i_cfg,NyLPC_TInt16 i_repeat);

const struct NyLPC_TNetInterfaceInfo* NyLPC_cNet_getInterfaceInfo(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

