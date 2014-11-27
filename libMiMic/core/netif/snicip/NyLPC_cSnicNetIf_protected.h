/*
 * NyLPC_cSnicNetIf_protected.h
 *
 *  Created on: 2014/11/23
 *      Author: nyatla
 */
#include "NyLPC_stdlib.h"

#ifndef NYLPC_CSNICNETIF_PROTECTED_H_
#define NYLPC_CSNICNETIF_PROTECTED_H_
#ifdef __cplusplus
extern "C" {
#endif


NyLPC_TBool NyLPC_cSonicNetIf_initialize(void);
void NyLPC_cSonicNetIf_finalize(void);


NyLPC_TUInt8 NyLPC_cSnicNetIf_regieterObject(void* i_object);
void NyLPC_cSnicNetIf_unregieterObject(void* i_object);


void* NyLPC_cSnicNetIf_lockObjectById(NyLPC_TUInt8 i_id);
void NyLPC_cSnicNetIf_unlockObject(const void* i_object);

void NyLPC_cSnicNetIf_sendShortPayload(const void* i_buf,NyLPC_TUInt8 i_cmd,NyLPC_TUInt16 i_len);
void NyLPC_cSnicNetIf_startPayload(NyLPC_TUInt8 i_cmd,NyLPC_TUInt16 i_len);
void NyLPC_cSnicNetIf_sendPayload(const void* i_buf,NyLPC_TUInt16 i_len);
void NyLPC_cSnicNetIf_endPayload();
NyLPC_TUInt16 NyLPC_cSnicNetIf_getPayloadLength(const void* i_buf,NyLPC_TUInt16 i_len);

NyLPC_TBool NyLPC_cSnicNetIf_readPayload(void* i_buf,NyLPC_TUInt8 i_len);

void NyLPC_cSnicNetIf_seekPayload(NyLPC_TUInt16 i_seek);

#ifdef __cplusplus
}
#endif

#endif /* NYLPC_CSNICNETIF_PROTECTED_H_ */
