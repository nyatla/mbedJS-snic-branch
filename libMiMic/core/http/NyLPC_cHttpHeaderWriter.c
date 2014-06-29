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

#include "NyLPC_cHttpHeaderWriter.h"
#include "NyLPC_cHttpdConfig.h"


/**
 * Httpリクエストヘッダに対応したHttpヘッダライタを構築します。
 */
NyLPC_TBool NyLPC_cHttpHeaderWriter_initialize(NyLPC_TcHttpHeaderWriter_t* i_inst,NyLPC_TiHttpPtrStream_t* i_ref_stream,const struct NyLPC_THttpBasicHeader* i_req_header)
{
    i_inst->_is_chunked=NyLPC_TUInt8_FALSE;
    i_inst->_content_length=0;
    i_inst->_ref_stream=i_ref_stream;
    i_inst->_is_error=NyLPC_TUInt8_FALSE;
    //書込エンコーディングをなしにセット
    NyLPC_iHttpPtrStream_setWriteEncoding(i_inst->_ref_stream,NyLPC_TiHttpPtrStream_ET_NONE);
    //必要に応じてリクエストの内容をパース
    if(i_req_header!=NULL){
        if(i_req_header->type!=NyLPC_THttpHeaderType_REQUEST){
            return NyLPC_TBool_FALSE;
        }
        //コネクションの持続性を決定
        if((i_req_header->connection==NyLPC_THttpMessgeHeader_Connection_CLOSE)||
            (i_req_header->startline.req.version!=NyLPC_THttpVersion_11))
        {
            i_inst->_is_close=NyLPC_TUInt8_TRUE;
        }else{
            i_inst->_is_close=NyLPC_TUInt8_FALSE;
        }
    }else{
        i_inst->_is_close=NyLPC_TUInt8_TRUE;
    }
    return NyLPC_TBool_TRUE;
}

#define NyLPC_cHttpHttpWriter_finalize(i)


void NyLPC_cHttpHeaderWriter_setChunked(NyLPC_TcHttpHeaderWriter_t* i_inst)
{
    i_inst->_is_chunked=NyLPC_TUInt8_TRUE;
    return;
}
void NyLPC_cHttpHeaderWriter_setContentLength(NyLPC_TcHttpHeaderWriter_t* i_inst,NyLPC_TUInt32 i_content_length)
{
    i_inst->_content_length=i_content_length;
    return;
}
void NyLPC_cHttpHeaderWriter_setConnectionClose(NyLPC_TcHttpHeaderWriter_t* i_inst,NyLPC_TBool i_is_close)
{
    i_inst->_is_close=i_is_close;
    return;
}



const static struct{
    NyLPC_TUInt16 code;
    const NyLPC_TChar*  message;
}status_line_tbl[]={
    {200,"OK"},
    {301,"Moved Permanently"},
    {302,"Moved Temporarily"},
    {400,"Bad Request"},
    {403,"Forbidden"},
    {404,"Not Found"},
    {405,"Method Not Allowed"},
    {500,"Internal Server Error"},
    {0,NULL}//これ最後にしてね。
};
const static char* getStatusMessage(NyLPC_TUInt16 i_status)
{
    int i=0;
    while(status_line_tbl[i].code!=0){
        if(i_status==status_line_tbl[i].code){
            return status_line_tbl[i].message;
        }
        i++;
    }
    return NULL;
}

static NyLPC_TBool writeln(NyLPC_TiHttpPtrStream_t* i_inst,const void* i_data,NyLPC_TInt16 i_length)
{
    if(NyLPC_iHttpPtrStream_write(i_inst,i_data,i_length)){
        if(NyLPC_iHttpPtrStream_write(i_inst,"\r\n",2)){
            return NyLPC_TBool_TRUE;
        }
    }
    return NyLPC_TBool_FALSE;
}


NyLPC_TBool NyLPC_cHttpHeaderWriter_writeRequestHeader(NyLPC_TcHttpHeaderWriter_t* i_inst,NyLPC_THttpMethodType i_method,const struct NyLPC_TIPv4Addr* i_host,NyLPC_TUInt16 i_port,const NyLPC_TChar* i_path)
{
    const NyLPC_TChar* t;
    NyLPC_TChar v[16];
    //エラー状態ならなにもしない。
    if(i_inst->_is_error){
        return NyLPC_TBool_FALSE;
    }

    t=NyLPC_THttpMethodType_toString(i_method);
    if(t==NULL){
        return NyLPC_TBool_FALSE;
    }
    //リクエストラインの記述
    //Method
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,t,-1)){
        NyLPC_OnErrorGoto(Error);
    }
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream," ",1)){
        NyLPC_OnErrorGoto(Error);
    }
    //Path
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,i_path,-1)){
        NyLPC_OnErrorGoto(Error);
    }
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream," HTTP/1.1\r\n",11)){
        NyLPC_OnErrorGoto(Error);
    }
    //HOSTの記述
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,"Host: ",6)){
        NyLPC_OnErrorGoto(Error);
    }
    NyLPC_TIPv4Addr_toString(i_host,v);
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,v,-1)){
        NyLPC_OnErrorGoto(Error);
    }
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,":",1)){
        NyLPC_OnErrorGoto(Error);
    }
    NyLPC_uitoa(i_port,v,10);
    if(!writeln(i_inst->_ref_stream,v,-1)){
        NyLPC_OnErrorGoto(Error);
    }

    //close
    if(i_inst->_is_close){
        if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,"Connection: CLOSE\r\n",-1)){
            NyLPC_OnErrorGoto(Error);
        }
    }

    //chunked
    if(i_inst->_is_chunked){
        if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,"Transfer-Encoding: chunked\r\n",-1)){
            NyLPC_OnErrorGoto(Error);
        }
    }else{
        if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,"Content-Length: ",-1)){
            NyLPC_OnErrorGoto(Error);
        }
        NyLPC_uitoa(i_inst->_content_length,v,10);
        if(!writeln(i_inst->_ref_stream,v,-1)){
            NyLPC_OnErrorGoto(Error);
        }
    }
    //送信サイズをリセット
    i_inst->_size_of_sent=0;
    return NyLPC_TBool_TRUE;
Error:
    i_inst->_is_error=NyLPC_TUInt8_FALSE;
    return NyLPC_TBool_FALSE;

}

/**
 * ステータスラインと、標準メッセージヘッダを出力します。
 */
NyLPC_TBool NyLPC_cHttpHeaderWriter_writeResponseHeader(NyLPC_TcHttpHeaderWriter_t* i_inst,NyLPC_TUInt16 i_status)
{
    NyLPC_TChar v[12];
    const char* m=getStatusMessage(i_status);
    //エラー状態ならなにもしない。
    if(i_inst->_is_error){
        return NyLPC_TBool_FALSE;
    }
    //検索に失敗したら500番に変更
    if(m==NULL){
        i_status=500;
        m=getStatusMessage(500);
    }
    //ステータスラインの記述
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,"HTTP/1.1 ",9)){
        NyLPC_OnErrorGoto(Error);
    }
    NyLPC_itoa(i_status,v,10);
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,v,-1)){
        NyLPC_OnErrorGoto(Error);
    }
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream," ",1)){
        NyLPC_OnErrorGoto(Error);
    }
    if(!writeln(i_inst->_ref_stream,m,-1)){
        NyLPC_OnErrorGoto(Error);
    }
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,"Server: " NyLPC_cHttpdConfig_SERVER "\r\n",-1)){
        NyLPC_OnErrorGoto(Error);
    }
    if(i_inst->_is_close){
        if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,"Connection: CLOSE\r\n",-1)){
            NyLPC_OnErrorGoto(Error);
        }
    }
    //ヘッダの記述
    if(i_inst->_is_chunked){
        if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,"Transfer-Encoding: chunked\r\n",-1)){
            NyLPC_OnErrorGoto(Error);
        }
    }else{
        if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,"Content-Length: ",-1)){
            NyLPC_OnErrorGoto(Error);
        }
        NyLPC_uitoa(i_inst->_content_length,v,10);
        if(!writeln(i_inst->_ref_stream,v,-1)){
            NyLPC_OnErrorGoto(Error);
        }
    }
    //送信サイズをリセット
    i_inst->_size_of_sent=0;
    return NyLPC_TBool_TRUE;
Error:
    i_inst->_is_error=NyLPC_TUInt8_FALSE;
    return NyLPC_TBool_FALSE;
}

/**
 * 独自定義のメッセージヘッダを記述します。
 */
NyLPC_TBool NyLPC_cHttpHeaderWriter_writeMessage(NyLPC_TcHttpHeaderWriter_t* i_inst,const NyLPC_TChar* i_name,const NyLPC_TChar* i_field)
{
    //エラー状態ならなにもしない。
    if(i_inst->_is_error){
        return NyLPC_TBool_FALSE;
    }
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,i_name,-1)){
        NyLPC_OnErrorGoto(Error);
    }
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,": ",2)){
        NyLPC_OnErrorGoto(Error);
    }
    if(!writeln(i_inst->_ref_stream,i_field,-1)){
        NyLPC_OnErrorGoto(Error);
    }
    return NyLPC_TBool_TRUE;
Error:
    i_inst->_is_error=NyLPC_TUInt8_FALSE;
    return NyLPC_TBool_FALSE;
}


NyLPC_TBool NyLPC_cHttpHeaderWriter_writeRawMessage(NyLPC_TcHttpHeaderWriter_t* i_inst,const NyLPC_TChar* i_additional_header)
{
    //エラー状態ならなにもしない。
    if(i_inst->_is_error){
        return NyLPC_TBool_FALSE;
    }
    if(!NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,i_additional_header,strlen(i_additional_header))){
        NyLPC_OnErrorGoto(Error);
    }
    return NyLPC_TBool_TRUE;
Error:
    i_inst->_is_error=NyLPC_TUInt8_FALSE;
    return NyLPC_TBool_FALSE;
}

/**
 * Httpヘッダの書き込みを完了します。
 * 続けてbody転送が可能な場合は、必要に応じてエンコーディングモードを更新します。
 * @return
 * 現在のストリームのステータスを返します。
 */
NyLPC_TBool NyLPC_cHttpHeaderWriter_close(NyLPC_TcHttpHeaderWriter_t* i_inst)
{
    //エラー状態ならなにもしない。
    if(i_inst->_is_error){
        return NyLPC_TBool_FALSE;
    }
    if(NyLPC_iHttpPtrStream_write(i_inst->_ref_stream,"\r\n",2)){
        if( NyLPC_iHttpPtrStream_flush(i_inst->_ref_stream)){
            return NyLPC_TBool_TRUE;
        }
    }
    return NyLPC_TBool_FALSE;
}



