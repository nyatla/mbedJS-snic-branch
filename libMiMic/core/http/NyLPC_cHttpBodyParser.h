#ifndef NyLPC_TcHttpBodyParser_H
#define NyLPC_TcHttpBodyParser_H
#include "NyLPC_cHttpBasicBodyParser.h"
#include "NyLPC_stdlib.h"



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct NyLPC_TcHttpBodyParser NyLPC_TcHttpBodyParser_t;

struct NyLPC_TcHttpBodyParser
{
    NyLPC_TcHttpBasicBodyParser_t _super;
    NyLPC_TChar* ref_buf;
    NyLPC_TUInt16 buf_size;
    NyLPC_TUInt16 len;
};



void NyLPC_cHttpBodyParser_initialize(NyLPC_TcHttpBodyParser_t* i_inst);
void NyLPC_cHttpBodyParser_finalize(NyLPC_TcHttpBodyParser_t* i_inst);

#define NyLPC_cHttpBodyParser_getState(i_inst) NyLPC_cHttpBasicBodyParser_getState(&((i_inst)->_super))
#define NyLPC_cHttpBodyParser_parseInit(i_inst,i_info) NyLPC_cHttpBasicBodyParser_parseInit(&((i_inst)->_super),(i_info))
#define NyLPC_cHttpBodyParser_parseFinish(i_inst) NyLPC_cHttpBasicBodyParser_parseFinish(&((i_inst)->_super))

/**
 * ストリームからHTTPBodyを読み出す。
 * @param i_out
 * 読み出したデータサイズ。戻り値trueの場合のみ有効。0の場合終端。
 * @return
 * エラーの発生状況
 */
NyLPC_TBool NyLPC_cHttpBodyParser_parseStream(NyLPC_TcHttpBodyParser_t* i_inst,NyLPC_TiHttpPtrStream_t* i_stream,NyLPC_TChar* i_buf,NyLPC_TInt16 i_buf_size,NyLPC_TInt16* i_out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
