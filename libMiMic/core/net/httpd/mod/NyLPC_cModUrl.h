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
#ifndef NYLPC_CMODURL_H_
#define NYLPC_CMODURL_H_

#include "NyLPC_http.h"
#include "../NyLPC_cHttpdConnection.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * URLの取得モード
 */
typedef NyLPC_TUInt8 NyLPC_cModUrl_ParseMode;
/**
 * URL文字列全てを取得する。
 */
#define NyLPC_cModUrl_ParseMode_ALL       0x00
/**
 * URLのパス部分だけを取得する。
 * /absolutepath/?queryや/absolutepath/#bookmarkの/absolutepath/だけを取り出します。
 */
#define NyLPC_cModUrl_ParseMode_PATH_ONLY 0x01


/**
 * コネクションヘッダからURLを取得します。
 */
typedef struct NyLPC_TcModUrl NyLPC_TcModUrl_t;


struct NyLPC_TcModUrl
{
    struct NyLPC_THttpBasicHeader _header;
    NyLPC_TcHttpBodyParser_t _body_parser;
};

/**
 * コンストラクタ。
 */
void NyLPC_cModUrl_initialize(NyLPC_TcModUrl_t* i_inst);
void NyLPC_cModUrl_finalize(NyLPC_TcModUrl_t* i_inst);

/**
 * コネクションから全てのURLをパースします。
 * NyLPC_cModUrl_execute2のラッパーです。i_i_pass_prefix_len=0,i_mode=NyLPC_cModUrl_ParseMode_ALLを指定したときと同じ動作をします。
 * @param o_url_buf
 * 取得したURL文字列。i_pass_prefix_lenが指定されている場合は、URLの後半部分です。
 * 戻り値がTRUEの場合に有効です。
 * @return
 * 処理が成功するとTRUEを返します。FALSEの場合、URLのパースに失敗しています。
 * TRUEの場合は引き続きResponseの送信処理をしてください。FALSEの場合はそのままリクエストハンドラを終了してください。
 */
#define NyLPC_cModUrl_execute(i_inst,i_connection,o_url_buf,i_length_buf) NyLPC_cModUrl_execute2((i_inst),(i_connection),(o_url_buf),(i_length_buf),0,NyLPC_cModUrl_ParseMode_ALL)

/**
 * コネクションからURLをパースします。
 * @param i_i_pass_prefix_len
 * URLの先頭から取り除く文字数。
 * 全てのURLを得るには0を指定します。
 * @param o_url_buf
 * 取得したURL文字列。i_pass_prefix_lenが指定されている場合は、URLの後半部分です。
 * 戻り値がTRUEの場合に有効です。
 * @param i_mode
 * URLの取得モードです。
 * <ul>
 * <li>NyLPC_cModUrl_ParseMode_ALL - URL全てを取得します。</li>
 * <li>NyLPC_cModUrl_ParseMode_PATH_ONLY - クエリ文字列、ブックマークを除く文字列を取得します。</li>
 * </ul>
 * @return
 * 処理が成功するとTRUEを返します。FALSEの場合、URLのパースに失敗しています。
 * TRUEの場合は引き続きResponseの送信処理をしてください。FALSEの場合はそのままリクエストハンドラを終了してください。
 */
NyLPC_TBool NyLPC_cModUrl_execute2(NyLPC_TcModUrl_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection,char* o_url_buf,NyLPC_TInt16 i_length_buf,NyLPC_TInt16 i_pass_prefix_len, NyLPC_cModUrl_ParseMode i_mode);

/**
 * HTTPストリームからBODY部分を読み出す。
 * この関数はexecuteが成功した後に利用できます。
 * @param read_len
 * 読み出したバイト数
 * @return
 * -1:エラー
 * 0:終端
 * 1以上:読み出したデータサイズ
 */
NyLPC_TInt16 NyLPC_cModUrl_readBody(NyLPC_TcModUrl_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection,void* i_buf,NyLPC_TInt16 i_buf_size);

/**
 * basicヘッダオブジェクトを返します。
 */
const struct NyLPC_THttpBasicHeader* NyLPC_cModUrl_getHeader(const NyLPC_TcModUrl_t* i_inst);

/**
 * Methodタイプを返します。
 */
NyLPC_THttpMethodType NyLPC_cModUrl_getMethod(const NyLPC_TcModUrl_t* i_inst);






#ifdef __cplusplus
}
#endif /* __cplusplus */




#endif /* NYLPC_CMODURL_H_ */
