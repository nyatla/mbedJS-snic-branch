/*
 * NyLPC_cUrlReader.h
 *
 *  Created on: 2013/04/22
 *      Author: nyatla
 */

#ifndef NYLPC_cUrlReader_H_
#define NYLPC_cUrlReader_H_
#include "NyLPC_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * URLからパス文字列のポインタと長さを返します。
 */
NyLPC_TBool NyLPC_cUrlReader_getPath(const NyLPC_TChar* i_src,const NyLPC_TChar** path,NyLPC_TInt32* path_len);

/**
 * 指定したURLクエリキーの値を探します。
 * @return
 * クエリ値の直前のポインタです。
 * 例えばキーがabcの時、探索する文字列が[^\?]*\?abc=cdfの場合、=の位置を返します。[^\?]*\?abc&cdfの場合、&の位置を返します。
 * cdfの場合、[^\?]*\?abc=cdfはNULL,[^\?]*\?abc&cdfは終端'\0'の位置を返します。
 * この関数は補助的なものです。Query文字列から値を取得するときは、getStr,getUInt,getInt等を使用してください。
 */
const NyLPC_TChar* NyLPC_cUrlReader_findKeyValue(const NyLPC_TChar* i_src,const NyLPC_TChar* i_key_name);
/**
 * URLから指定キー[:KEY:]のURLクエリ値[:VALUE:]を取得します。
 * [:query:] := [^\?]*\?(&[:KEY:](=[:VALUE:])?&)*([:KEY:](=[:VALUE:])?)
 * [:KEY:]   := [a-zA-Z0-9_-]
 * [:VALUE:] := [^\#&]
 */
NyLPC_TBool NyLPC_cUrlReader_getStr(const NyLPC_TChar* i_src,const NyLPC_TChar* i_key_name,const NyLPC_TChar** str,NyLPC_TInt32* str_len);

/**
 * URLから指定キー[:KEY:]のURLクエリ値[:VALUE:]を32bit-unsigned値で取得します。
 * [:query:] := [^\?]*\?(&[:KEY:](=[:VALUE:])?&)*([:KEY:](=[:VALUE:])?)
 * [:KEY:]   := [a-zA-Z0-9_-]
 * [:VALUE:] := [:DIGIT:]+|0x[:HEX:]
 */
NyLPC_TBool NyLPC_cUrlReader_getUInt(const NyLPC_TChar* i_buf,const NyLPC_TChar* i_key_name,NyLPC_TUInt32* value);
/**
 * URLから指定キー[:KEY:]のURLクエリ値[:VALUE:]を32bit-unsigned値で取得します。
 * [:query:] := [^\?]*\?(&[:KEY:](=[:VALUE:])?&)*([:KEY:](=[:VALUE:])?)
 * [:KEY:]   := [a-zA-Z0-9_-]
 * [:VALUE:] := -?[:DIGIT:]+|0x[:HEX:]
 */
NyLPC_TBool NyLPC_cUrlReader_getInt(const NyLPC_TChar* i_buf,const NyLPC_TChar* i_key_name,NyLPC_TInt32* value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


