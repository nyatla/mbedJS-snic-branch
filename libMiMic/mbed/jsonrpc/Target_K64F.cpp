/**
 * @file
 *K64Fのターゲットテーブルです。
 */
#include "TargetTypes.h"
#include "RpcHandlerBase.h"
#ifdef TARGET_K64F
namespace MiMic
{

#define NS_FRDM_PTA     (PINID_FRDM_PTx)
#define NS_FRDM_PTB     (NS_FRDM_PTA+32)
#define NS_FRDM_PTC     (NS_FRDM_PTB+32)
#define NS_FRDM_PTD     (NS_FRDM_PTC+32)
#define NS_FRDM_PTE     (NS_FRDM_PTD+32)




const static struct TPinNameMapItem pin_table[]={
    {PTA0 ,NS_FRDM_PTA + 0},    {PTA1 ,NS_FRDM_PTA + 1},    {PTA2 ,NS_FRDM_PTA + 2},    {PTA3 ,NS_FRDM_PTA + 3},
    {PTA4 ,NS_FRDM_PTA + 4},    {PTA5 ,NS_FRDM_PTA + 5},    {PTA6 ,NS_FRDM_PTA + 6},    {PTA7 ,NS_FRDM_PTA + 7},
    {PTA8 ,NS_FRDM_PTA + 8},    {PTA9 ,NS_FRDM_PTA + 9},    {PTA10,NS_FRDM_PTA +10},    {PTA11,NS_FRDM_PTA +11},
    {PTA12,NS_FRDM_PTA +12},    {PTA13,NS_FRDM_PTA +13},    {PTA14,NS_FRDM_PTA +14},    {PTA15,NS_FRDM_PTA +15},
    {PTA16,NS_FRDM_PTA +16},    {PTA17,NS_FRDM_PTA +17},    {PTA18,NS_FRDM_PTA +18},    {PTA19,NS_FRDM_PTA +19},
    {PTA20,NS_FRDM_PTA +20},    {PTA21,NS_FRDM_PTA +21},    {PTA22,NS_FRDM_PTA +22},    {PTA23,NS_FRDM_PTA +23},
    {PTA24,NS_FRDM_PTA +24},    {PTA25,NS_FRDM_PTA +25},    {PTA26,NS_FRDM_PTA +26},    {PTA27,NS_FRDM_PTA +27},
    {PTA28,NS_FRDM_PTA +28},    {PTA29,NS_FRDM_PTA +29},    {PTA30,NS_FRDM_PTA +30},    {PTA31,NS_FRDM_PTA +31},

    {PTB0 ,NS_FRDM_PTB + 0},    {PTB1 ,NS_FRDM_PTB + 1},    {PTB2 ,NS_FRDM_PTB + 2},    {PTB3 ,NS_FRDM_PTB + 3},
    {PTB4 ,NS_FRDM_PTB + 4},    {PTB5 ,NS_FRDM_PTB + 5},    {PTB6 ,NS_FRDM_PTB + 6},    {PTB7 ,NS_FRDM_PTB + 7},
    {PTB8 ,NS_FRDM_PTB + 8},    {PTB9 ,NS_FRDM_PTB + 9},    {PTB10,NS_FRDM_PTB +10},    {PTB11,NS_FRDM_PTB +11},
    {PTB12,NS_FRDM_PTB +12},    {PTB13,NS_FRDM_PTB +13},    {PTB14,NS_FRDM_PTB +14},    {PTB15,NS_FRDM_PTB +15},
    {PTB16,NS_FRDM_PTB +16},    {PTB17,NS_FRDM_PTB +17},    {PTB18,NS_FRDM_PTB +18},    {PTB19,NS_FRDM_PTB +19},
    {PTB20,NS_FRDM_PTB +20},    {PTB21,NS_FRDM_PTB +21},    {PTB22,NS_FRDM_PTB +22},    {PTB23,NS_FRDM_PTB +23},
    {PTB24,NS_FRDM_PTB +24},    {PTB25,NS_FRDM_PTB +25},    {PTB26,NS_FRDM_PTB +26},    {PTB27,NS_FRDM_PTB +27},
    {PTB28,NS_FRDM_PTB +28},    {PTB29,NS_FRDM_PTB +29},    {PTB30,NS_FRDM_PTB +30},    {PTB31,NS_FRDM_PTB +31},

    {PTC0 ,NS_FRDM_PTC + 0},    {PTC1 ,NS_FRDM_PTC + 1},    {PTC2 ,NS_FRDM_PTC + 2},    {PTC3 ,NS_FRDM_PTC + 3},
    {PTC4 ,NS_FRDM_PTC + 4},    {PTC5 ,NS_FRDM_PTC + 5},    {PTC6 ,NS_FRDM_PTC + 6},    {PTC7 ,NS_FRDM_PTC + 7},
    {PTC8 ,NS_FRDM_PTC + 8},    {PTC9 ,NS_FRDM_PTC + 9},    {PTC10,NS_FRDM_PTC +10},    {PTC11,NS_FRDM_PTC +11},
    {PTC12,NS_FRDM_PTC +12},    {PTC13,NS_FRDM_PTC +13},    {PTC14,NS_FRDM_PTC +14},    {PTC15,NS_FRDM_PTC +15},
    {PTC16,NS_FRDM_PTC +16},    {PTC17,NS_FRDM_PTC +17},    {PTC18,NS_FRDM_PTC +18},    {PTC19,NS_FRDM_PTC +19},
    {PTC20,NS_FRDM_PTC +20},    {PTC21,NS_FRDM_PTC +21},    {PTC22,NS_FRDM_PTC +22},    {PTC23,NS_FRDM_PTC +23},
    {PTC24,NS_FRDM_PTC +24},    {PTC25,NS_FRDM_PTC +25},    {PTC26,NS_FRDM_PTC +26},    {PTC27,NS_FRDM_PTC +27},
    {PTC28,NS_FRDM_PTC +28},    {PTC29,NS_FRDM_PTC +29},    {PTC30,NS_FRDM_PTC +30},    {PTC31,NS_FRDM_PTC +31},

    {PTD0 ,NS_FRDM_PTD + 0},    {PTD1 ,NS_FRDM_PTD + 1},    {PTD2 ,NS_FRDM_PTD + 2},    {PTD3 ,NS_FRDM_PTD + 3},
    {PTD4 ,NS_FRDM_PTD + 4},    {PTD5 ,NS_FRDM_PTD + 5},    {PTD6 ,NS_FRDM_PTD + 6},    {PTD7 ,NS_FRDM_PTD + 7},
    {PTD8 ,NS_FRDM_PTD + 8},    {PTD9 ,NS_FRDM_PTD + 9},    {PTD10,NS_FRDM_PTD +10},    {PTD11,NS_FRDM_PTD +11},
    {PTD12,NS_FRDM_PTD +12},    {PTD13,NS_FRDM_PTD +13},    {PTD14,NS_FRDM_PTD +14},    {PTD15,NS_FRDM_PTD +15},
    {PTD16,NS_FRDM_PTD +16},    {PTD17,NS_FRDM_PTD +17},    {PTD18,NS_FRDM_PTD +18},    {PTD19,NS_FRDM_PTD +19},
    {PTD20,NS_FRDM_PTD +20},    {PTD21,NS_FRDM_PTD +21},    {PTD22,NS_FRDM_PTD +22},    {PTD23,NS_FRDM_PTD +23},
    {PTD24,NS_FRDM_PTD +24},    {PTD25,NS_FRDM_PTD +25},    {PTD26,NS_FRDM_PTD +26},    {PTD27,NS_FRDM_PTD +27},
    {PTD28,NS_FRDM_PTD +28},    {PTD29,NS_FRDM_PTD +29},    {PTD30,NS_FRDM_PTD +30},    {PTD31,NS_FRDM_PTD +31},

    {PTE0 ,NS_FRDM_PTE + 0},    {PTE1 ,NS_FRDM_PTE + 1},    {PTE2 ,NS_FRDM_PTE + 2},    {PTE3 ,NS_FRDM_PTE + 3},
    {PTE4 ,NS_FRDM_PTE + 4},    {PTE5 ,NS_FRDM_PTE + 5},    {PTE6 ,NS_FRDM_PTE + 6},    {PTE7 ,NS_FRDM_PTE + 7},
    {PTE8 ,NS_FRDM_PTE + 8},    {PTE9 ,NS_FRDM_PTE + 9},    {PTE10,NS_FRDM_PTE +10},    {PTE11,NS_FRDM_PTE +11},
    {PTE12,NS_FRDM_PTE +12},    {PTE13,NS_FRDM_PTE +13},    {PTE14,NS_FRDM_PTE +14},    {PTE15,NS_FRDM_PTE +15},
    {PTE16,NS_FRDM_PTE +16},    {PTE17,NS_FRDM_PTE +17},    {PTE18,NS_FRDM_PTE +18},    {PTE19,NS_FRDM_PTE +19},
    {PTE20,NS_FRDM_PTE +20},    {PTE21,NS_FRDM_PTE +21},    {PTE22,NS_FRDM_PTE +22},    {PTE23,NS_FRDM_PTE +23},
    {PTE24,NS_FRDM_PTE +24},    {PTE25,NS_FRDM_PTE +25},    {PTE26,NS_FRDM_PTE +26},    {PTE27,NS_FRDM_PTE +27},
    {PTE28,NS_FRDM_PTE +28},    {PTE29,NS_FRDM_PTE +29},    {PTE30,NS_FRDM_PTE +30},    {PTE31,NS_FRDM_PTE +31},

    //LED
    {LED_RED,PINID_FRDM_OTHER_LEDx+0},{LED_GREEN,PINID_FRDM_OTHER_LEDx+1},{LED_BLUE,PINID_FRDM_OTHER_LEDx+2},
    //Push buttons
    {SW2,PINID_FRDM_OTHER_SWx+2},{SW3,PINID_FRDM_OTHER_SWx+3},

    // Other mbed Pin Names 
    {LED1  ,PINID_OTHER_MBED_LEDx+0},  {LED2  ,PINID_OTHER_MBED_LEDx+1},  {LED3  ,PINID_OTHER_MBED_LEDx+2},  {LED4  ,PINID_OTHER_MBED_LEDx+3},

    {USBTX,PINID_OTHER_MBED_USBx+0},{USBRX,PINID_OTHER_MBED_USBx+1},

    // Arch Pro Pin Names(Arudino)
    {D0 ,PINID_ARCH_PRO_Dx+0},  {D1 ,PINID_ARCH_PRO_Dx+1},  {D2,PINID_ARCH_PRO_Dx+2},  {D3,PINID_ARCH_PRO_Dx+3},
    {D4 ,PINID_ARCH_PRO_Dx+4},  {D5 ,PINID_ARCH_PRO_Dx+5},  {D6,PINID_ARCH_PRO_Dx+6},  {D7,PINID_ARCH_PRO_Dx+7},
    {D8 ,PINID_ARCH_PRO_Dx+8},  {D9 ,PINID_ARCH_PRO_Dx+9},  {D10,PINID_ARCH_PRO_Dx+10},{D11,PINID_ARCH_PRO_Dx+11},
    {D12,PINID_ARCH_PRO_Dx+12}, {D13,PINID_ARCH_PRO_Dx+13}, {D14,PINID_ARCH_PRO_Dx+14},{D15,PINID_ARCH_PRO_Dx+15},
   
    {A0,PINID_ARCH_PRO_Ax+0},  {A1,PINID_ARCH_PRO_Ax+1},  {A2,PINID_ARCH_PRO_Ax+2},  {A3,PINID_ARCH_PRO_Ax+3},
    {A4,PINID_ARCH_PRO_Ax+4},  {A5,PINID_ARCH_PRO_Ax+5},
 
    {NC,PINID_NC}
};

#define NUMBER_OF_PINMODE 4
const static struct TPinModeMapItem pinmode_table[]=
{
    {PullUp     ,PINMODEID+0},
    {PullDown   ,PINMODEID+1},
    {PullNone   ,PINMODEID+2},
    {PullDefault,PINMODEID+1}
};
#define NUMBER_OF_PORT_NAME 5
const static struct TPortNameMapItem portname_table[]=
{
    {PortA  ,PORTID+0},
    {PortB  ,PORTID+1},
    {PortC  ,PORTID+2},
    {PortD  ,PORTID+3},
    {PortE  ,PORTID+4},
};

PinName RpcHandlerBase::pinId2PinName(unsigned int i_id)
{
    for(int i=0;pin_table[i].name!=NC;i++){
        if(i_id==pin_table[i].id){
            return pin_table[i].name;
        }
    }
    return NC;
}

PinMode RpcHandlerBase::pinmodeId2PinMode(unsigned int i_id)
{
    for(int i=0;i<NUMBER_OF_PINMODE;i++){
        if(i_id==pinmode_table[i].id){
            return pinmode_table[i].mode;
        }
    }
    return PullDefault;
}
PortName RpcHandlerBase::portId2PortName(unsigned int i_id)
{
    for(int i=0;i<NUMBER_OF_PORT_NAME;i++){
        if(i_id==portname_table[i].id){
            return portname_table[i].port;
        }
    }
    return PortA;
}

}
#endif

