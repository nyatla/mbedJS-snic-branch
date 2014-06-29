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

#ifndef NYLPC_CHTTPBODYWRITER_H_
#define NYLPC_CHTTPBODYWRITER_H_

#include "NyLPC_cHttpHeaderWriter.h"
#include "NyLPC_cHttpStream.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NyLPC_TcHttpBodyWriter NyLPC_TcHttpBodyWriter_t;
struct NyLPC_TcHttpBodyWriter
{
    NyLPC_TiHttpPtrStream_t* _ref_stream;
    NyLPC_TUInt8 _is_chunked;
    NyLPC_TUInt8 _is_error;
    NyLPC_TUInt32 _size_of_sent;
    /** _is_chunked==NyLPC_TUInt8_TRUEの時のみ有効. ContentLengthの値*/
    NyLPC_TUInt32 _content_length;
};

/**
 * 送信モードの初期値は、contentlength=0,chunked無しです。
 */
void NyLPC_cHttpBodyWriter_initialize(NyLPC_TcHttpBodyWriter_t* i_inst,NyLPC_TcHttpStream_t* i_stream);
#define NyLPC_cHttpBodyWriter_finalize(i);
/** Chunkedエンコーディングで送信する場合*/
void NyLPC_cHttpBodyWriter_setChunked(NyLPC_TcHttpBodyWriter_t* i_inst);
/** Contentlengthで送信する場合*/
void NyLPC_cHttpBodyWriter_setContentLength(NyLPC_TcHttpBodyWriter_t* i_inst,NyLPC_TUInt32 i_content_length);
NyLPC_TBool NyLPC_cHttpBodyWriter_write(NyLPC_TcHttpBodyWriter_t* i_inst,const void* i_buf,NyLPC_TUInt32 i_len);
NyLPC_TBool NyLPC_cHttpBodyWriter_close(NyLPC_TcHttpBodyWriter_t* i_inst);
NyLPC_TBool NyLPC_cHttpBodyWriter_format(NyLPC_TcHttpBodyWriter_t* i_inst,const NyLPC_TChar* i_fmt,...);
NyLPC_TBool NyLPC_cHttpBodyWriter_formatV(NyLPC_TcHttpBodyWriter_t* i_inst,const NyLPC_TChar* i_fmt,va_list i_args);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CHTTPBODYWRITER_H_ */
