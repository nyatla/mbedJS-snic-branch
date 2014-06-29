#include "NyLPC_config.h"
#if NyLPC_MCU==NyLPC_MCU_LPC4088

#include "../EthDev.h"
#include "EtherDev_LPC4088_protected.h"



const struct TiEthernetDevice* getEthernetDevicePnP(void)
{
	const struct TiEthernetDevice* ret;
	if(EthDev_LPC4088_getInterface(&ret)){
		return ret;
	}
	return NULL;
}


#endif


