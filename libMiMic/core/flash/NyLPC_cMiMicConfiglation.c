#include "NyLPC_flash.h"
#include "NyLPC_http.h"
#include "NyLPC_cMiMicConfiglation.h"
#include "NyLPC_net.h"
/**
 * Onchip flashを使ったコンフィギュレーション保存システムです。
 */




#define IP2Int(a0,a1,a2,a3) ((0xff000000&(((NyLPC_TUInt32)(a0))<<24))|(0x00ff0000&(((NyLPC_TUInt32)(a1))<<16))|(0x0000ff00&(((NyLPC_TUInt32)(a2))<<8))|(0x000000ff&(((NyLPC_TUInt32)(a3)))))
/**
 * コンフィギュレーションの保存セクタ
 */
#define MIMIC_CONFIGLATION_FLASH_SECTOR 29
#define MIMIC_CONFIGLATION_FLASH_SECTOR_ADDR 0x00078000

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



const static NyLPC_TUInt32 FAST_BOOT_DATA=0xfffffffe;
/**
 * ユーザコンフィギュレーションを更新する。
 * この関数をコールするときは、割込/FreeRTOSを一時停止すること。
 */
NyLPC_TBool NyLPC_cMiMicConfiglation_updateConfigulation(const struct NyLPC_TMiMicConfigulation* i_congfiglation)
{
    const NyLPC_TUInt32* volatile fast_boot=&(factory_default.fast_boot);
    //イレース
    if(!NyLPC_cOnchipFlashWriter_elase(MIMIC_CONFIGLATION_FLASH_SECTOR,MIMIC_CONFIGLATION_FLASH_SECTOR)){
        NyLPC_OnErrorGoto(Error);
    }
    //コンフィギュレーションを書き込む。
    if(!NyLPC_cOnchipFlashWriter_writeSector(MIMIC_CONFIGLATION_FLASH_SECTOR,0x00000000,i_congfiglation,sizeof(struct NyLPC_TMiMicConfigulation))){
        NyLPC_OnErrorGoto(Error);
    }
    //プログラム済フラッシュの一部を書き換えてユーザコンフィギュレーションをONにする。
    if(*fast_boot==0xffffffff){
        //フラグ値のアドレスが4バイトアライメントにあるFlashメモリか確認する。
        if(((NyLPC_TUInt32)fast_boot)%4==0 && (!NyLPC_cOnchipFlashWriter_isOnchipFlash(fast_boot))){
            //書き込み
            NyLPC_cOnchipFlashWriter_write(fast_boot,&FAST_BOOT_DATA,4);
        }else{
            NyLPC_OnErrorGoto(Error);
        }
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
        return (const struct NyLPC_TMiMicConfigulation*)(MIMIC_CONFIGLATION_FLASH_SECTOR_ADDR);
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
    volatile const NyLPC_TUInt32* fast_boot=&(factory_default.fast_boot);
    return (*fast_boot)!=0xffffffff;
}


