/*
 * NyLPC_cMiMicEnv.h
 *
 *  Created on: 2013/03/08
 *      Author: nyatla
 */

#ifndef NYLPC_CMIMICENV_H_
#define NYLPC_CMIMICENV_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "NyLPC_stdlib.h"


#define NyLPC_cMiMicEnv_VERSION         1
#define NyLPC_cMiMicEnv_SHORT_NAME      2
#define NyLPC_cMiMicEnv_ETHERNET_PHY    3
#define NyLPC_cMiMicEnv_MCU_NAME        4

const char* NyLPC_cMiMicEnv_getStrProperty(NyLPC_TUInt16 i_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CMIMICENV_H_ */
