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


#include <stdlib.h>
#include "NyLPC_utils.h"
#include "NyLPC_cHttpBodyWriter.h"






/**
 * PrintHandler
 */
static NyLPC_TBool printHandler(void* i_inst,const void* i_buf,NyLPC_TUInt32 i_len)
{
    //エラー状態ならFALSE
    if(((NyLPC_TcHttpBodyWriter_t*)i_inst)->_is_error){
        return NyLPC_TBool_FALSE;
    }
    ((NyLPC_TcHttpBodyWriter_t*)i_inst)->_size_of_sent+=i_len;
    if(!NyLPC_iHttpPtrStream_write(((NyLPC_TcHttpBodyWriter_t*)i_inst)->_ref_stream,i_buf,i_len)){
        ((NyLPC_TcHttpBodyWriter_t*)i_inst)->_is_error=NyLPC_TUInt8_TRUE;
        return NyLPC_TBool_FALSE;
    }
    return NyLPC_TBool_TRUE;
}



void NyLPC_cHttpBodyWriter_initialize(NyLPC_TcHttpBodyWriter_t* i_inst,NyLPC_TcHttpStream_t* i_stream)
{
    i_inst->_ref_stream=&(i_stream->super);
    i_inst->_is_chunked=NyLPC_TUInt8_FALSE;
    i_inst->_is_error=NyLPC_TUInt8_FALSE;
    i_inst->_size_of_sent=0;
    i_inst->_content_length=0;
    NyLPC_iHttpPtrStream_setWriteEncoding(i_inst->_ref_stream,NyLPC_TiHttpPtrStream_ET_NONE);
}

void NyLPC_cHttpBodyWriter_setChunked(NyLPC_TcHttpBodyWriter_t* i_inst)
{
    i_inst->_is_chunked=NyLPC_TUInt8_TRUE;
    NyLPC_iHttpPtrStream_setWriteEncoding(i_inst->_ref_stream,NyLPC_TiHttpPtrStream_ET_CHUNKED);
}
void NyLPC_cHttpBodyWriter_setContentLength(NyLPC_TcHttpBodyWriter_t* i_inst,NyLPC_TUInt32 i_content_length)
{
    i_inst->_is_chunked=NyLPC_TUInt8_FALSE;
    i_inst->_content_length=i_content_length;
    NyLPC_iHttpPtrStream_setWriteEncoding(i_inst->_ref_stream,NyLPC_TiHttpPtrStream_ET_NONE);
}


/**
 * HttpBodyを書き込みます。
 * @return
 * 偽を返した場合は、コネクションを切断してください。
 */
NyLPC_TBool NyLPC_cHttpBodyWriter_write(NyLPC_TcHttpBodyWriter_t* i_inst,const void* i_buf,NyLPC_TUInt32 i_len)
{
    return printHandler(i_inst,i_buf,i_len);
}

/**
 * HttpBodyの書き込みを完了します。
 * @return
 */
NyLPC_TBool NyLPC_cHttpBodyWriter_close(NyLPC_TcHttpBodyWriter_t* i_inst)
{
    //エラー状態ならFALSE
    if(i_inst->_is_error){
        return NyLPC_TBool_FALSE;
    }
    //chunkedの場合、フッタを書き込む
    if(i_inst->_is_chunked){
        //エンコーディングを戻す。
        NyLPC_iHttpPtrStream_setWriteEncoding(i_inst->_ref_stream,NyLPC_TiHttpPtrStream_ET_NONE);
        //フッタを書き込む。
        if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,"0\r\n\r\n",5)){
            i_inst->_is_error=NyLPC_TUInt8_TRUE;
            return NyLPC_TBool_FALSE;
        }
    }
    //エラーでないときはストリームをフラッシュ
    NyLPC_iHttpPtrStream_flush(i_inst->_ref_stream);
    //クローズのステータスで状態を変える。
    return NyLPC_TBool_TRUE;
}

/**
 * printfライクな書式出力を提供します。
 * @i_fmt
 * 書式文字列です。%d,%x,%s,%c,%%をサポートします。
 */
NyLPC_TBool NyLPC_cHttpBodyWriter_format(NyLPC_TcHttpBodyWriter_t* i_inst,const NyLPC_TChar* i_fmt,...)
{
    NyLPC_TBool ret;
    va_list a;
    //エラー状態ならFALSE
    if(i_inst->_is_error){
        return NyLPC_TBool_FALSE;
    }
    va_start(a,i_fmt);
    ret=   NyLPC_cFormatWriter_print(printHandler,i_inst,i_fmt,a);
    va_end(a);
    return ret;
}

NyLPC_TBool NyLPC_cHttpBodyWriter_formatV(NyLPC_TcHttpBodyWriter_t* i_inst,const NyLPC_TChar* i_fmt,va_list i_args)
{
    NyLPC_TBool ret;
    //エラー状態ならFALSE
    if(i_inst->_is_error){
        return NyLPC_TBool_FALSE;
    }
    ret=NyLPC_cFormatWriter_print(printHandler,i_inst,i_fmt,i_args);
    return ret;
}

/**
 * テスト用のコード。
 */
#define TEST
#ifndef TEST
//テスト
#include "NyLPC_cHttpHeaderWriter.h"

const char* TP1=
        "HTTP/0.9 200 OK\r\n"
        "HOST: 127.0.0.0.0.0.1\r\n"
        "CONTENt-LENGTH: 1285\r\n"
        "CONNECTION: CloSe\r\n"
        "ETAG: nyatla.jp\r\n"
        "ETAG: nyatla.jp\r\n"
        "Transfer-Encoding:chunked\r\n"
        "\r\n";
const char* TP2=
        "GET /nyanyanya!/nyoronnnnnnnnnnnn?m,fpeofjregnoegnr HTTP/1.1\r\n"
        "HOST: 127.0.0.0.0.0.1\r\n"
        "CONTENt-LENGTH: 1285\r\n"
        "CONNECTION: Keep\r\n"
        "ETAG: nyatla.jp\r\n"
        "ETAG: nyatla.jp\r\n"
        "\r\n";
const char* DT="0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";






void main()
{
    NyLPC_TcHttpStream_t st;
    NyLPC_TcHttpBasicHeaderParser_t hp;
    struct NyLPC_THttpBasicHeader reqheader;
    NyLPC_TcHttpHeaderWriter_t hw;
    NyLPC_TcHttpBodyWriter_t bw;
    NyLPC_TcTcpSocket_t ts;
    int body_len;
    NyLPC_cTcpSocket_initialized(NULL,TP2,strlen(TP2));

    //TCPのオープン
    if(!NyLPC_cHttpStream_initialize(&st,&ts)){
        //エラー
    }
    for(;;){
        //ヘッダ解析
        NyLPC_cHttpBasicHeaderParser_initialize(&hp);
        if(!NyLPC_cHttpShortRequestHeaderParser_parse(&hp,&st,&reqheader)){
            //エラー
            puts("Error");
        }
        //ヘッダの内容確認
        if(reqheader.type!=NyLPC_THttpHeaderType_REQUEST){
            //BadRequest
            puts("Error");
        }
        if(reqheader.startline.req.method!=NyLPC_THttpMethodType_GET){
            //リクエストサポートしてない
            puts("Error");
        }
        //
        NyLPC_cHttpHeaderWriter_initialize(&hw,&st,&reqheader);
//      NyLPC_cHttpResponseWriter_setClose(&hw);
        body_len=100;
        NyLPC_cHttpHeaderWriter_setContentLength(&hw,body_len);
        NyLPC_cHttpHeaderWriter_writeResponseHeader(&hw,500);
        NyLPC_cHttpHeaderWriter_close(&hw);

        NyLPC_cHttpBodyWriter_initialize(&bw,&st);
        NyLPC_cHttpBodyWriter_setChunked(&bw);
        NyLPC_cHttpBodyWriter_write(&bw,"TEST",4);
        NyLPC_cHttpBodyWriter_printf(&bw,"TEST");
        NyLPC_cHttpBodyWriter_printf(&bw,"TEST[%s][%d][%c],%%,[%x]","abcde",123,'s',0xff0011);
        NyLPC_cHttpBodyWriter_close(&bw);
        NyLPC_cHttpHttpWriter_finalize(&hw);
    }
    NyLPC_cHttpStream_finalize(&st);
    //TCPのクローズ
    return;
}
#endif
