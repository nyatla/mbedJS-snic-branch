/*
 * NyLPC_cUrlEncode.h
 *
 *  Created on: 2013/06/11
 *      Author: nyatla
 */

#ifndef NYLPC_CURLENCODE_H_
#define NYLPC_CURLENCODE_H_

#include "NyLPC_stdlib.h"
#include <stdarg.h>
#include <ctype.h>

typedef struct NyLPC_TcUrlEncode NyLPC_TcUrlEncode_t;

typedef NyLPC_TUInt32 NyLPC_TcUrlEncode_ST;
#define NyLPC_TcUrlEncode_ST_NEXT   2
#define NyLPC_TcUrlEncode_ST_DONE   1
#define NyLPC_TcUrlEncode_ST_ERROR  0

struct NyLPC_TcUrlEncode
{
    /**一時バッファ */
    NyLPC_TChar v;
    /** バッファに蓄積してる長さ */
    NyLPC_TInt8 _len;
};

NyLPC_TBool NyLPC_cUrlEncode_initialize(NyLPC_TcUrlEncode_t* i_inst);

#define NyLPC_cUrlEncode_finalize(i_inst)

#define NyLPC_cUrlEncode_reset(i_inst) (i_inst)->_len=0


NyLPC_TcUrlEncode_ST NyLPC_cUrlEncode_decode(NyLPC_TcUrlEncode_t* i_inst,NyLPC_TChar c,NyLPC_TChar* out);


#endif /* NYLPC_CURLENCODE_H_ */
