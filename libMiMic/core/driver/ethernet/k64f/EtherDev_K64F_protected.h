#ifndef EtherDev_K64F_protected_h
#define EtherDev_K64F_protected_h
#include "NyLPC_stdlib.h"
#include "../NyLPC_IEthernetDevice.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

NyLPC_TBool EthDev_K64F_getInterface(
	const struct TiEthernetDevice** o_dev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

