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
#ifndef NYLPC_CHTTPHEADERWRITER_H_
#define NYLPC_CHTTPHEADERWRITER_H_

#include "NyLPC_stdlib.h"
#include "NyLPC_netif.h"
#include "NyLPC_cHttpStream.h"
#include "NyLPC_cHttpBasicHeaderParser.h"
#include "NyLPC_cHttpdConfig.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct NyLPC_TcHttpHeaderWriter NyLPC_TcHttpHeaderWriter_t;
struct NyLPC_TcHttpHeaderWriter
{
    NyLPC_TUInt8 _is_chunked;
    NyLPC_TUInt8 _is_close;
    //異常状態をチェックするためのフラグ
    NyLPC_TUInt8 _is_error;
    
    NyLPC_TUInt32 _content_length;
    NyLPC_TUInt32 _size_of_sent;

    NyLPC_TiHttpPtrStream_t* _ref_stream;
};

/**
 * Httpリクエストヘッダに対応したHttpヘッダライタを構築します。
 * インスタンスは、正常/異常の2状態を持ちます。
 * <p>初期値について -
 * ConnectionCloseは有効です。
 * </p>
 */
NyLPC_TBool NyLPC_cHttpHeaderWriter_initialize(NyLPC_TcHttpHeaderWriter_t* i_inst,NyLPC_TiHttpPtrStream_t* i_ref_stream,const struct NyLPC_THttpBasicHeader* i_req_header);

#define NyLPC_cHttpHeaderWriter_finalize(i)
NyLPC_TBool NyLPC_cHttpHeaderWriter_writeRequestHeader(NyLPC_TcHttpHeaderWriter_t* i_inst,NyLPC_THttpMethodType i_method,const struct NyLPC_TIPv4Addr* i_host,NyLPC_TUInt16 i_port,const NyLPC_TChar* i_path);
NyLPC_TBool NyLPC_cHttpHeaderWriter_writeResponseHeader(NyLPC_TcHttpHeaderWriter_t* i_inst,NyLPC_TUInt16 i_status);
NyLPC_TBool NyLPC_cHttpHeaderWriter_writeMessage(NyLPC_TcHttpHeaderWriter_t* i_inst,const NyLPC_TChar* i_name,const NyLPC_TChar* i_field);
/**
 * \r\n区切りのメッセージをそのままヘッダに挿入します。i_additional_headerの終端は\r\nで閉じてください。
 */
NyLPC_TBool NyLPC_cHttpHeaderWriter_writeRawMessage(NyLPC_TcHttpHeaderWriter_t* i_inst,const NyLPC_TChar* i_additional_header);
NyLPC_TBool NyLPC_cHttpHeaderWriter_close(NyLPC_TcHttpHeaderWriter_t* i_inst);
void NyLPC_cHttpHeaderWriter_setContentLength(NyLPC_TcHttpHeaderWriter_t* i_inst,NyLPC_TUInt32 i_content_length);
/**
 * ConnectionCloseのON/OFFを設定します。
 */
void NyLPC_cHttpHeaderWriter_setConnectionClose(NyLPC_TcHttpHeaderWriter_t* i_inst,NyLPC_TBool i_is_close);
void NyLPC_cHttpHeaderWriter_setChunked(NyLPC_TcHttpHeaderWriter_t* i_inst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CHTTPHEADERWRITER_H_ */
