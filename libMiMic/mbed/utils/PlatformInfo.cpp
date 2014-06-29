#include "PlatformInfo.h"
#define REG_HCSR 0xe000edf0
#define REG_DEMCR 0xE000EDFC

#include "mbed.h"
#include "NyLPC_os.h"
namespace MiMic
{
    int PlatformInfo::_pftype=PF_UNKNOWN;
    void PlatformInfo::check()
    {
    #if PlatformInfo_DETECTION_MODE==PlatformInfo_DETECTION_MODE_MBED
        _pftype=PF_MBED;
        return;
    #elif PlatformInfo_DETECTION_MODE==PlatformInfo_DETECTION_MODE_LPCXPRESSO
        _pftype=PF_LPCXPRESSO;
        return;
    #elif PlatformInfo_DETECTION_MODE==PlatformInfo_DETECTION_MODE_AUTO
        //LPCXpresso is return S_RESET_ST==1 when standalone.
        wait_ms(200);
        unsigned int v;
        v=*(unsigned int*)REG_HCSR;
        //check Debug Halting Control and Status::S_RESET_ST sticky bit
        if((v & 0x02000000)!=0){
            //may be LPC-Standalone
            _pftype=PF_LPCXPRESSO;        
            return;
        }
        v=(*(unsigned int*)REG_DEMCR);
        if((v & 0x01000000)==0x0){
            //may be mbed
            _pftype=PF_MBED;
            return;
        }
        _pftype=PF_LPCXPRESSO;
        return;
    #else
        #error "ERROR!"
    #endif
    }
    int PlatformInfo::getPlatformType()
    {
        if(_pftype==PF_UNKNOWN){
            check();
        }
        return _pftype;
    }
}