/**
 * @file
 * このファイルはTarget定義ファイルからのみincludeして下さい。
 */
#pragma once
#include "mbed.h"

namespace MiMic
{

/*
 * mbed1768,LPC4088に定義されているピン名の名前空間を定義します。
 * 一時的にPINIDを加える場合は、NS_USER_PINから始まるIDを使用してください。
 */

/** Px_yピンに該当するピンID。 P0_0,P0_1...P0_31,P1_0の順番*/
#define PINID_LPC				0x00010000
/** */
#define PINID_MBED_DIP			0x00020000
#define PINID_OTHER_MBED		0x00030000
#define PINID_OTHER_MBED_LEDx	(PINID_OTHER_MBED | 0x00000000)
#define PINID_OTHER_MBED_USBx	(PINID_OTHER_MBED | 0x00000100)
#define PINID_ARCH_PRO			0x00040000
#define PINID_ARCH_PRO_Dx		(PINID_ARCH_PRO | 0x00000000)
#define PINID_ARCH_PRO_Ax		(PINID_ARCH_PRO | 0x00000100)
#define PINID_ARCH_PRO_I2C_x	(PINID_ARCH_PRO | 0x00000200)
#define PINID_FRDM_PTx			0x00050000
#define PINID_FRDM_OTHER		0x00060000
#define PINID_FRDM_OTHER_LEDx	(PINID_FRDM_OTHER|0x00000000)
#define PINID_FRDM_OTHER_SWx	(PINID_FRDM_OTHER|0x00000100)

#define PINID_USER				0x40000000
#define PINID_NC				0x7fffffff

#define PINMODEID				0x00010000
#define PORTID					0x00010000

struct TPinNameMapItem{
	PinName name;
	unsigned int id;
};
struct TPinModeMapItem{
	PinMode mode;
	unsigned int id;
};
struct TPortNameMapItem{
	PortName port;
	unsigned int id;
};

}
