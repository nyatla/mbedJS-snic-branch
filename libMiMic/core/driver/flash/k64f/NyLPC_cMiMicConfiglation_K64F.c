#include "NyLPC_config.h"
#if NyLPC_MCU==NyLPC_MCU_K64F
#include "NyLPC_stdlib.h"
#include "NyLPC_flash.h"
#include "NyLPC_http.h"
#include "NyLPC_net.h"
#include "K64F_IAP.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * Onchip flashを使ったコンフィギュレーション保存システムです。
 * K64Fの0x0080000に構造体NyLPC_TMiMicConfigulationを保存します。
 */




#define IP2Int(a0,a1,a2,a3) ((0xff000000&(((NyLPC_TUInt32)(a0))<<24))|(0x00ff0000&(((NyLPC_TUInt32)(a1))<<16))|(0x0000ff00&(((NyLPC_TUInt32)(a2))<<8))|(0x000000ff&(((NyLPC_TUInt32)(a3)))))
/**
 * コンフィギュレーションの保存セクタ
 */
#define MIMIC_CONFIGLATION_FLASH_ADDR ((void*)0x00080000)

/**
 * コンフィギュレーション値はホストオーダーで保存する。
 */
const struct NyLPC_TMiMicConfigulation factory_default=
{
    0xffffffff,             //fastboot
    "MiMic020102030405",
    0x02010203,0x0405ffff,  //Mac addr
    //IPv4
    NyLPC_TcNetConfig_IPV4_FLAG_MODE_MANUAL,    //flags
    IP2Int(192,168,0,39),
    IP2Int(255,255,255,0),
    IP2Int(192,168,0,254),
    //ServerFlags
    NyLPC_TcNetConfig_SERVICE_FLAG_MDNS|NyLPC_TcNetConfig_SERVICE_FLAG_UPNP,
    //HTTP
    80, //HTTP-Port
    0   //padding
};


#define FAST_BOOT_DATA 0xfffffffe
/**
 * ユーザコンフィギュレーションを更新する。
 * この関数をコールするときは、割込/FreeRTOSを一時停止すること。
 */
NyLPC_TBool NyLPC_cMiMicConfiglation_updateConfigulation(const struct NyLPC_TMiMicConfigulation* i_congfiglation)
{
    struct NyLPC_TMiMicConfigulation d;
    d.fast_boot=FAST_BOOT_DATA;
    memcpy(d.hostname,i_congfiglation->hostname,NyLPC_TcNetConfig_HOSTNAME_LEN);
    d.mac_00_01_02_03=i_congfiglation->mac_00_01_02_03;
    d.mac_04_05_xx_xx=i_congfiglation->mac_04_05_xx_xx;
    d.ipv4_flags=i_congfiglation->ipv4_flags;
    d.ipv4_addr_net=i_congfiglation->ipv4_addr_net;
    d.ipv4_mask_net=i_congfiglation->ipv4_mask_net;
    d.ipv4_drut_net=i_congfiglation->ipv4_drut_net;
    d.srv_flags=i_congfiglation->srv_flags;
    d.http_port=i_congfiglation->http_port;
    d.padding32=0xffff;
    d.padding64=0xffffffff;  
    
    
    if(K64F_IAP_erase_sector((int)MIMIC_CONFIGLATION_FLASH_ADDR)!=K64F_IAP_TIAPCode_Success){
        NyLPC_OnErrorGoto(Error);
    }
    if(K64F_IAP_program_flash((int)MIMIC_CONFIGLATION_FLASH_ADDR,(char*)(&d), sizeof(struct NyLPC_TMiMicConfigulation))!=K64F_IAP_TIAPCode_Success){
        NyLPC_OnErrorGoto(Error);
    }
    return NyLPC_TBool_TRUE;
Error:
    return NyLPC_TBool_FALSE;
}
/**
 * コンフィギュレーション値を返す。
 */
const struct NyLPC_TMiMicConfigulation* NyLPC_cMiMicConfiglation_loadFromFlash(void)
{
    if(NyLPC_cMiMicConfiglation_hasUserConfigulation()){
        //userコンフィギュレーション読むよ
        return (const struct NyLPC_TMiMicConfigulation*)(MIMIC_CONFIGLATION_FLASH_ADDR);
    }else{
        //Userコンフィギュレーションない
        return &factory_default;
    }
}
const struct NyLPC_TMiMicConfigulation* NyLPC_cMiMicConfiglation_loadFactoryDefault(void)
{
    return &factory_default;
}





/**
 * ユーザコンフィギュレーションが存在すると、true.
 */
NyLPC_TBool NyLPC_cMiMicConfiglation_hasUserConfigulation(void)
{
    //初回読出しはFlashにFFFFFFFFが格納されているのを期待する。
    volatile const NyLPC_TUInt32* fast_boot=((NyLPC_TUInt32*)MIMIC_CONFIGLATION_FLASH_ADDR);
    return (*fast_boot)!=0xffffffff;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
