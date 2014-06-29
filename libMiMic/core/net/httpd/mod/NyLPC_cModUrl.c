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
#include "NyLPC_cModUrl.h"
#include "NyLPC_stdlib.h"
#include "NyLPC_http.h"
#include "../NyLPC_cHttpdUtils.h"
#include "../NyLPC_cHttpdConnection_protected.h"

typedef struct TcHeaderParser
{
    NyLPC_TcHttpBasicHeaderParser_t super;
    char* url_buf;
    NyLPC_TInt16 length_of_buf;
    NyLPC_TInt16 length_of_url;
    NyLPC_TInt16 reason;
    NyLPC_TUInt8 skip;
    NyLPC_TUInt8 mode;
}TcHeaderParser_t;



static NyLPC_TBool NyLPC_cModUrl_urlHandler(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
    TcHeaderParser_t* inst=(TcHeaderParser_t*)i_inst;
    if(inst->skip){
        //SKIPが有効な場合
        if(inst->length_of_url<0){
            inst->length_of_url++;
            if(inst->length_of_url==0){
                inst->skip=NyLPC_TUInt8_FALSE;
            }
        }
        return NyLPC_TBool_TRUE;
    }
    if((inst->mode & NyLPC_cModUrl_ParseMode_PATH_ONLY)==NyLPC_cModUrl_ParseMode_PATH_ONLY){
        if(strchr("?#",i_c)){
            inst->url_buf[inst->length_of_url]='\0';
            inst->skip=NyLPC_TUInt8_TRUE;
            return NyLPC_TBool_TRUE;//Terminate
        }
    }
    inst->url_buf[inst->length_of_url]=i_c;
    if(i_c=='\0'){
        return NyLPC_TBool_TRUE;//Terminate
    }
    inst->length_of_url++;
    if(inst->length_of_url==inst->length_of_buf){
        inst->reason=414;
        return NyLPC_TBool_FALSE;//長すぎる。
    }

    return NyLPC_TBool_TRUE;
}
/**
 * デフォルトハンドラ
 */
static const struct NyLPC_TcHttpBasicHeaderParser_Handler _handler=
{
    NULL,
    NyLPC_cModUrl_urlHandler
};



void NyLPC_cModUrl_initialize(NyLPC_TcModUrl_t* i_inst)
{
    NyLPC_cHttpBodyParser_initialize(&i_inst->_body_parser);
}
void NyLPC_cModUrl_finalize(NyLPC_TcModUrl_t* i_inst)
{
    NyLPC_cHttpBodyParser_finalize(&i_inst->_body_parser);
}

const struct NyLPC_THttpBasicHeader* NyLPC_cModUrl_getHeader(const NyLPC_TcModUrl_t* i_inst)
{
    return &(i_inst->_header);
}

/**
 * Methodタイプを返します。
 */
NyLPC_THttpMethodType NyLPC_cModUrl_getMethod(const NyLPC_TcModUrl_t* i_inst)
{
    return i_inst->_header.startline.req.method;
}



NyLPC_TBool NyLPC_cModUrl_execute2(NyLPC_TcModUrl_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection,char* o_url_buf,NyLPC_TInt16 i_length_buf,NyLPC_TInt16 i_pass_prefix_len,NyLPC_cModUrl_ParseMode i_mode)
{
    TcHeaderParser_t parser;
    NyLPC_Assert(i_length_buf>0);
    NyLPC_Assert(i_pass_prefix_len>=0);
    //コネクションのステータスチェック
    if(!NyLPC_cHttpdConnection_getReqStatus(i_connection)==NyLPC_cHttpdConnection_ReqStatus_REQPARSE)
    {
        NyLPC_OnErrorGoto(Error1);
    }
    //リクエストParse済へ遷移
    NyLPC_cHttpdConnection_setReqStatusParsed(i_connection);

    NyLPC_cHttpBasicHeaderParser_initialize(&parser.super,&_handler);
    parser.length_of_buf=i_length_buf;
    parser.length_of_url=-i_pass_prefix_len;//無視するPrefix長
    parser.skip=(parser.length_of_url<0)?NyLPC_TUInt8_TRUE:NyLPC_TUInt8_FALSE;//スキップの初期値の設定
    parser.url_buf=o_url_buf;
    parser.url_buf[0]='\0';//URL長<=prefix長に備えてNULLターミネイト
    parser.reason=400;
    parser.mode=i_mode;
    //プリフェッチしたデータを流す
    NyLPC_cHttpBasicHeaderParser_parseInit(&parser.super,&(i_inst->_header));
    NyLPC_cHttpdConnection_pushPrefetchInfo(i_connection,&parser.super,&(i_inst->_header));
    //後続をストリームから取り込む
    if(!NyLPC_cHttpBasicHeaderParser_parseStream(&parser.super,NyLPC_cHttpdConnection_refStream(i_connection),&(i_inst->_header))){
        NyLPC_OnErrorGoto(Error2);
    }
    if(!NyLPC_cHttpBasicHeaderParser_parseFinish(&parser.super,&(i_inst->_header))){
        NyLPC_OnErrorGoto(Error2);
    }
    //@todo http/1.1 && POSTの場合はcontinueを送る。
    if(i_inst->_header.startline.req.version==NyLPC_THttpVersion_11){
        if(i_inst->_header.startline.req.method==NyLPC_THttpMethodType_POST)
        {
            NyLPC_cHttpdConnection_send100Continue(i_connection);
        }
    }
    //Request::ConnectionがClose設定,又はHTTP1.1では無い場合,CLOSE
    if(i_inst->_header.connection==NyLPC_THttpMessgeHeader_Connection_CLOSE || i_inst->_header.startline.req.version!=NyLPC_THttpVersion_11)
    {
        NyLPC_cHttpdConnection_setConnectionMode(i_connection,NyLPC_TcHttpdConnection_CONNECTION_MODE_CLOSE);
    }

    NyLPC_cHttpBasicHeaderParser_finalize(&parser);
    //BodyParserの初期化
    if(NyLPC_cHttpBodyParser_getState(&i_inst->_body_parser)!=NyLPC_TcHttpBasicBodyParser_ST_NULL){
        NyLPC_cHttpBodyParser_parseFinish(&i_inst->_body_parser);
    }
    NyLPC_cHttpBodyParser_parseInit(&i_inst->_body_parser,&i_inst->_header);
    return NyLPC_TBool_TRUE;
Error2:
    //400Error
    NyLPC_cHttpdUtils_sendErrorResponse(i_connection,parser.reason);
    NyLPC_cHttpBasicHeaderParser_finalize(&parser);
Error1:
    return NyLPC_TBool_FALSE;
}





NyLPC_TInt16 NyLPC_cModUrl_readBody(NyLPC_TcModUrl_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection,void* i_buf,NyLPC_TInt16 i_buf_size)
{
    NyLPC_TInt16 l;
    //リクエストは解析済であること
    if(!NyLPC_cHttpdConnection_getReqStatus(i_connection)==NyLPC_cHttpdConnection_ReqStatus_END)
    {
        return -1;
    }
    if(!NyLPC_cHttpBodyParser_parseStream(&i_inst->_body_parser,NyLPC_cHttpdConnection_refStream(i_connection),i_buf,i_buf_size,&l)){
        return -1;
    }
    return l;
}

