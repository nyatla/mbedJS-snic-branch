/*
 * NyLPC_cBase64.h
 *
 *  Created on: 2013/09/04
 *      Author: nyatla
 */

#ifndef NYLPC_CBASE64_H_
#define NYLPC_CBASE64_H_
#include "NyLPC_stdlib.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @param i_src
 * @param length
 * @param i_dest
 * Base64文字列の出力領域. length/3*4+1の長さが必要。
 */
void NyLPC_cBase64_encode(const NyLPC_TChar* i_src,NyLPC_TUInt16 length,char* i_dest);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CBASE64_H_ */

