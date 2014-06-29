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

#ifndef NyLPC_TcHttpBasicBodyParser_H
#define NyLPC_TcHttpBasicBodyParser_H
#include "NyLPC_stdlib.h"
#include "NyLPC_cHttpStream.h"
#include "NyLPC_cHttpBasicHeaderParser.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */





/**
 * このクラスは、HttpBodyのパーサを定義します。
 * BasicBodyParserは結果値をコールバック関数で通知します。
 */
typedef struct NyLPC_TcHttpBasicBodyParser NyLPC_TcHttpBasicBodyParser_t;

/**
 * BodyParserのステータス定義値。
 */
typedef NyLPC_TUInt8 NyLPC_TcHttpBasicBodyParser_ST;
#define NyLPC_TcHttpBasicBodyParser_ST_CHUNK_HEADER_START   ((NyLPC_TcHttpBasicBodyParser_ST)0x11)//ヘッダ解析待ち
#define NyLPC_TcHttpBasicBodyParser_ST_CHUNK_HEADER_SP      ((NyLPC_TcHttpBasicBodyParser_ST)0x12)//ヘッダ解析中
#define NyLPC_TcHttpBasicBodyParser_ST_CHUNK_HEADER_EXT     ((NyLPC_TcHttpBasicBodyParser_ST)0x13)//ヘッダ解析中
#define NyLPC_TcHttpBasicBodyParser_ST_CHUNK_BODY           ((NyLPC_TcHttpBasicBodyParser_ST)0x14)//フッタ解析中
#define NyLPC_TcHttpBasicBodyParser_ST_CHUNK_FOOTER         ((NyLPC_TcHttpBasicBodyParser_ST)0x15)//フッタ解析中
#define NyLPC_TcHttpBasicBodyParser_ST_CHUNK_END                ((NyLPC_TcHttpBasicBodyParser_ST)0x16)//フッタ解析中
#define NyLPC_TcHttpBasicBodyParser_ST_BODY                 ((NyLPC_TcHttpBasicBodyParser_ST)0x17)//BODYパース中
#define NyLPC_TcHttpBasicBodyParser_ST_EOB                  ((NyLPC_TcHttpBasicBodyParser_ST)0x7F)//BODY確定
#define NyLPC_TcHttpBasicBodyParser_ST_ERROR                    ((NyLPC_TcHttpBasicBodyParser_ST)0x80)//終わり
#define NyLPC_TcHttpBasicBodyParser_ST_NULL                 ((NyLPC_TcHttpBasicBodyParser_ST)0x00)//初期状態


#define NyLPC_TcHttpBasicBodyParser_ST_isError(i_v) ((i_v)==NyLPC_TcHttpBasicBodyParser_ST_ERROR)


/**
 * parserのHandler
 */
typedef NyLPC_TBool (*NyLPC_TcHttpBasicBodyParser_bodyHandler) (NyLPC_TcHttpBasicBodyParser_t* i_inst,NyLPC_TChar i_c);

struct NyLPC_TcHttpBasicBodyParser_Handler
{
    NyLPC_TcHttpBasicBodyParser_bodyHandler bodyHandler;
};




/**
 * クラス構造体
 */
struct NyLPC_TcHttpBasicBodyParser
{
    NyLPC_THttpMessgeHeader_TransferEncoding _encode_type;
    NyLPC_TcHttpBasicBodyParser_ST _status;
    union{
        struct{
            int recv_len;
        }chunked;
        struct{
            NyLPC_TUInt32 content_length;
        }normal;
    }_data;
    struct NyLPC_TcHttpBasicBodyParser_Handler* _handler;
};


void NyLPC_cHttpBasicBodyParser_initialize(NyLPC_TcHttpBasicBodyParser_t* i_inst,struct NyLPC_TcHttpBasicBodyParser_Handler* i_handler);
#define NyLPC_cHttpBasicBodyParser_finalize(i_inst)
#define NyLPC_cHttpBasicBodyParser_getState(i_v) ((i_v)->_status)

/**
 * パーサの開始処理をします。
 * 関数は、parseInit->parseChar[n回]->(parseStream)->parseFinishの順でコールします。
 * parseChar、又はparseStreamでエラーが発生した場合は、後続の関数を呼び出すことは出来ません。
 * parseCharでEOHに達した場合、parseCharまたはparseStreamを続けて呼ぶことは出来ません。
 * parseFinishはparseCharまたはparseStreamでEOHに達した場合のみ呼び出すことが出来ます。
 */
void NyLPC_cHttpBasicBodyParser_parseInit(NyLPC_TcHttpBasicBodyParser_t* i_instt,const struct NyLPC_THttpBasicHeader* i_info);

/**
 * パーサの処理を閉じます。
 * @return
 * パース処理が正常に終了したかの真偽値
 */
NyLPC_TBool NyLPC_cHttpBasicBodyParser_parseFinish(NyLPC_TcHttpBasicBodyParser_t* i_inst);

/**
 * HTTPストリームをパースします。
 * @return
 */
NyLPC_TInt32 NyLPC_cHttpBasicBodyParser_parseChar(NyLPC_TcHttpBasicBodyParser_t* i_inst,const NyLPC_TChar* i_c,NyLPC_TInt32 i_size);


/**
 * ストリームから読み出して、EOHに達するまでパースします。
 * コール前にNyLPC_cHttpBasicHeaderParser_parseInitでパーサを開始してください。
 * @return
 * 処理が正常に終了したかを返します。
 * TRUEの場合、ステータスはEOHに達しています。(parseFinishをコールできます。)
 */
//NyLPC_TcHttpBasicBodyParser_ST NyLPC_cHttpBasicHeaderParser_parseStream(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TcHttpStream_t* i_stream,NyLPC_TChar* i_buf,NyLPC_TUInt16 i_buf_size,NyLPC_TUInt16 i_out_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
