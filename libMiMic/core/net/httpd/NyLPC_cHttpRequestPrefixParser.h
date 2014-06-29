/*********************************************************************************
 * PROJECT: MiMic
 * --------------------------------------------------------------------------------
 *
 * This file is part of MiMic
 * Copyright (C)2011 Ryo Iizuka
 *
 * MiMic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by　the Free Software Foundation, either version 3 of the　License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information please contact.
 *  http://nyatla.jp/
 *  <airmail(at)ebony.plala.or.jp> or <nyatla(at)nyatla.jp>
 *
 *********************************************************************************/
#ifndef NYLPC_cHttpRequestPrefixParser_H_
#define NYLPC_cHttpRequestPrefixParser_H_

#include "NyLPC_http.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NyLPC_TcHttpRequestPrefixParser NyLPC_TcHttpRequestPrefixParser_t;

#define NyLPC_TcHttpRequestPrefixParser_MAX_URL_LEN 24
/**
 * Httpリクエストの先頭部分を取り出します。
 */
struct NyLPC_TcHttpRequestPrefixParser
{
    NyLPC_THttpMethodType method;
    NyLPC_TChar _url[NyLPC_TcHttpRequestPrefixParser_MAX_URL_LEN];
};

void NyLPC_cHttpRequestPrefixParser_initialize(NyLPC_TcHttpRequestPrefixParser_t* i_inst);
#define NyLPC_cHttpRequestPrefixParser_finalize(i_inst) NyLPC_cHttpBasicHeaderParser_finalize(i_inst)

/**
 * THttpHeaderPrefix._urlに最大TcHttpRequestPrefixParser_MAX_URL_LEN文字のPath文字列を取得する。
 * 最大文字数よりも少なければ全てのPath文字列を蓄積し、多ければ先頭だけを蓄積する。
 * @return
 */
NyLPC_TBool NyLPC_cHttpRequestPrefixParser_parse(NyLPC_TcHttpRequestPrefixParser_t* i_inst,NyLPC_TiHttpPtrStream_t* i_stream);

const NyLPC_TChar* NyLPC_cHttpRequestPrefixParser_getUrlPrefix(const NyLPC_TcHttpRequestPrefixParser_t* i_inst);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* NYLPC_CHTTPSHORTHTTPHEADERPARSER_H_ */
