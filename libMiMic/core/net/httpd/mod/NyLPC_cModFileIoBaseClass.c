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
#include "NyLPC_cModFileIoBaseClass.h"
#include "NyLPC_stdlib.h"
#include "NyLPC_http.h"
#include "../NyLPC_cHttpdConnection_protected.h"
#include "../NyLPC_cHttpdUtils.h"
#include "NyLPC_net.h"

#define FNAME_MAX   48
#define STRBUF_MAX  48

struct TModFileIoHeader
{
    struct NyLPC_THttpBasicHeader super;
    NyLPC_TcUrlEncode_t urlencode;
    NyLPC_TUInt8 _content_id;
    //解析用
    NyLPC_TUInt8 _qery_name_id;
    NyLPC_TUInt8 _astate;
    NyLPC_TInt16 _prefix_len;
    NyLPC_TcStr_t _tstr;
    NyLPC_TChar _tstr_buf[STRBUF_MAX];
    /** 文字列のパーサ*/
    char fname[FNAME_MAX];//対象ファイル名の格納先
};

#define ST_PARSE_PATH 1
#define ST_PARSE_QUERY_NAME 2
#define ST_PARSE_QUERY_VALUE 3      //Query読み出し中
#define ST_PARSE_QUERY_VALUE_NAME 4
/**
 * コンテンツID定義(コンテンツ名に対応)
 */
#define CONTENT_ID_UPLOAD  2
#define CONTENT_ID_CREATE  3
#define CONTENT_ID_REMOVE  4
#define CONTENT_ID_UNKNOWN 0


#define QNAME_ID_NAME    1
#define QNAME_ID_UNKNOWN 0


static const struct NyLPC_TTextIdTbl url_tbl[]=
{
    {"upload.api",CONTENT_ID_UPLOAD},
    {"create.api",CONTENT_ID_CREATE},
    {"remove.api",CONTENT_ID_REMOVE},
    {NULL,CONTENT_ID_UNKNOWN}
};

static const struct NyLPC_TTextIdTbl qname_id_table[]=
{
    {"name",QNAME_ID_NAME},
    {NULL,QNAME_ID_UNKNOWN}
};



static NyLPC_TBool urlHandler(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
    struct TModFileIoHeader* out=(struct TModFileIoHeader*)o_out;
    NyLPC_TChar c;
    //読み飛ばし
    if(out->_prefix_len<0){
        out->_prefix_len++;
        return NyLPC_TBool_TRUE;//読み飛ばし
    }
    //Path解析
    if(out->_astate==ST_PARSE_PATH){
        if(i_c!='\0' && i_c!='?'){
            if(!NyLPC_cStr_put(&(out->_tstr),i_c)){
                //ERROR
                NyLPC_OnErrorGoto(ERROR);
            }
        }else{
            out->_content_id=NyLPC_TTextIdTbl_getMatchId(NyLPC_cStr_str(&(out->_tstr)),url_tbl);
            switch(out->_content_id)
            {
            case CONTENT_ID_UPLOAD:
            case CONTENT_ID_CREATE:
            case CONTENT_ID_REMOVE:
                break;
            default:
                NyLPC_OnErrorGoto(ERROR);
            }
            NyLPC_cStr_clear(&(out->_tstr));
            out->_astate=ST_PARSE_QUERY_NAME;//クエリ名解析へ
        }
        return NyLPC_TBool_TRUE;
    }
    switch(out->_content_id)
    {
    case CONTENT_ID_UPLOAD:
    case CONTENT_ID_CREATE:
    case CONTENT_ID_REMOVE:
        switch(out->_astate){
        case ST_PARSE_QUERY_NAME:
            if(i_c!='\0' && i_c!='&' && i_c!='='){
                if(!NyLPC_cStr_put(&(out->_tstr),i_c)){
                    NyLPC_OnErrorGoto(ERROR);
                }
            }else{
                //Query確定。
                out->_qery_name_id=NyLPC_TTextIdTbl_getMatchId(NyLPC_cStr_str(&(out->_tstr)),qname_id_table);
                NyLPC_cStr_clear(&(out->_tstr));
                //クエリ値がある場合
                switch(out->_qery_name_id){
                case QNAME_ID_NAME:
                    NyLPC_cUrlEncode_reset(&out->urlencode);
                    out->_astate=ST_PARSE_QUERY_VALUE_NAME;
                    break;
                default:
                    out->_astate=ST_PARSE_QUERY_VALUE;
                    break;
                }
            }
            return NyLPC_TBool_TRUE;
        case ST_PARSE_QUERY_VALUE:
            //未知のクエリは無視
            if(i_c!='\0' && i_c!='&'){
            }else{
                //クエリ値解析完了
                out->_astate=ST_PARSE_QUERY_NAME;
            }
            return NyLPC_TBool_TRUE;
        case ST_PARSE_QUERY_VALUE_NAME:
            if(i_c!='\0' && i_c!='&'){
                //URLデコードしながら蓄積
                switch(NyLPC_cUrlEncode_decode(&out->urlencode,i_c,&c)){
                case NyLPC_TcUrlEncode_ST_NEXT:
                    break;
                case NyLPC_TcUrlEncode_ST_DONE:
                    if(!NyLPC_cStr_put(&(out->_tstr),c)){
                        NyLPC_OnErrorGoto(ERROR);
                    }
                    break;
                default:
                    NyLPC_OnErrorGoto(ERROR);
                }
                return NyLPC_TBool_TRUE;
            }else{
                if(NyLPC_cStr_len(&out->_tstr)<1){
                    //ファイル名短すぎ
                    NyLPC_OnErrorGoto(ERROR);
                }
                //ファイル名を保存
                strcpy(out->fname,(const char*)NyLPC_cStr_str(&out->_tstr));
                //終端しているなら、次のクエリへ
                out->_astate=ST_PARSE_QUERY_NAME;
            }
            return NyLPC_TBool_TRUE;
        default:
            break;
        }
        NyLPC_OnErrorGoto(ERROR);
    default:
        NyLPC_OnErrorGoto(ERROR);
    }
    return NyLPC_TBool_TRUE;
ERROR:
    return NyLPC_TBool_FALSE;
}
/**
 * デフォルトハンドラ
 */
static const struct NyLPC_TcHttpBasicHeaderParser_Handler handler=
{
    NULL,
    urlHandler
};


/**
 * コンストラクタ。
 */
void NyLPC_cModFileIoBaseClass_initialize(NyLPC_TcModFileIoBaseClass_t* i_inst,const NyLPC_TChar* i_ref_root_path)
{
    NyLPC_cModRomFiles_initialize(&i_inst->super,i_ref_root_path,NULL,0);
}
void NyLPC_cModFileIoBaseClass_finalize(NyLPC_TcModFileIoBaseClass_t* i_inst)
{
    NyLPC_cModRomFiles_finalize(&i_inst->super);
}
/**
 * モジュールがコネクションをハンドリングできるかを返します。
 */
NyLPC_TBool NyLPC_cModFileIoBaseClass_canHandle(NyLPC_TcModFileIoBaseClass_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection)
{
    return NyLPC_cModRomFiles_canHandle(&i_inst->super,i_connection);
}

static struct TModFileIoHeader single_header;


/**
 * モジュールを実行します。
 */
NyLPC_TBool NyLPC_cModFileIoBaseClass_execute(NyLPC_TcModFileIoBaseClass_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection)
{
    union{
        NyLPC_TcHttpBasicHeaderParser_t parser;
        NyLPC_TcHttpBodyParser_t body_parser;
    }sh;
    NyLPC_TUInt8 method_type;
    //リクエストParse済へ遷移(この関数の後はModが責任を持ってリクエストを返却)
    NyLPC_cHttpdConnection_setReqStatusParsed(i_connection);

    //排他ロック
    NyLPC_cHttpdConnection_lock(i_connection);
    {//parser

        //URL解析の準備
        single_header._prefix_len=-((NyLPC_TInt16)strlen(i_inst->super._ref_root_path)+2);
        single_header._astate=ST_PARSE_PATH;
        single_header.fname[0]='\0';
        NyLPC_cUrlEncode_initialize(&single_header.urlencode);
        NyLPC_cStr_initialize(&single_header._tstr,single_header._tstr_buf,STRBUF_MAX);

        NyLPC_cHttpBasicHeaderParser_initialize(&sh.parser,&handler);

        //プリフェッチしたデータを流す
        NyLPC_cHttpBasicHeaderParser_parseInit(&sh.parser,&(single_header.super));
        NyLPC_cHttpdConnection_pushPrefetchInfo(i_connection,&sh.parser,&single_header.super);
        //後続をストリームから取り込む
        if(!NyLPC_cHttpBasicHeaderParser_parseStream(&sh.parser,NyLPC_cHttpdConnection_refStream(i_connection),&(single_header.super))){
            NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
            NyLPC_OnErrorGoto(Error1);
        }
        if(!NyLPC_cHttpBasicHeaderParser_parseFinish(&sh.parser,&(single_header.super))){
            NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
            NyLPC_OnErrorGoto(Error1);
        }
        //HeaderParserはここで破棄(URLEncode,cSTRも)
        NyLPC_cHttpBasicHeaderParser_finalize(&sh.parser);
        NyLPC_cUrlEncode_finalize(&single_header.urlencode);
        NyLPC_cStr_finalize(&single_header._tstr);
    }
    //Request::ConnectionがClose設定,又はHTTP1.1では無い場合,CLOSE
    if(single_header.super.connection==NyLPC_THttpMessgeHeader_Connection_CLOSE || single_header.super.startline.req.version!=NyLPC_THttpVersion_11)
    {
        NyLPC_cHttpdConnection_setConnectionMode(i_connection,NyLPC_TcHttpdConnection_CONNECTION_MODE_CLOSE);
    }
    //返答フェーズ
    {
        method_type=NyLPC_cHttpdConnection_getMethod(i_connection);

        //CGIの実行
        switch(single_header._content_id)
        {
        case CONTENT_ID_UPLOAD:
            //ファイル名とBodyParserを通知
            if(method_type==NyLPC_THttpMethodType_POST)
            {
                NyLPC_cHttpdConnection_send100Continue(i_connection);
                NyLPC_cHttpBodyParser_initialize(&sh.body_parser);
                NyLPC_cHttpBodyParser_parseInit(&sh.body_parser,&single_header.super);
                //ハンドラ内ではparseStreamのみ実行
                if(!i_inst->_abstruct_function.upload(i_connection,single_header.fname,&sh.body_parser)){
                    NyLPC_cHttpdConnection_setConnectionMode(i_connection,NyLPC_TcHttpdConnection_CONNECTION_MODE_CLOSE);
                }
                if(NyLPC_cHttpBodyParser_parseFinish(&sh.body_parser)){
                    NyLPC_cHttpBodyParser_finalize(&sh.body_parser);
                    NyLPC_OnErrorGoto(Error2);//エラーメッセージはハンドラ内で送られていると仮定する。
                }
                NyLPC_cHttpBodyParser_finalize(&sh.body_parser);
            }else{
                NyLPC_OnErrorGoto(Error2);
            }
            break;
        case CONTENT_ID_CREATE:
            if(method_type==NyLPC_THttpMethodType_GET || method_type==NyLPC_THttpMethodType_HEAD)
            {
                //イベント起動
                if(!i_inst->_abstruct_function.create(i_connection,single_header.fname)){
                    NyLPC_cHttpdConnection_setConnectionMode(i_connection,NyLPC_TcHttpdConnection_CONNECTION_MODE_CLOSE);
                }
                break;
            }
            NyLPC_OnErrorGoto(Error2_405);
        case CONTENT_ID_REMOVE:
            //ファイル名を通知
            if(method_type==NyLPC_THttpMethodType_GET || method_type==NyLPC_THttpMethodType_HEAD)
            {
                //イベント起動
                if(!i_inst->_abstruct_function.remove(i_connection,single_header.fname)){
                    NyLPC_cHttpdConnection_setConnectionMode(i_connection,NyLPC_TcHttpdConnection_CONNECTION_MODE_CLOSE);
                }
                break;
            }
            NyLPC_OnErrorGoto(Error2_405);
        default:
            NyLPC_cHttpdUtils_sendErrorResponse(i_connection,400);
            NyLPC_OnErrorGoto(Error2);
        }
    }
//占有解除
    NyLPC_cHttpdConnection_unlock(i_connection);
    return NyLPC_TBool_TRUE;
Error2_405:
    NyLPC_cHttpdUtils_sendErrorResponse(i_connection,405);
Error2:
    //VM排他ロックの解除
    NyLPC_cHttpdConnection_unlock(i_connection);
    return NyLPC_TBool_FALSE;
Error1:
    NyLPC_cHttpBasicHeaderParser_finalize(&parser);
    NyLPC_cStr_finalize(&single_header._tstr);
    NyLPC_cUrlEncode_finalize(&single_header.urlencode);
    //VM排他ロックの解除
    NyLPC_cHttpdConnection_unlock(i_connection);
    return NyLPC_TBool_FALSE;
}


