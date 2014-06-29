#include "NyLPC_config.h"
#if NyLPC_MCU==NyLPC_MCU_LPC17xx

#include "../EthDev.h"
#include "EtherDev_DP83848C_protected.h"
#include "EtherDev_LAN8720_protected.h"



const struct TiEthernetDevice* getEthernetDevicePnP(void)
{
	const struct TiEthernetDevice* ret;
	if(EthDev_LAN8720_getInterface(&ret)){
		return ret;
	}
	if(EthDev_DP83848C_getInterface(&ret)){
		return ret;
	}
	return NULL;
}


#endif


