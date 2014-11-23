#ifndef NYLPC_CNETIF_H_
#define NYLPC_CNETIF_H_
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

typedef struct NyLPC_TcNetIf NyLPC_TcNetIf_t;

void NyLPC_cNetIf_initialize(void);
#define NyLPC_cNetIf_finalize()

NyLPC_TiTcpSocket_t* NyLPC_cNetIf_createTcpSocketEx(NyLPC_TSocketType i_socktype);
NyLPC_TiUdpSocket_t* NyLPC_cNetIf_createUdpSocketEx(NyLPC_TUInt16 i_port,NyLPC_TSocketType i_socktype);
NyLPC_TiTcpListener_t* NyLPC_cNetIf_createTcpListenerEx(NyLPC_TUInt16 i_port);

/**
 * 指定したIPアドレスを要求するARPリクエストを発行します。
 */
void NyLPC_cNetIf_sendArpRequest(const struct NyLPC_TIPv4Addr* i_addr);

/**
 * ARPテーブルに指定したIPがあるかを返します。
 */
NyLPC_TBool NyLPC_cNetIf_hasArpInfo(const struct NyLPC_TIPv4Addr* i_addr);

void NyLPC_cNetIf_start(const NyLPC_TcIPv4Config_t* i_ref_config);
void NyLPC_cNetIf_stop(void);
NyLPC_TBool NyLPC_cNetIf_isInitService(void);

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
NyLPC_TBool NyLPC_cNetIf_requestAddrDhcp(NyLPC_TcIPv4Config_t* i_cfg,NyLPC_TInt16 i_repeat);
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
NyLPC_TBool NyLPC_cNetIf_requestAddrApipa(NyLPC_TcIPv4Config_t* i_cfg,NyLPC_TInt16 i_repeat);

const struct NyLPC_TNetInterfaceInfo* NyLPC_cNetIf_getInterfaceInfo(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

