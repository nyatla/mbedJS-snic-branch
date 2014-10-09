#include "NyLPC_config.h"
#if NyLPC_MCU==NyLPC_MCU_K64F

#include "../EthDev.h"
#include "EtherDev_K64F_protected.h"



const struct TiEthernetDevice* getEthernetDevicePnP(void)
{
	const struct TiEthernetDevice* ret;
	if(EthDev_K64F_getInterface(&ret)){
		return ret;
	}
	return NULL;
}


#endif


