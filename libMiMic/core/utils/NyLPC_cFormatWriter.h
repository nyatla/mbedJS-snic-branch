#include "NyLPC_stdlib.h"
#include <stdarg.h>
#ifndef NYLPC_CFORMATTEXTWRITER_H_
#define NYLPC_CFORMATTEXTWRITER_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef NyLPC_TBool(*NyLPC_cFormatWriter_printHandler)(void* i_inst,const void* i_buf,NyLPC_TUInt32 i_len);
/**
 * printfライクな書式出力関数です。i_handlerへi_fmtに示される書式文字列を出力します。
 * @param i_fmt
 * フォーマット文字列。以下の形式をサポートします。
 * %d       int値
 * %u       uint値
 * %c       char値
 * %%       '%'
 * %s,%.*s  文字列
 * 独自拡張
 * %.*B     BYTE配列をXX形式で並べた文字列
 *
 */
NyLPC_TBool NyLPC_cFormatWriter_print(NyLPC_cFormatWriter_printHandler i_handler,void* i_inst,const NyLPC_TChar* i_fmt,va_list args);

/**
 * 書式文字列を出力した時のバイト長さを求めます。
 */
NyLPC_TInt16 NyLPC_cFormatWriter_length(const NyLPC_TChar* i_fmt,va_list args);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* NYLPC_CFORMATTEXTREADER_H_ */
