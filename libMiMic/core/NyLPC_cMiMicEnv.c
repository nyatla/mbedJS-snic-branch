#include "NyLPC_cMiMicEnv.h"
#include "NyLPC_netif.h"

const static char* VERSION="MiMic/1.7.0";

#if NyLPC_MCU==NyLPC_MCU_LPC4088
const static char* MCU="LPC4088";
static const char* PNAME_LPCXPRESSO="LPCXpresso";
static const char* PNAME_MBED="mbed";
#elif NyLPC_MCU==NyLPC_MCU_LPC17xx
const static char* MCU="LPC176x";
static const char* PNAME_LPCXPRESSO="LPCXpresso";
static const char* PNAME_MBED="mbed";
#elif NyLPC_MCU==NyLPC_MCU_K64F
const static char* MCU="K64F";
static const char* PNAME_FRDM="FRDM";
#endif

const static char* UNKNOWN="UNKNOWN";





const char* NyLPC_cMiMicEnv_getStrProperty(NyLPC_TUInt16 i_id)
{
    switch(i_id){
    case NyLPC_cMiMicEnv_VERSION:
        return VERSION;
    case NyLPC_cMiMicEnv_SHORT_NAME:
#if NyLPC_MCU==NyLPC_MCU_K64F
        return PNAME_FRDM;
#else
        switch(*(NyLPC_cNetIf_getInterfaceInfo()->device_name)){
        case 'L':
            return PNAME_LPCXPRESSO;
        case 'D':
            return PNAME_MBED;
        default:
            return UNKNOWN;
        }
#endif
    case NyLPC_cMiMicEnv_ETHERNET_PHY:
        return NyLPC_cNetIf_getInterfaceInfo()->device_name;
    case NyLPC_cMiMicEnv_MCU_NAME:
        return MCU;
    default:
        return UNKNOWN;
    }
}
