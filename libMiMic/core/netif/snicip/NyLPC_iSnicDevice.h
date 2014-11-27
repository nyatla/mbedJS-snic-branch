/*
 * NyLPC_cSnicIo.h
 *
 *  Created on: 2014/11/20
 *      Author: nyatla
 */

#ifndef NYLPC_TiSnicDevice_H_
#define NYLPC_TiSnicDevice_H_
#include "NyLPC_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void(*NyLPC_TiSnicDevice_finalize)(void);
//
typedef const void*(*NyLPC_TiSnicDevice_pread)(NyLPC_TUInt16* i_len);
typedef void(*NyLPC_TiSnicDevice_pseek)(NyLPC_TUInt16 i_len);
typedef void(*NyLPC_TiSnicDevice_write)(const void* i_buf,NyLPC_TUInt16 i_length);
typedef void(*NyLPC_TiSnicDevice_reset)(void);

struct NyLPC_TiSnicDevice_Interface{
	NyLPC_TiSnicDevice_finalize finalize;
	NyLPC_TiSnicDevice_pread pread;
	NyLPC_TiSnicDevice_pseek pseek;
	NyLPC_TiSnicDevice_write write;
	NyLPC_TiSnicDevice_reset reset;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CSNICIO_H_ */
