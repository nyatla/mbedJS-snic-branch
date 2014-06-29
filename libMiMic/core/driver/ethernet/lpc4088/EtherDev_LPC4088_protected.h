/*
 * This is part of EthDev_LPC17xx.h
 */

#ifndef EtherDev_LPC4088_protected_h
#define EtherDev_LPC4088_protected_h
#include "NyLPC_stdlib.h"
#include "../NyLPC_IEthernetDevice.h"
#include "EthDev_LPC17xx.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

NyLPC_TBool EthDev_LPC4088_getInterface(
	const struct TiEthernetDevice** o_dev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

