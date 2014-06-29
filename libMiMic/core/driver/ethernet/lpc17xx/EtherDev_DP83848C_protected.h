/*
 * This is part of EthDev_LPC17xx.h
 */

#ifndef EtherDev_DP83848C_protected_h
#define EtherDev_DP83848C_protected_h


#include "NyLPC_stdlib.h"
#include "../NyLPC_IEthernetDevice.h"
#include "EthDev_LPC17xx.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

NyLPC_TBool EthDev_DP83848C_getInterface(
	const struct TiEthernetDevice** o_dev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

