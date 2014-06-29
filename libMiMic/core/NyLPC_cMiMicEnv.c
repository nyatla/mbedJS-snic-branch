#include "NyLPC_cMiMicEnv.h"
#include "../uip/NyLPC_cUipService_protected.h"

const static char* VERSION="MiMic/1.5.10";

#if NyLPC_MCU==NyLPC_MCU_LPC4088
const static char* MCU="LPC4088";
#elif NyLPC_MCU==NyLPC_MCU_LPC17xx
const static char* MCU="LPC176x";
#endif

const static char* UNKNOWN="UNKNOWN";


static const char* PNAME_LPCXPRESSO="LPCXpresso";
static const char* PNAME_MBED="mbed";



const char* NyLPC_cMiMicEnv_getStrProperty(NyLPC_TUInt16 i_id)
{
    switch(i_id){
    case NyLPC_cMiMicEnv_VERSION:
        return VERSION;
    case NyLPC_cMiMicEnv_SHORT_NAME:
        switch(*(NyLPC_cUipService_refDeviceName())){
        case 'L':
            return PNAME_LPCXPRESSO;
        case 'D':
            return PNAME_MBED;
        default:
            return UNKNOWN;
        }
    case NyLPC_cMiMicEnv_ETHERNET_PHY:
        return NyLPC_cUipService_refDeviceName();
    case NyLPC_cMiMicEnv_MCU_NAME:
        return MCU;
    default:
        return UNKNOWN;
    }
}
