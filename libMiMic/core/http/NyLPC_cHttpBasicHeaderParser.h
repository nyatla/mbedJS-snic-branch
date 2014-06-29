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

#ifndef NyLPC_TcHttpBasicHeaderParser_H
#define NyLPC_TcHttpBasicHeaderParser_H
#include "NyLPC_stdlib.h"
#include "NyLPC_cHttpStream.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * HeaderParserのステータス定義値。
 */
typedef NyLPC_TUInt8 NyLPC_TcHttpBasicHeaderParser_ST;
#define NyLPC_TcHttpBasicHeaderParser_ST_START                  ((NyLPC_TcHttpBasicHeaderParser_ST)0x01)//HTTPステータスラインか、リクエストライン
#define NyLPC_TcHttpBasicHeaderParser_ST_RL_URL                 ((NyLPC_TcHttpBasicHeaderParser_ST)0x12)//URL
#define NyLPC_TcHttpBasicHeaderParser_ST_RL_VERSION             ((NyLPC_TcHttpBasicHeaderParser_ST)0x13)//VERSION
#define NyLPC_TcHttpBasicHeaderParser_ST_SL_STATUSCODE          ((NyLPC_TcHttpBasicHeaderParser_ST)0x21)//ステータスコード
#define NyLPC_TcHttpBasicHeaderParser_ST_SL_REASON              ((NyLPC_TcHttpBasicHeaderParser_ST)0x22)//Reason-Phrase
#define NyLPC_TcHttpBasicHeaderParser_ST_MSGHEAD                ((NyLPC_TcHttpBasicHeaderParser_ST)0x31)//MESSAGE
#define NyLPC_TcHttpBasicHeaderParser_ST_MSGPARAM               ((NyLPC_TcHttpBasicHeaderParser_ST)0x32)//MESSAGEパラメータ部
#define NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONTENTLENGTH      ((NyLPC_TcHttpBasicHeaderParser_ST)0x33)//MESSAGEContentLength
#define NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONNECTION         ((NyLPC_TcHttpBasicHeaderParser_ST)0x34)//MESSAGEConnection
#define NyLPC_TcHttpBasicHeaderParser_ST_MSG_TRANSFERENCODING   ((NyLPC_TcHttpBasicHeaderParser_ST)0x35)//TransferEncoding
#define NyLPC_TcHttpBasicHeaderParser_ST_MSG_RANGE              ((NyLPC_TcHttpBasicHeaderParser_ST)0x36)//Range
#define NyLPC_TcHttpBasicHeaderParser_ST_EOH                    ((NyLPC_TcHttpBasicHeaderParser_ST)0x7F)//終わり
#define NyLPC_TcHttpBasicHeaderParser_ST_ERROR                  ((NyLPC_TcHttpBasicHeaderParser_ST)0x80)//終わり
/*
    プライベート関数
*/
#define NyLPC_TcHttpBasicHeaderParser_ST_isError(i_v) ((i_v)==NyLPC_TcHttpBasicHeaderParser_ST_ERROR)



/**
 * このクラスは、Httpヘッダのパーサを定義します。
 */
typedef struct NyLPC_TcHttpBasicHeaderParser NyLPC_TcHttpBasicHeaderParser_t;


/**
 * HTTPメソッドの定義値。
 */
typedef NyLPC_TUInt8 NyLPC_THttpMethodType;
//HTTP STANDARD
#define NyLPC_THttpMethodType_NULL          ((NyLPC_THttpMethodType)0x00)
#define NyLPC_THttpMethodType_GET           ((NyLPC_THttpMethodType)0x01)
#define NyLPC_THttpMethodType_POST          ((NyLPC_THttpMethodType)0x02)
#define NyLPC_THttpMethodType_HEAD          ((NyLPC_THttpMethodType)0x03)
//SSDP
#define NyLPC_THttpMethodType_M_SEARCH      ((NyLPC_THttpMethodType)0x11)
#define NyLPC_THttpMethodType_NOTIFY        ((NyLPC_THttpMethodType)0x12)

const char* NyLPC_THttpMethodType_toString(NyLPC_THttpMethodType i_method);

/**
 * HTTPバージョンの定義値
 */
typedef NyLPC_TUInt8 NyLPC_THttpVersion;
#define NyLPC_THttpVersion_09       ((NyLPC_THttpVersion)0x01)
#define NyLPC_THttpVersion_10       ((NyLPC_THttpVersion)0x02)
#define NyLPC_THttpVersion_11       ((NyLPC_THttpVersion)0x03)
#define NyLPC_THttpVersion_UNKNOWN  ((NyLPC_THttpVersion)0x04)


typedef NyLPC_TUInt8 NyLPC_THttpHeaderType;
#define NyLPC_THttpHeaderType_REQUEST  ((NyLPC_THttpHeaderType)0x01)
#define NyLPC_THttpHeaderType_RESPONSE ((NyLPC_THttpHeaderType)0x02)


typedef NyLPC_TUInt8 NyLPC_THttpMessgeHeader_Connection;
#define NyLPC_THttpMessgeHeader_Connection_NONE  ((NyLPC_THttpMessgeHeader_Connection)0x01)
#define NyLPC_THttpMessgeHeader_Connection_CLOSE ((NyLPC_THttpMessgeHeader_Connection)0x02)
#define NyLPC_THttpMessgeHeader_Connection_KEEPALIVE ((NyLPC_THttpMessgeHeader_Connection)0x03)
#define NyLPC_THttpMessgeHeader_Connection_UPGRADE ((NyLPC_THttpMessgeHeader_Connection)0x04)
#define NyLPC_THttpMessgeHeader_Connection_UNKNOWN ((NyLPC_THttpMessgeHeader_Connection)0x10)

typedef NyLPC_TUInt8 NyLPC_THttpMessgeHeader_TransferEncoding;
#define NyLPC_THttpMessgeHeader_TransferEncoding_NONE    ((NyLPC_THttpMessgeHeader_TransferEncoding)0x01)   //TEはない
#define NyLPC_THttpMessgeHeader_TransferEncoding_CHUNKED ((NyLPC_THttpMessgeHeader_TransferEncoding)0x02)
#define NyLPC_THttpMessgeHeader_TransferEncoding_UNKNOWN ((NyLPC_THttpMessgeHeader_TransferEncoding)0x10)

#define NyLPC_THttpContentLength_INVALID_LENGTH 0xFFFFFFFF





/**
 * この構造体は、NyLPC_cHttpBasicHeaderParserの結果を格納します。
 */
struct NyLPC_THttpBasicHeader
{
    NyLPC_THttpMessgeHeader_TransferEncoding transfer_encoding;
    NyLPC_THttpMessgeHeader_Connection connection;
    NyLPC_THttpHeaderType type;
    NyLPC_TUInt8 _padding;
    union{
        struct{
            NyLPC_THttpVersion version;
            NyLPC_THttpMethodType method;
        }req;
        struct{
            NyLPC_THttpVersion version;
            NyLPC_TUInt32 status;
        }res;
    }startline;
    NyLPC_TUInt32 content_length;
};

/**
 * このヘッダが持続性接続を求めているか判定します。
 */
NyLPC_TBool NyLPC_THttpBasicHeader_isPersistent(const struct NyLPC_THttpBasicHeader* i_struct);






/**
 * コンフィギュレーション値。
 * ショートパラメータ解析バッファのサイズ
 */
#define NyLPC_cHttpBasicHeaderParser_SIZE_OF_WBS 32




/**
 * 独自のメッセージフィールドを受け取るイベントハンドラです。
 * メッセージハンドラはNyLPC_cHttpBasicHeaderParserから３種類のメッセージを受け取ります。
 * ハンドラは、NyLPC_TcHttpBasicHeaderParserが処理しないメッセージヘッダをパース中に、次の順番で呼び出されます。
 * <ol>
 * <li>開始メッセージ - i_nameに有効な文字列を指定して、そのフィールド値が入力される事を伝えます。i_cはnullです。
 * <li>フィールドメッセージ - i_nameにNULL,i_cに\0以外の文字を指定して、フィールド値が入力されている事を指示します。
 * <li>フィールドエンド - i_nameにNULL,i_cに0を指定して、フィールドが完了した事を指示します。
 * </ol>
*/
typedef NyLPC_TBool (*NyLPC_cHttpBasicHeaderParser_messageHandler) (NyLPC_TcHttpBasicHeaderParser_t* i_inst,const NyLPC_TChar* i_name,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out);
/**
 * リクエストのURL文字列を受け取るイベントハンドラです。
 * メッセージハンドラはNyLPC_cHttpBasicHeaderParserから３種類のメッセージを受け取ります。
 * ハンドラは、NyLPC_TcHttpBasicHeaderParserが処理しないメッセージヘッダをパース中に、次の順番で呼び出されます。
 * <ol>
 * <li>URLメッセージ - i_cに\0以外の文字を指定して、URL値が入力されている事を指示します。
 * <li>URLエンド - i_cに0を指定して、URLが完了した事を指示します。
 * </ol>
*/
typedef NyLPC_TBool (*NyLPC_cHttpBasicHeaderParser_urlHandler) (NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out);



/**
 * HTTPヘッダパーサハンドラの集合です。
 * ハンドラにNULL指定の場合TRUEを返したと見なします。
 */
struct NyLPC_TcHttpBasicHeaderParser_Handler
{
    NyLPC_cHttpBasicHeaderParser_messageHandler messageHandler;
    NyLPC_cHttpBasicHeaderParser_urlHandler urlHandler;
};



/**
 * クラス構造体
 */
struct NyLPC_TcHttpBasicHeaderParser
{
    /**継承クラスで実装すべきインタフェイス*/
    const struct NyLPC_TcHttpBasicHeaderParser_Handler* _handler;
    NyLPC_TUInt16 _rcode;//_stがERRORの時にエラーコードを格納する。
    /** パースの実行状態*/
    NyLPC_TcHttpBasicHeaderParser_ST _st;
    /**ワーク文字列*/
    NyLPC_TcStr_t _wsb;
    /**ワーク文字列のバッファ*/
    char _wsb_buf[NyLPC_cHttpBasicHeaderParser_SIZE_OF_WBS];
};


void NyLPC_cHttpBasicHeaderParser_initialize(NyLPC_TcHttpBasicHeaderParser_t* i_inst,const struct NyLPC_TcHttpBasicHeaderParser_Handler* i_handler);
#define NyLPC_cHttpBasicHeaderParser_finalize(i_inst)

/**
 * パーサの開始処理をします。
 * 関数は、parseInit->parseChar[n回]->(parseStream)->parseFinishの順でコールします。
 * parseChar、又はparseStreamでエラーが発生した場合は、後続の関数を呼び出すことは出来ません。
 * parseCharでEOHに達した場合、parseCharまたはparseStreamを続けて呼ぶことは出来ません。
 * parseFinishはparseCharまたはparseStreamでEOHに達した場合のみ呼び出すことが出来ます。
 */
void NyLPC_cHttpBasicHeaderParser_parseInit(NyLPC_TcHttpBasicHeaderParser_t* i_inst,struct NyLPC_THttpBasicHeader* o_out);

/**
 * パーサの処理を閉じます。
 * @return
 * パース処理が正常に終了したかの真偽値
 */
NyLPC_TBool NyLPC_cHttpBasicHeaderParser_parseFinish(NyLPC_TcHttpBasicHeaderParser_t* i_inst,struct NyLPC_THttpBasicHeader* o_out);

/**
 * 文字列をパースします。
 * コール前にNyLPC_cHttpBasicHeaderParser_parseInitでパーサを開始してください。
 * @return
 * パースした文字数。エラーの場合-1です。
 * 0以上の場合、getParseStatusでパーサの状態を確認してください。
 */
NyLPC_TInt32 NyLPC_cHttpBasicHeaderParser_parseChar(NyLPC_TcHttpBasicHeaderParser_t* i_inst,const NyLPC_TChar* i_c,NyLPC_TInt32 i_size,struct NyLPC_THttpBasicHeader* o_out);


/**
 * ストリームから読み出して、EOHに達するまでパースします。
 * コール前にNyLPC_cHttpBasicHeaderParser_parseInitでパーサを開始してください。
 * @return
 * 処理が正常に終了したかを返します。
 * TRUEの場合、ステータスはEOHに達しています。(parseFinishをコールできます。)
 */
NyLPC_TBool NyLPC_cHttpBasicHeaderParser_parseStream(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TiHttpPtrStream_t* i_stream,struct NyLPC_THttpBasicHeader* o_out);


/**
 * parse関数がエラーの場合に、
 * 候補のエラーのステータスコードを返す。
 */
#define NyLPC_cHttpBasicHeaderParser_getStatusCode(inst) ((inst)->_rcode)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
