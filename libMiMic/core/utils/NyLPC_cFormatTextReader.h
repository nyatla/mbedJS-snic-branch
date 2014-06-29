/**
 * @file
 * NyLPC_cFormattextReader.h
 * 書式テキストを読み出す為の関数群です。
 *  Created on: 2013/04/20
 *      Author: nyatla
 */
#include "NyLPC_stdlib.h"

#ifndef NYLPC_CFORMATTEXTREADER_H_
#define NYLPC_CFORMATTEXTREADER_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * [a-zA-Z0-9_-]で構成されるワードを取得します。
 * This function peek a word from string.
 * @return
 * size of seeked.
 */
NyLPC_TInt32 NyLPC_cFormatTextReader_readWord(const NyLPC_TChar* buf,const NyLPC_TChar** top);

/**
 * 文字列からIPアドレスを取得します。
 * [:number:]\.[:number:]\.[:number:]\.[:number:]
 * [:number:]は0-255までに制限されます。
 * @param v
 * uint8[4]
 * @return
 * next pointer
 */
NyLPC_TInt32 NyLPC_cFormatTextReader_readIpAddr(const NyLPC_TChar* buf,NyLPC_TUInt8* v);

/**
 * 文字列からMACアドレスを取得します。
 * [:hex:]:[:hex:]:[:hex:]:[:hex:]
 * @param v
 * uint8[6]
 */
NyLPC_TInt32 NyLPC_cFormatTextReader_readMacAddr(const NyLPC_TChar* buf,NyLPC_TUInt8* v);

/**
 * 連続するスペースを読み飛ばします。
 */
NyLPC_TInt32 NyLPC_cFormatTextReader_seekSpace(const NyLPC_TChar* s);

/**
 * 文字列から10進数の数値を読み出します。
 * @return
 * 読み飛ばしたスペース
 */
NyLPC_TInt32 NyLPC_cFormatTextReader_readUInt(const NyLPC_TChar* buf,NyLPC_TUInt32* v);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* NYLPC_CFORMATTEXTREADER_H_ */
