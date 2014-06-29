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
#include "NyLPC_cHttpBasicHeaderParser_protected.h"
#include <stdlib.h>

#define HTTP_TIMEOUT NyLPC_TiHttpPtrStream_DEFAULT_HTTP_TIMEOUT

static const struct NyLPC_TTextIdTbl method_id_table[]=
{
    //HTTP STANDARD
    {"GET",NyLPC_THttpMethodType_GET},
    {"POST",NyLPC_THttpMethodType_POST},
    {"HEAD",NyLPC_THttpMethodType_HEAD},
    //SSDP
    {"M-SEARCH",NyLPC_THttpMethodType_M_SEARCH},
    {"NOTIFY",NyLPC_THttpMethodType_NOTIFY},
    {NULL,NyLPC_THttpMethodType_NULL}
};
/*--------------------------------------------------------------------------------
 *
 * NyLPC_THttpMethodType
 *
 --------------------------------------------------------------------------------*/
const char* NyLPC_THttpMethodType_toString(NyLPC_THttpMethodType i_method)
{
    const char* ret=NyLPC_TTextIdTbl_getTextById(i_method,method_id_table);
    if(ret==NULL){
        NyLPC_Abort();
    }
    return ret;
}

static NyLPC_TBool parseRequestMethodStr(NyLPC_TcStr_t* i_str,NyLPC_THttpMethodType* o_out)
{
    //解析処理
    *o_out=NyLPC_TTextIdTbl_getMatchIdIgnoreCase(NyLPC_cStr_str(i_str),method_id_table);
    if(*o_out==NyLPC_THttpMethodType_NULL){
        NyLPC_OnErrorGoto(ERROR);
    }
    return NyLPC_TBool_TRUE;
ERROR:
    return NyLPC_TBool_FALSE;
}
/*--------------------------------------------------------------------------------
 *
 * NyLPC_THttpBasicHeader
 *
 --------------------------------------------------------------------------------*/

NyLPC_TBool NyLPC_THttpBasicHeader_isPersistent(const struct NyLPC_THttpBasicHeader* i_struct)
{
    switch(i_struct->type)
    {
    case NyLPC_THttpHeaderType_REQUEST:
        return (i_struct->connection!=NyLPC_THttpMessgeHeader_Connection_CLOSE)&&(i_struct->startline.req.version==NyLPC_THttpVersion_11);
    case NyLPC_THttpHeaderType_RESPONSE:
    default:
        break;
    }
    return NyLPC_TBool_FALSE;
}








/*
    文字コードの定義
*/
#define HTTP_SP 0x20
#define HTTP_LF 0x0A
#define HTTP_CR 0x0D
#define HTTP_DM ':' 




static NyLPC_TcHttpBasicHeaderParser_ST parseMessage_Connection(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out);
static NyLPC_TcHttpBasicHeaderParser_ST parseMessage_ContentLength(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out);
static NyLPC_TcHttpBasicHeaderParser_ST parseMessage1(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out);
static NyLPC_TcHttpBasicHeaderParser_ST parseVersion(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,NyLPC_TcHttpBasicHeaderParser_ST i_next,struct NyLPC_THttpBasicHeader* o_out);
static NyLPC_TcHttpBasicHeaderParser_ST parseRequestUrl(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out);
static NyLPC_TcHttpBasicHeaderParser_ST parseMessageParam(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out);
static NyLPC_TcHttpBasicHeaderParser_ST parseStartLine(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out);
static NyLPC_TcHttpBasicHeaderParser_ST parseStatusCode(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out);
static NyLPC_TcHttpBasicHeaderParser_ST parseReason(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c);
static NyLPC_TcHttpBasicHeaderParser_ST parseMessage_TransferEncoding(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out);


static NyLPC_TBool parseHttpVersionStr(NyLPC_TcStr_t* i_str,NyLPC_THttpVersion* o_out);
static NyLPC_TBool parseRequestMethodStr(NyLPC_TcStr_t* i_str,NyLPC_THttpMethodType* o_out);

static NyLPC_TBool testHeader(struct NyLPC_THttpBasicHeader* i_header,NyLPC_TUInt16* o_error);


/**
 * デフォルトハンドラ
 */
static const struct NyLPC_TcHttpBasicHeaderParser_Handler _default_handler=
{
    NULL,NULL
};



/*----------------------------------------
    Public/Protected関数
----------------------------------------*/


void NyLPC_cHttpBasicHeaderParser_initialize(NyLPC_TcHttpBasicHeaderParser_t* i_inst,const struct NyLPC_TcHttpBasicHeaderParser_Handler* i_handler)
{
    NyLPC_cStr_initialize(&(i_inst->_wsb),i_inst->_wsb_buf,NyLPC_cHttpBasicHeaderParser_SIZE_OF_WBS);
    i_inst->_handler=((i_handler==NULL)?&_default_handler:i_handler);
}

/**
 * parserの初期化
 */
void NyLPC_cHttpBasicHeaderParser_parseInit(NyLPC_TcHttpBasicHeaderParser_t* i_inst,struct NyLPC_THttpBasicHeader* o_out)
{
    //出力構造体を初期化
    o_out->connection=NyLPC_THttpMessgeHeader_Connection_NONE;
    o_out->content_length=NyLPC_THttpContentLength_INVALID_LENGTH;
    o_out->transfer_encoding=NyLPC_THttpMessgeHeader_TransferEncoding_NONE;
    i_inst->_st=NyLPC_TcHttpBasicHeaderParser_ST_START;
}
/**
 * parseCharがNyLPC_TcHttpBasicHeaderParser_ST_EOHを返却したらコールすること。
 */
NyLPC_TBool NyLPC_cHttpBasicHeaderParser_parseFinish(NyLPC_TcHttpBasicHeaderParser_t* i_inst,struct NyLPC_THttpBasicHeader* o_out)
{
    if(i_inst->_st!=NyLPC_TcHttpBasicHeaderParser_ST_EOH)
    {
        return NyLPC_TBool_FALSE;
    }
    //整合性チェック
    if(!testHeader(o_out,&i_inst->_rcode)){
        i_inst->_st=NyLPC_TcHttpBasicHeaderParser_ST_ERROR;
        return NyLPC_TBool_FALSE;
    }
    return NyLPC_TBool_TRUE;
}
/**
 * 文字列をパースします。
 * コール前にNyLPC_cHttpBasicHeaderParser_parseInitでパーサを開始してください。
 * @return
 * パースした文字列の長さ。-1の場合はエラー。
 * TRUEの場合、NyLPC_cHttpBasicHeaderParser_getParseStatus関数で状態をチェックして、後続の処理を選択してください。
 */
NyLPC_TInt32 NyLPC_cHttpBasicHeaderParser_parseChar(NyLPC_TcHttpBasicHeaderParser_t* i_inst,const NyLPC_TChar* i_c,NyLPC_TInt32 i_size,struct NyLPC_THttpBasicHeader* o_out)
{
    int i;
    NyLPC_TChar c;
    //Errorチェック
    if(NyLPC_TcHttpBasicHeaderParser_ST_ERROR==i_inst->_st)
    {
        return 0;
    }
    for(i=0;i<i_size;i++){
        c=*(i_c+i);
        switch(i_inst->_st)
        {
        case NyLPC_TcHttpBasicHeaderParser_ST_START:
            i_inst->_st=parseStartLine(i_inst,c,o_out);
            break;
        case NyLPC_TcHttpBasicHeaderParser_ST_MSGPARAM:
            i_inst->_st=parseMessageParam(i_inst,c,o_out);
            break;
        case NyLPC_TcHttpBasicHeaderParser_ST_MSGHEAD:
            i_inst->_st=parseMessage1(i_inst,c,o_out);
            break;
        case NyLPC_TcHttpBasicHeaderParser_ST_RL_URL:
            i_inst->_st=parseRequestUrl(i_inst,c,o_out);
            break;
        case NyLPC_TcHttpBasicHeaderParser_ST_RL_VERSION:
            i_inst->_st=parseVersion(i_inst,c,NyLPC_TcHttpBasicHeaderParser_ST_MSGHEAD,o_out);
            break;
        case NyLPC_TcHttpBasicHeaderParser_ST_SL_STATUSCODE:
            i_inst->_st=parseStatusCode(i_inst,c,o_out);
            break;
        case NyLPC_TcHttpBasicHeaderParser_ST_SL_REASON:
            i_inst->_st=parseReason(i_inst,c);
            break;
        case NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONTENTLENGTH:
            i_inst->_st=parseMessage_ContentLength(i_inst,c,o_out);
            break;
        case NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONNECTION:
            i_inst->_st=parseMessage_Connection(i_inst,c,o_out);
            break;
        case NyLPC_TcHttpBasicHeaderParser_ST_MSG_TRANSFERENCODING:
            i_inst->_st=parseMessage_TransferEncoding(i_inst,c,o_out);
            break;
        default:
            i_inst->_rcode=500;
            i_inst->_st=NyLPC_TcHttpBasicHeaderParser_ST_ERROR;
        }
        if(NyLPC_TcHttpBasicHeaderParser_ST_isError(i_inst->_st)){
            //エラー起こしたら終了。
            return i;
        }else if(i_inst->_st==NyLPC_TcHttpBasicHeaderParser_ST_EOH){
            //ヘッダ終端なら終了。
            return i+1;
        }
    }
    return i_size;
}



/**
 * ストリームから読み出して、パースします。
 * コール前にNyLPC_cHttpBasicHeaderParser_parseInitでパーサを開始してください。
 * @return
 * FALSE-失敗/TRUE-成功
 * 関数が成功した場合、NyLPC_cHttpBasicHeaderParser_parseFinishでパーサを閉じることが出来ます。
 */
NyLPC_TBool NyLPC_cHttpBasicHeaderParser_parseStream(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TiHttpPtrStream_t* i_stream,struct NyLPC_THttpBasicHeader* o_out)
{
    const char* rp_base;
    NyLPC_TInt32 rsize;
    for(;;){
        //タイムアウト付でストリームから読み出し。
        rsize=NyLPC_iHttpPtrStream_pread(i_stream,(const void**)(&rp_base),HTTP_TIMEOUT);
        if(rsize<=0){
            return NyLPC_TBool_FALSE;
        }
        rsize=NyLPC_cHttpBasicHeaderParser_parseChar(i_inst,rp_base,rsize,o_out);
        if(i_inst->_st==NyLPC_TcHttpBasicHeaderParser_ST_ERROR){
            //パース失敗
            NyLPC_iHttpPtrStream_rseek(i_stream,rsize);
            return NyLPC_TBool_FALSE;
        }
        if(i_inst->_st==NyLPC_TcHttpBasicHeaderParser_ST_EOH){
            //パース成功
            NyLPC_iHttpPtrStream_rseek(i_stream,rsize);
            return NyLPC_TBool_TRUE;
        }
        NyLPC_iHttpPtrStream_rseek(i_stream,(NyLPC_TUInt16)rsize);
    }
    return NyLPC_TBool_FALSE;
}


///**
// * ストリームから読み出して、パースします。
// */
//NyLPC_TBool NyLPC_cHttpBasicHeaderParser_parse(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TcHttpStream_t* i_stream,struct NyLPC_THttpBasicHeader* o_out)
//{
//  NyLPC_TcHttpBasicHeaderParser_ST st;
//  const char* rp_base;
//  NyLPC_TInt32 rsize;
//  char c;
//  int i;
//
//  //出力構造体を初期化
//  st=NyLPC_TcHttpBasicHeaderParser_ST_START;
//  o_out->connection=NyLPC_THttpMessgeHeader_Connection_NONE;
//  o_out->content_length=NyLPC_THttpContentLength_INVALID_LENGTH;
//  o_out->transfer_encoding=NyLPC_THttpMessgeHeader_TransferEncoding_NONE;
//
//  for(;;){
//      //タイムアウト付でストリームから読み出し。
//      rsize=NyLPC_iHttpPtrStream_pread(i_stream,(const void**)(&rp_base));
//      if(rsize<=0){
//          return NyLPC_TBool_FALSE;
//      }
//      for(i=0;i<rsize;i++){
//          c=*(rp_base+i);
//          switch(st)
//          {
//          case NyLPC_TcHttpBasicHeaderParser_ST_START:
//              st=parseStartLine(i_inst,c,o_out);
//              break;
//          case NyLPC_TcHttpBasicHeaderParser_ST_MSGPARAM:
//              st=parseMessageParam(i_inst,c,o_out);
//              break;
//          case NyLPC_TcHttpBasicHeaderParser_ST_MSGHEAD:
//              st=parseMessage1(i_inst,c,o_out);
//              break;
//          case NyLPC_TcHttpBasicHeaderParser_ST_RL_URL:
//              st=parseRequestUrl(i_inst,c,o_out);
//              break;
//          case NyLPC_TcHttpBasicHeaderParser_ST_RL_VERSION:
//              st=parseVersion(i_inst,c,NyLPC_TcHttpBasicHeaderParser_ST_MSGHEAD,o_out);
//              break;
//          case NyLPC_TcHttpBasicHeaderParser_ST_SL_STATUSCODE:
//              st=parseStatusCode(i_inst,c,o_out);
//              break;
//          case NyLPC_TcHttpBasicHeaderParser_ST_SL_REASON:
//              st=parseReason(i_inst,c);
//              break;
//          case NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONTENTLENGTH:
//              st=parseMessage_ContentLength(i_inst,c,o_out);
//              break;
//          case NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONNECTION:
//              st=parseMessage_Connection(i_inst,c,o_out);
//              break;
//          case NyLPC_TcHttpBasicHeaderParser_ST_MSG_TRANSFERENCODING:
//              st=parseMessage_TransferEncoding(i_inst,c,o_out);
//              break;
//          default:
//              i_inst->_rcode=500;
//              st=NyLPC_TcHttpBasicHeaderParser_ST_ERROR;
//          }
//          //エラー起こしたら終了。
//          if(NyLPC_TcHttpBasicHeaderParser_ST_isError(st)){
//              return NyLPC_TBool_FALSE;
//          }
//          //パース成功
//          if(st==NyLPC_TcHttpBasicHeaderParser_ST_EOH){
//              //整合性チェック
//              if(!testHeader(o_out,&i_inst->_rcode)){
//                  st=NyLPC_TcHttpBasicHeaderParser_ST_ERROR;
//                  return NyLPC_TBool_FALSE;
//              }
//              //シーク
//              NyLPC_iHttpPtrStream_rseek(i_stream,i+1);
//              return NyLPC_TBool_TRUE;
//          }
//      }
//      //シーク
//      NyLPC_iHttpPtrStream_rseek(i_stream,(NyLPC_TUInt16)rsize);
//  }
//  return NyLPC_TBool_FALSE;
//}



/*----------------------------------------
    private関数
----------------------------------------*/
/**
ヘッダの整合性をとる。
*/
static NyLPC_TBool testHeader(struct NyLPC_THttpBasicHeader* i_header,NyLPC_TUInt16* o_error)
{
    switch(i_header->startline.req.version){
    case NyLPC_THttpVersion_09:
        if(i_header->type==NyLPC_THttpHeaderType_REQUEST){
            //Requestの時だけmethodチェック
            //GETだけ
            if(i_header->startline.req.method!=NyLPC_THttpMethodType_GET){
                *o_error=400;
                break;
            }
        }
        //TEは受け付けない。
        if(i_header->transfer_encoding!=NyLPC_THttpMessgeHeader_TransferEncoding_NONE){
            *o_error=400;
            break;
        }
        //ContentLength=0,Connection=Closedに修正。
        i_header->content_length=0;
        i_header->connection=NyLPC_THttpMessgeHeader_Connection_CLOSE;
        return NyLPC_TBool_TRUE;
    case NyLPC_THttpVersion_10:
        //TEは受け付けない。
        if(i_header->transfer_encoding!=NyLPC_THttpMessgeHeader_TransferEncoding_NONE){
            *o_error=406;
            break;
        }
        //ContentLengthが無いときは0
        if(i_header->content_length==NyLPC_THttpContentLength_INVALID_LENGTH){
            i_header->content_length=0;
        }
        //Connection=Closedに修正。(1.0のKeepaliveは難しいから無視)
        i_header->connection=NyLPC_THttpMessgeHeader_Connection_CLOSE;
        return NyLPC_TBool_TRUE;
    case NyLPC_THttpVersion_11:
        if(i_header->content_length==NyLPC_THttpContentLength_INVALID_LENGTH){
            //Contentlength無しのChunked指定はOK
            if(i_header->transfer_encoding!=NyLPC_THttpMessgeHeader_TransferEncoding_CHUNKED){
                //Chunkedが無い場合はContent-Lengthは0と仮定
                i_header->content_length=0;
            }else{
                //content-length無し && Chunked有
                //OK
            }
        }else if(i_header->transfer_encoding!=NyLPC_THttpMessgeHeader_TransferEncoding_NONE){
            //ContentLengthあるのにChunkedとは何事
            *o_error=400;
            break;
        }
        return NyLPC_TBool_TRUE;
    case NyLPC_THttpVersion_UNKNOWN:
        //おい馬鹿やめろ
        *o_error=505;
        break;
    default:
        *o_error=500;
        break;
    }
    return NyLPC_TBool_FALSE;
}

static NyLPC_TcHttpBasicHeaderParser_ST parseMessage_TransferEncoding(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{

    //先頭のスペース除外
    if(NyLPC_cStr_len(&(i_inst->_wsb))==0){
        if(i_c==HTTP_SP){
            return NyLPC_TcHttpBasicHeaderParser_ST_MSG_TRANSFERENCODING;//変化なし
        }
    }
    if(i_c==HTTP_CR){
        //CRの無視
        return NyLPC_TcHttpBasicHeaderParser_ST_MSG_TRANSFERENCODING;//変化なし
    }else if(i_c==HTTP_LF){
        //大文字化
        NyLPC_cStr_toUpper(&(i_inst->_wsb));
        //close?
        o_out->transfer_encoding=NyLPC_cStr_isEqual(&(i_inst->_wsb),"CHUNKED")?NyLPC_THttpMessgeHeader_TransferEncoding_CHUNKED:NyLPC_THttpMessgeHeader_TransferEncoding_UNKNOWN;
        NyLPC_cStr_clear(&(i_inst->_wsb));
        return NyLPC_TcHttpBasicHeaderParser_ST_MSGHEAD;
    }
    if(!NyLPC_cStr_put(&(i_inst->_wsb),i_c)){
        i_inst->_rcode=500;
        NyLPC_OnErrorGoto(Error);
    }
    return NyLPC_TcHttpBasicHeaderParser_ST_MSG_TRANSFERENCODING;//変化なし;
Error:
    return NyLPC_TcHttpBasicHeaderParser_ST_ERROR;
}


static NyLPC_TcHttpBasicHeaderParser_ST parseMessage_Connection(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
    const static NyLPC_TUInt8 id[]={
        NyLPC_THttpMessgeHeader_Connection_CLOSE,
        NyLPC_THttpMessgeHeader_Connection_KEEPALIVE,
        NyLPC_THttpMessgeHeader_Connection_UPGRADE,
        NyLPC_THttpMessgeHeader_Connection_UNKNOWN
    };
    const static NyLPC_TChar* str[]={
        "CLOSE",
        "KEEP-ALIVE",
        "UPGRADE"
    };
    NyLPC_TUInt8 i;
    //先頭のスペース除外
    if(NyLPC_cStr_len(&(i_inst->_wsb))==0){
        if(i_c==HTTP_SP){
            return NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONNECTION;//変化なし
        }
    }
    if(i_c==HTTP_CR){
        //CRの無視
        return NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONNECTION;//変化なし
    }else if(i_c==HTTP_LF){
        //大文字化
        NyLPC_cStr_toUpper(&(i_inst->_wsb));
        //Convert to ID
        o_out->connection=NyLPC_THttpMessgeHeader_Connection_UNKNOWN;
        for(i=0;id[i]!=NyLPC_THttpMessgeHeader_Connection_UNKNOWN;i++){
            if(NyLPC_cStr_isEqual(&(i_inst->_wsb),str[i])){
                o_out->connection=id[i];
                break;
            }
        }
        NyLPC_cStr_clear(&(i_inst->_wsb));
        return NyLPC_TcHttpBasicHeaderParser_ST_MSGHEAD;
    }
    if(!NyLPC_cStr_put(&(i_inst->_wsb),i_c)){
        i_inst->_rcode=500;
        NyLPC_OnErrorGoto(Error);
    }
    return NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONNECTION;//変化なし;
Error:
    return NyLPC_TcHttpBasicHeaderParser_ST_ERROR;
}


static NyLPC_TcHttpBasicHeaderParser_ST parseMessage_ContentLength(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
    char* e;
    char* p;

    //先頭のスペース除外
    if(NyLPC_cStr_len(&(i_inst->_wsb))==0)
    {
        if(i_c==HTTP_SP){
            return NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONTENTLENGTH;//変化なし
        }
    }
    if(i_c==HTTP_CR){
        //CRの無視
        return NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONTENTLENGTH;//変化なし
    }else if(i_c==HTTP_LF){
        p=NyLPC_cStr_str(&(i_inst->_wsb));
        o_out->content_length=strtol(p,&e,10);
        if(e==p){
            i_inst->_rcode=400;
            NyLPC_OnErrorGoto(Error);//ｷﾞｬｰ
        }
        NyLPC_cStr_clear(&(i_inst->_wsb));
        return NyLPC_TcHttpBasicHeaderParser_ST_MSGHEAD;
    }
    if(!NyLPC_cStr_put(&(i_inst->_wsb),i_c)){
        i_inst->_rcode=500;
        NyLPC_OnErrorGoto(Error);
    }
    return NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONTENTLENGTH;//変化なし;
Error:
    return NyLPC_TcHttpBasicHeaderParser_ST_ERROR;
}

static NyLPC_TcHttpBasicHeaderParser_ST parseStatusCode(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
    NyLPC_TcStr_t* ws=&(i_inst->_wsb);
    char* e;
    char* p;

    //先頭のスペース除外
    if(NyLPC_cStr_len(ws)==0)
    {
        if(i_c==HTTP_SP){
            return NyLPC_TcHttpBasicHeaderParser_ST_SL_STATUSCODE;//変化なし
        }
    }
    if(i_c==HTTP_SP){
    //SPで終了
        p=NyLPC_cStr_str(ws);
        o_out->startline.res.status=(strtol(p,&e,10));
        if(e==p){
            i_inst->_rcode=400;
            NyLPC_OnErrorGoto(Error);//ｷﾞｬｰ
        }
        NyLPC_cStr_clear(ws);
        return NyLPC_TcHttpBasicHeaderParser_ST_SL_REASON;
    }
    if(!NyLPC_cStr_put(ws,i_c)){
        i_inst->_rcode=500;
        NyLPC_OnErrorGoto(Error);
    }
    return NyLPC_TcHttpBasicHeaderParser_ST_SL_STATUSCODE;//変化なし;
Error:
    return NyLPC_TcHttpBasicHeaderParser_ST_ERROR;
}
static NyLPC_TcHttpBasicHeaderParser_ST parseReason(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c)
{
    NyLPC_TcStr_t* ws=&(i_inst->_wsb);
    //LFくるまで飛ばす。
    switch(i_c){
    case HTTP_LF:
        NyLPC_cStr_clear(ws);
        return NyLPC_TcHttpBasicHeaderParser_ST_MSGHEAD;
    default:
        break;
    }
    //URLパーサへ通知
    return NyLPC_TcHttpBasicHeaderParser_ST_SL_REASON;//変化なし
}
static NyLPC_TcHttpBasicHeaderParser_ST parseMessageParam(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
    NyLPC_TcStr_t* ws=&(i_inst->_wsb);
    //先頭のスペース除外
    if(NyLPC_cStr_len(ws)==0){
        if(i_c==HTTP_SP){
            return NyLPC_TcHttpBasicHeaderParser_ST_MSGPARAM;//変化なし
        }else{
            NyLPC_cStr_put(ws,'C');//開始フラグ
        }
    }
    switch(i_c){
    case HTTP_CR:
        return NyLPC_TcHttpBasicHeaderParser_ST_MSGPARAM;//変化なし
    case HTTP_LF:
        //メッセージフィールドの終端を通知
        if(i_inst->_handler->messageHandler!=NULL){
            if(!i_inst->_handler->messageHandler(i_inst,NULL,0,o_out)){
                i_inst->_rcode=500;
                NyLPC_OnErrorGoto(Error);
            }
        }
        NyLPC_cStr_clear(ws);
        return NyLPC_TcHttpBasicHeaderParser_ST_MSGHEAD;
    default:
        //メッセージフィールドの追記
        if(i_inst->_handler->messageHandler!=NULL){
            if(!i_inst->_handler->messageHandler(i_inst,NULL,i_c,o_out)){
                i_inst->_rcode=500;
                NyLPC_OnErrorGoto(Error);
            }
        }
        break;
    }
    //URLパーサへ通知
    return NyLPC_TcHttpBasicHeaderParser_ST_MSGPARAM;//変化なし
Error:
    return NyLPC_TcHttpBasicHeaderParser_ST_ERROR;
}

static NyLPC_TcHttpBasicHeaderParser_ST parseMessage1(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
    const static char* KNOWN_MSG[]={"CONNECTION","CONTENT-LENGTH","TRANSFER-ENCODING",NULL};
    int i;

    switch(i_c){
    case HTTP_DM:
    //メッセージの名前確定。遷移先判定
        //ヘッダ名を大文字にする。
        NyLPC_cStr_toUpper(&(i_inst->_wsb));
        for(i=0;KNOWN_MSG[i]!=NULL;i++){
            if(NyLPC_cStr_isEqual(&(i_inst->_wsb),KNOWN_MSG[i])){
                //確定。
                NyLPC_cStr_clear(&(i_inst->_wsb));
                switch(i){
                case 0://CONNECTION
                    return NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONNECTION;
                case 1://CONTENT-LENGTH
                    return NyLPC_TcHttpBasicHeaderParser_ST_MSG_CONTENTLENGTH;
                case 2://TRANSFER-ENCODING
                    return NyLPC_TcHttpBasicHeaderParser_ST_MSG_TRANSFERENCODING;
                default://エラー
                    break;
                }
                i_inst->_rcode=500;
                NyLPC_OnErrorGoto(Error);
            }
        }
        //メッセージフィールドの名前を通知
        if(i_inst->_handler->messageHandler!=NULL){
            if(!i_inst->_handler->messageHandler(i_inst,NyLPC_cStr_str(&(i_inst->_wsb)),0,o_out)){
                i_inst->_rcode=500;
                NyLPC_OnErrorGoto(Error);
            }
            NyLPC_cStr_clear(&(i_inst->_wsb));
        }
        //カスタムヘッダ解析へ。
        return NyLPC_TcHttpBasicHeaderParser_ST_MSGPARAM;
    case HTTP_CR:
        return NyLPC_TcHttpBasicHeaderParser_ST_MSGHEAD;//変化なし
    case HTTP_LF:
        //1文字で終了ならパースエンド。バリデーションチェックへ
        if(NyLPC_cStr_len(&(i_inst->_wsb))==0){
            NyLPC_cStr_clear(&(i_inst->_wsb));
            return NyLPC_TcHttpBasicHeaderParser_ST_EOH;
        }
        //これはひどい。
        i_inst->_rcode=400;
        NyLPC_OnErrorGoto(Error);
    default:
        break;
    }
    if(!NyLPC_cStr_put(&(i_inst->_wsb),i_c)){
        i_inst->_rcode=500;
        NyLPC_OnErrorGoto(Error);
    }
    return NyLPC_TcHttpBasicHeaderParser_ST_MSGHEAD;//変化なし;
Error:
    return NyLPC_TcHttpBasicHeaderParser_ST_ERROR;
}

static NyLPC_TcHttpBasicHeaderParser_ST parseVersion(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,NyLPC_TcHttpBasicHeaderParser_ST i_next,struct NyLPC_THttpBasicHeader* o_out)
{
    //先頭のスペース除外
    if(NyLPC_cStr_len(&(i_inst->_wsb))==0){
        if(i_c==HTTP_SP){
            return NyLPC_TcHttpBasicHeaderParser_ST_RL_VERSION;//変化なし
        }
    }
    if(i_c==HTTP_CR){
    //CRの無視
        return NyLPC_TcHttpBasicHeaderParser_ST_RL_VERSION;//変化なし
    }else if(i_c==HTTP_LF){
    //LFで確定
        if(!parseHttpVersionStr(&(i_inst->_wsb),&(o_out->startline.req.version))){
            i_inst->_rcode=505;
            NyLPC_cStr_clear(&(i_inst->_wsb));
            NyLPC_OnErrorGoto(Error);
        }
        NyLPC_cStr_clear(&(i_inst->_wsb));
        return i_next;//遷移(エラーの時はそのままエラーコードが渡る。)
    }
    if(!NyLPC_cStr_put(&(i_inst->_wsb),i_c)){
        //追記処理しっぱい
        i_inst->_rcode=500;
        NyLPC_OnErrorGoto(Error);
    }
    return NyLPC_TcHttpBasicHeaderParser_ST_RL_VERSION;//変化なし
Error:
    return NyLPC_TcHttpBasicHeaderParser_ST_ERROR;
}

/**
URLパーサ。登録した関数に転送する？
*/
static NyLPC_TcHttpBasicHeaderParser_ST parseRequestUrl(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
    //先頭のスペース除外
    if(NyLPC_cStr_len(&(i_inst->_wsb))==0){
        if(i_c==HTTP_SP){
            return NyLPC_TcHttpBasicHeaderParser_ST_RL_URL;//変化なし
        }else{
            NyLPC_cStr_put(&(i_inst->_wsb),'C');//開始フラグ
        }
    }
    //次のスペースがくるまで。
    if(i_c==HTTP_SP){
        NyLPC_cStr_clear(&(i_inst->_wsb));
        //URLハンドラへ通知
        if(i_inst->_handler->urlHandler!=NULL){
            if(!i_inst->_handler->urlHandler(i_inst,0,o_out)){
                i_inst->_rcode=500;
                NyLPC_OnErrorGoto(Error);
            }
        }
        return NyLPC_TcHttpBasicHeaderParser_ST_RL_VERSION;
    }
    //URLパーサへ通知
    if(i_inst->_handler->urlHandler!=NULL){
        if(!i_inst->_handler->urlHandler(i_inst,i_c,o_out)){
            i_inst->_rcode=500;
            NyLPC_OnErrorGoto(Error);
        }
    }
    return NyLPC_TcHttpBasicHeaderParser_ST_RL_URL;//変化なし
Error:
    return NyLPC_TcHttpBasicHeaderParser_ST_ERROR;
}

/**
Methodパーサ
    [:HTTPMETHOD:]を得る。
*/
static NyLPC_TcHttpBasicHeaderParser_ST parseStartLine(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
    if(i_c==HTTP_SP){
    //SPがデリミタ
        //HTTPステータスを試す。
        if(parseHttpVersionStr(&(i_inst->_wsb),&(o_out->startline.res.version))){
            //これはHTTPステータス
            o_out->type=NyLPC_THttpHeaderType_RESPONSE;
            NyLPC_cStr_clear(&(i_inst->_wsb));
            return NyLPC_TcHttpBasicHeaderParser_ST_SL_STATUSCODE;
        }
        //HTTPリクエストを試す。
        if(!parseRequestMethodStr(&(i_inst->_wsb),&(o_out->startline.req.method))){
            i_inst->_rcode=400;
            NyLPC_OnErrorGoto(ERROR);
        }
        //これはHTTPリクエスト
        o_out->type=NyLPC_THttpHeaderType_REQUEST;
        NyLPC_cStr_clear(&(i_inst->_wsb));
        return NyLPC_TcHttpBasicHeaderParser_ST_RL_URL;
    }
    if(!NyLPC_cStr_put(&(i_inst->_wsb),i_c)){
        i_inst->_rcode=500;
        NyLPC_OnErrorGoto(ERROR);
    }
    return NyLPC_TcHttpBasicHeaderParser_ST_START;//変化なし
ERROR:
    return NyLPC_TcHttpBasicHeaderParser_ST_ERROR;
}





static NyLPC_TBool parseHttpVersionStr(NyLPC_TcStr_t* i_str,NyLPC_THttpVersion* o_out)
{
    NyLPC_TChar* p;
    char* e;
    long ma,mi;

    p=NyLPC_cStr_str(i_str);
    if(NyLPC_cStr_len(i_str)<6){
        NyLPC_OnErrorGoto(Error);
    }
    if(strncmp(p,"HTTP/",5)!=0){
        NyLPC_OnErrorGoto(Error);
    }
    p+=5;
    ma=strtol(p,&e,10);
    if(p==e){
        NyLPC_OnErrorGoto(Error);
    }
    p=e;//.をチェック
    if(*p!='.'){
        NyLPC_OnErrorGoto(Error);
    }
    p++;
    mi=strtoul(p,&e,10);
    if(p==e){
        NyLPC_OnErrorGoto(Error);
    }
    if(ma<0 ||mi<0){
        NyLPC_OnErrorGoto(Error);
    }
    switch(ma){
    case 0:
        if(mi>=9){
            //HTTP0.9相当
            *o_out=NyLPC_THttpVersion_09;
        }
        break;
    case 1:
        if(mi==0){
            //HTTP1.0
            *o_out=NyLPC_THttpVersion_10;
        }else if(mi>=1){
            //HTTP1.1
            *o_out=NyLPC_THttpVersion_11;
        }else{
            *o_out=NyLPC_THttpVersion_UNKNOWN;
        }
        break;
    default:
        //お前など知らん
        *o_out=NyLPC_THttpVersion_UNKNOWN;
        break;
    }
    return NyLPC_TBool_TRUE;//変化なし
Error:
    return NyLPC_TBool_FALSE;
}

