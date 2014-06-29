/*
 * NyLPC_cMiMicDbCompiler.h
 *
 *  Created on: 2011/09/10
 *      Author: nyatla
 */

#ifndef NYLPC_CMIMICDBCOMPILER_H_
#define NYLPC_CMIMICDBCOMPILER_H_
#include "NyLPC_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NyLPC_TcMiMicDbCompiler NyLPC_TcMiMicDbCompiler_t;


typedef NyLPC_TUInt8 NyLPC_TcMiMicDbCompiler_ERROR;
//不明なエラー
#define NyLPC_TcMiMicDbCompiler_ERROR_UNKNOWN              0x00
//出力バッファが足りない。
#define NyLPC_TcMiMicDbCompiler_ERROR_OUT_BUFFER_TOO_SHORT 0x01
//BCの形式がおかしい。
#define NyLPC_TcMiMicDbCompiler_ERROR_FORMAT               0x02
//BCフラグメント単位がおかしい。(途中で終わってる？)
#define NyLPC_TcMiMicDbCompiler_ERROR_FRAGMENT_UNIT        0x03


typedef NyLPC_TUInt8 NyLPC_TcMiMicDbCompiler_RET;
#define NyLPC_TcMiMicDbCompiler_RET_OK 1
#define NyLPC_TcMiMicDbCompiler_RET_CONTINUE 2
#define NyLPC_TcMiMicDbCompiler_RET_ERROR 3



struct NyLPC_TcMiMicDbCompiler
{
    NyLPC_TUInt8 _bc_fragment_len;
    NyLPC_TcMiMicDbCompiler_ERROR error_reason;//エラー理由
    NyLPC_TChar _tmp[8];//テンポラリ
};

void NyLPC_cMiMicDbCompiler_initialize(NyLPC_TcMiMicDbCompiler_t* i_inst);
#define NyLPC_cMiMicDbCompiler_finalize(i);

NyLPC_TcMiMicDbCompiler_RET NyLPC_cMiMicDbCompiler_compileFragment2(NyLPC_TcMiMicDbCompiler_t* i_inst,NyLPC_TChar i_bc,NyLPC_TUInt32* o_val);
NyLPC_TUInt16 NyLPC_cMiMicDbCompiler_compile(NyLPC_TcMiMicDbCompiler_t* i_inst,const struct NyLPC_TCharArrayPtr* i_bc,struct NyLPC_TUInt32ArrayPtr* o_val);
/**
 * フラグメントを集積中であるかを返します。TRUEのとき、パース中であり、フラグメント待ち状態です。
 */
#define NyLPC_cMiMicDbCompiler_hasFragment(i) ((i)->_bc_fragment_len>0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CMIMICDBCOMPILER_H_ */
