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
#ifndef NYLPC_CHTTPNULLHTTPHEADERPARSER_H_
#define NYLPC_CHTTPNULLHTTPHEADERPARSER_H_
#include "NyLPC_cHttpBasicHeaderParser.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * URLが最大31文字までの、短いHttpリクエストを処理します。
 * このクラスは、NyLPC_TBasicHttpHeader_tにキャストできます。
 */
typedef struct NyLPC_TcHttpNullReqestHeaderParser NyLPC_TcHttpNullRequestHeaderParser_t;



struct NyLPC_TcHttpNullReqestHeaderParser{
    NyLPC_TcHttpBasicHeaderParser_t super;
    struct NyLPC_THttpBasicHeader _header;
};

void NyLPC_cHttpNullRequestHeaderParser_initialize(NyLPC_TcHttpNullRequestHeaderParser_t* i_inst);

#define NyLPC_cHttpNullRequestHeaderParser_finalize(i_inst) NyLPC_cHttpBasicHeaderParser_finalize(i_inst);

/**
 * parseInit,parseStream,parseFinishを一括で実行します。
 */
NyLPC_TBool NyLPC_cHttpNullRequestHeaderParser_parse(NyLPC_TcHttpNullRequestHeaderParser_t* i_inst,NyLPC_TcHttpStream_t* i_stream);


/** override
 */
#define NyLPC_cHttpNullRequestHeaderParser_parseInit(i_inst) NyLPC_cHttpBasicHeaderParser_parseInit(&((i_inst)->super),&((i_inst)->_header))

/** override
 */
#define NyLPC_cHttpNullRequestHeaderParser_parseFinish(i_inst) NyLPC_cHttpBasicHeaderParser_parseFinish(&((i_inst)->super),&((i_inst)->_header))

/** override
 */
#define NyLPC_cHttpNullRequestHeaderParser_parseChar(i_inst,i_c,i_size) NyLPC_cHttpBasicHeaderParser_parseChar(&((i_inst)->super),i_c,i_size,&((i_inst)->_header));

/** override
 */
#define NyLPC_cHttpNullRequestHeaderParser_parseStream(i_inst,i_stream) NyLPC_cHttpBasicHeaderParser_parseStream(&((i_inst)->super),i_stream,&((i_inst)->_header))



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CHTTPSHORTHTTPHEADERPARSER_H_ */
