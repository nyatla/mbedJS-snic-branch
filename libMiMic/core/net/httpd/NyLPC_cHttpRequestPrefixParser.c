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
#include "NyLPC_cHttpRequestPrefixParser.h"




struct THttpHeaderPrefix{
    struct NyLPC_THttpBasicHeader super;
    /** URL蓄積用のオブジェクト*/
    NyLPC_TcStr_t surl;
    /** コールバック関数の成功/失敗を判定するフラグ*/
    NyLPC_TBool status;
};



const NyLPC_TChar* NyLPC_cHttpRequestPrefixParser_getUrlPrefix(const NyLPC_TcHttpRequestPrefixParser_t* i_inst)
{
    return i_inst->_url;
}
/**
 * URLハンドラ。
 * 規定文字数のPathを入力されるか、Pathが終了するまで文字列を蓄積する。
 */
static NyLPC_TBool urlHandler(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
    struct THttpHeaderPrefix* s=(struct THttpHeaderPrefix*)o_out;

    //終端なら終わらせる。
    if(i_c=='\0'){
        s->status=NyLPC_TBool_TRUE;
        return NyLPC_TBool_FALSE;
    }
    //容量がいっぱい->解析を終わらせる。
    if(NyLPC_cStr_capacity(&(s->surl))<1)
    {
        s->status=NyLPC_TBool_TRUE;
        return NyLPC_TBool_FALSE;
    }
    NyLPC_cStr_put(&(s->surl),i_c);
    return NyLPC_TBool_TRUE;
}
/**
 * デフォルトハンドラ
 */
static const struct NyLPC_TcHttpBasicHeaderParser_Handler _handler=
{
    NULL,
    urlHandler
};
void NyLPC_cHttpRequestPrefixParser_initialize(NyLPC_TcHttpRequestPrefixParser_t* i_inst)
{
    return;
}

NyLPC_TBool NyLPC_cHttpRequestPrefixParser_parse(NyLPC_TcHttpRequestPrefixParser_t* i_inst,NyLPC_TiHttpPtrStream_t* i_stream)
{
    struct THttpHeaderPrefix hout;
    NyLPC_TcHttpBasicHeaderParser_t parser;
    NyLPC_cStr_initialize(&hout.surl,i_inst->_url,NyLPC_TcHttpRequestPrefixParser_MAX_URL_LEN);
    hout.status=NyLPC_TBool_FALSE;
    NyLPC_cHttpBasicHeaderParser_initialize(&parser,&_handler);
    NyLPC_cHttpBasicHeaderParser_parseInit(&parser,&hout.super);
    NyLPC_cHttpBasicHeaderParser_parseStream(&parser,i_stream,&hout.super);//どの道エラー
    NyLPC_cHttpBasicHeaderParser_parseFinish(&parser,&hout.super);//どの道エラー
    if(!hout.status){
        NyLPC_OnErrorGoto(Error);
    }
    //Errorで帰ってくるのでparsefinishは不要
    //NyLPC_cHttpBasicHeaderParser_parseFinish(&parser,&hout.super);
    i_inst->method=hout.super.startline.req.method;
    NyLPC_cHttpBasicHeaderParser_finalize(&parser);
    //この時点では、メソッドとURLの一部がパースされているはず。
    NyLPC_cStr_finalize(&hout.surl);
    //フラグをチェックして返す。
    return NyLPC_TBool_TRUE;
Error:
    NyLPC_cHttpBasicHeaderParser_finalize(&parser);
    NyLPC_cStr_finalize(&hout.surl);
    return NyLPC_TBool_FALSE;
}






