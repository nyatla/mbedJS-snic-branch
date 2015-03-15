#include "NyLPC_cHttpdConnection_protected.h"
#include "NyLPC_http.h"
#include "NyLPC_netif.h"
#include "NyLPC_cHttpdUtils.h"
#include "./NyLPC_cHttpd_protected.h"



NyLPC_TBool NyLPC_cHttpdConnection_initialize(NyLPC_TcHttpdConnection_t* i_inst,NyLPC_TcHttpd_t* i_parent_httpd)
{
    i_inst->_socket=NyLPC_cNet_createTcpSocketEx(NyLPC_TSocketType_TCP_HTTP);
    if(i_inst->_socket==NULL){
    	return NyLPC_TBool_FALSE;
    }
    NyLPC_cHttpRequestPrefixParser_initialize(&(i_inst->_pparser));
    i_inst->_parent_httpd=i_parent_httpd;
    i_inst->_res_status=NyLPC_cHttpdConnection_ResStatus_CLOSED;
    i_inst->_req_status=NyLPC_cHttpdConnection_ReqStatus_LISTEN;
	return NyLPC_TBool_TRUE;
}

void NyLPC_cHttpdConnection_finalize(NyLPC_TcHttpdConnection_t* i_inst)
{
    NyLPC_cHttpdConnection_closeResponse(i_inst);
    NyLPC_cHttpdConnection_closeSocket(i_inst);
    NyLPC_cHttpRequestPrefixParser_finalize(i_inst);
    NyLPC_iTcpSocket_finalize(i_inst->_socket);
}

const NyLPC_TChar* NyLPC_cHttpdConnection_getUrlPrefix(const NyLPC_TcHttpdConnection_t* i_inst)
{
    return NyLPC_cHttpRequestPrefixParser_getUrlPrefix(&i_inst->_pparser);
}
void NyLPC_cHttpdConnection_setReqStatusParsed(NyLPC_TcHttpdConnection_t* i_inst)
{
    i_inst->_req_status=NyLPC_cHttpdConnection_ReqStatus_END;
}

#define NyLPC_cHttpHeaderWriter_CONTENT_LENGTH_UNLIMITED 0xFFFFFFFF


NyLPC_TBool NyLPC_cHttpdConnection_send100Continue(NyLPC_TcHttpdConnection_t* i_inst)
{
    //状態の確認
    if(i_inst->_res_status!=NyLPC_cHttpdConnection_ResStatus_HEAD)
    {
        NyLPC_OnErrorGoto(Error_Status);
    }
    //ステータスラインの記述
    if(!NyLPC_iHttpPtrStream_write(&(i_inst->_in_stream.super),"HTTP/1.1 100 Continue\r\n\r\n",25)){
        NyLPC_OnErrorGoto(Error);
    }
    return NyLPC_TBool_TRUE;
Error:
Error_Status:
    i_inst->_res_status=NyLPC_cHttpdConnection_ResStatus_ERROR;
    return NyLPC_TBool_FALSE;
}

/**
 * レスポンスヘッダを送信します。
 */
NyLPC_TBool NyLPC_cHttpdConnection_sendResponseHeader(NyLPC_TcHttpdConnection_t* i_inst,NyLPC_TUInt16 i_response_code,const NyLPC_TChar* i_content_type,const NyLPC_TChar* i_additional_header)
{
    return NyLPC_cHttpdConnection_sendResponseHeader2(i_inst,i_response_code,i_content_type,NyLPC_cHttpHeaderWriter_CONTENT_LENGTH_UNLIMITED,i_additional_header);
}

NyLPC_TBool NyLPC_cHttpdConnection_sendResponseHeader2(NyLPC_TcHttpdConnection_t* i_inst,NyLPC_TUInt16 i_response_code,const NyLPC_TChar* i_content_type,NyLPC_TUInt32 i_content_length,const NyLPC_TChar* i_additional_header)
{
    NyLPC_TcHttpHeaderWriter_t* h=&(i_inst->_head_writer);
    //状態の確認
    if(i_inst->_res_status!=NyLPC_cHttpdConnection_ResStatus_HEAD)
    {
        NyLPC_OnErrorGoto(Error_Status);
    }
    //ヘッダ送信
    if(!NyLPC_cHttpHeaderWriter_initialize(h,&(i_inst->_in_stream.super),NULL)){
        NyLPC_OnErrorGoto(ERROR_SEND);
    }
    //Headerの転送モードセット
    if(i_content_length==NyLPC_cHttpHeaderWriter_CONTENT_LENGTH_UNLIMITED){
        NyLPC_cHttpHeaderWriter_setChunked(h);
    }else{
        NyLPC_cHttpHeaderWriter_setContentLength(h,i_content_length);
    }
    //continueにセットされていたらcloseをFALSEに
    NyLPC_cHttpHeaderWriter_setConnectionClose(h,(i_inst->_connection_message_mode!=NyLPC_TcHttpdConnection_CONNECTION_MODE_CONTINUE));

    if(!NyLPC_cHttpHeaderWriter_writeResponseHeader(h,i_response_code)){
        NyLPC_OnErrorGoto(ERROR_SEND);
    }
    if(!NyLPC_cHttpHeaderWriter_writeMessage(h,"Content-type",i_content_type)){
        NyLPC_OnErrorGoto(ERROR_SEND);
    }
    if(i_additional_header!=NULL){
        if(!NyLPC_cHttpHeaderWriter_writeRawMessage(h,i_additional_header)){
            NyLPC_OnErrorGoto(ERROR_SEND);
        }
    }
    NyLPC_cHttpHeaderWriter_close(h);
    NyLPC_cHttpHeaderWriter_finalize(h);
    i_inst->_res_status=NyLPC_cHttpdConnection_ResStatus_BODY;
    //BodyWriter生成
    NyLPC_cHttpBodyWriter_initialize(&(i_inst->_body_writer),&(i_inst->_in_stream));
    //bodyのchunkedもセット
    if(i_content_length==NyLPC_cHttpHeaderWriter_CONTENT_LENGTH_UNLIMITED){
        NyLPC_cHttpBodyWriter_setChunked(&(i_inst->_body_writer));
    }else{
        NyLPC_cHttpBodyWriter_setContentLength(&(i_inst->_body_writer),i_content_length);
    }
    return NyLPC_TBool_TRUE;
ERROR_SEND:
    NyLPC_cHttpHeaderWriter_finalize(&(i_inst->_head_writer));
Error_Status:
    i_inst->_res_status=NyLPC_cHttpdConnection_ResStatus_ERROR;
    return NyLPC_TBool_FALSE;
}


/**
 * レスポンスBodyを送信します。
 * 関数を実行後、_res_statusはBODYかERRORに遷移します。
 */
NyLPC_TBool NyLPC_cHttpdConnection_sendResponseBody(NyLPC_TcHttpdConnection_t* i_inst,const void* i_data,NyLPC_TUInt32 i_size)
{
    if(i_inst->_res_status!=NyLPC_cHttpdConnection_ResStatus_BODY)
    {
        NyLPC_OnErrorGoto(Error);
    }
    //Bodyの書込み
    if(!NyLPC_cHttpBodyWriter_write(&(i_inst->_body_writer),i_data,i_size)){
        NyLPC_OnErrorGoto(Error_Send);
    }
    return NyLPC_TBool_TRUE;
Error_Send:
    NyLPC_cHttpBodyWriter_finalize(&(i_inst->_in_stream));
Error:
    i_inst->_res_status=NyLPC_cHttpdConnection_ResStatus_ERROR;
    return NyLPC_TBool_FALSE;
}
/**
 * レスポンスBodyを書式出力して送信します。
 * 関数を実行後、_res_statusはBODYかERRORに遷移します。
 */
NyLPC_TBool NyLPC_cHttpdConnection_sendResponseBodyF(NyLPC_TcHttpdConnection_t* i_inst,const char* i_fmt,...)
{
    va_list a;
    if(i_inst->_res_status!=NyLPC_cHttpdConnection_ResStatus_BODY)
    {
        NyLPC_OnErrorGoto(Error);
    }
    //Bodyの書込み
    va_start(a,i_fmt);
    if(!NyLPC_cHttpBodyWriter_formatV(&(i_inst->_body_writer),i_fmt,a)){
        NyLPC_OnErrorGoto(Error_Send);
    }
    va_end(a);
    return NyLPC_TBool_TRUE;
Error_Send:
    va_end(a);
    NyLPC_cHttpBodyWriter_finalize(&(i_inst->_in_stream));
Error:
    i_inst->_res_status=NyLPC_cHttpdConnection_ResStatus_ERROR;
    return NyLPC_TBool_FALSE;
}

/**
 * ヘッダのみのErrorレスポンスを送信する。
 * この関数はワーク用のHeaderWriterを使います。
 */
static void sendErrorResponse(NyLPC_TcHttpdConnection_t* i_inst,NyLPC_TInt16 i_status)
{
    NyLPC_TcHttpHeaderWriter_t* h=&(i_inst->_head_writer);
    if(NyLPC_cHttpHeaderWriter_initialize(h,&i_inst->_in_stream.super,NULL)){
        //ヘッダを送信
        NyLPC_cHttpHeaderWriter_setConnectionClose(h,NyLPC_TBool_TRUE);
        NyLPC_cHttpHeaderWriter_writeResponseHeader(h,i_status);
        NyLPC_cHttpHeaderWriter_close(h);
        NyLPC_cHttpHeaderWriter_finalize(h);
    }
}
/**
 * 関数を実行後、_res_statusはCLOSEDかHEADかERRORに遷移する。
 */
NyLPC_TBool NyLPC_cHttpdConnection_closeResponse(NyLPC_TcHttpdConnection_t* i_inst)
{
    NyLPC_TcHttpBodyWriter_t* b;
    switch(i_inst->_res_status){
    case NyLPC_cHttpdConnection_ResStatus_CLOSED:
    case NyLPC_cHttpdConnection_ResStatus_ERROR:
        //何もせずにコネクションをクローズする。
        return NyLPC_TBool_FALSE;
    case NyLPC_cHttpdConnection_ResStatus_HEAD:
        //エラー500を送信してクローズする。
        sendErrorResponse(i_inst,500);
        i_inst->_res_status=NyLPC_cHttpdConnection_ResStatus_CLOSED;
        return NyLPC_TBool_FALSE;
    case NyLPC_cHttpdConnection_ResStatus_BODY:
        //正常終了。BODYをクローズし、終了する。
        b=&(i_inst->_body_writer);
        NyLPC_cHttpBodyWriter_close(b);
        NyLPC_cHttpBodyWriter_finalize(&b);
        i_inst->_res_status=NyLPC_cHttpdConnection_ResStatus_HEAD;
        if(i_inst->_connection_message_mode!=NyLPC_TcHttpdConnection_CONNECTION_MODE_CONTINUE)
        {
            i_inst->_res_status=NyLPC_cHttpdConnection_ResStatus_CLOSED;
            return NyLPC_TBool_FALSE;
        }
        return NyLPC_TBool_TRUE;
    default:
        NyLPC_Abort();
    }
    return NyLPC_TBool_TRUE;
}

/**
 * コネクションのプリフェッチデータをヘッダパーサへpushします。
 */
NyLPC_TBool NyLPC_cHttpdConnection_pushPrefetchInfo(NyLPC_TcHttpdConnection_t* i_inst,NyLPC_TcHttpBasicHeaderParser_t* i_header_parser,struct NyLPC_THttpBasicHeader* o_out)
{
    const char* method=NyLPC_THttpMethodType_toString(i_inst->_pparser.method);
    if(NyLPC_cHttpBasicHeaderParser_parseChar(i_header_parser,method,strlen(method),o_out)<0){
        NyLPC_OnErrorGoto(Error);
    }
    if(NyLPC_cHttpBasicHeaderParser_parseChar(i_header_parser," ",1,o_out)<0){
        NyLPC_OnErrorGoto(Error);
    }
    if(NyLPC_cHttpBasicHeaderParser_parseChar(i_header_parser,i_inst->_pparser._url,strlen(i_inst->_pparser._url),o_out)<0){
        NyLPC_OnErrorGoto(Error);
    }
    return NyLPC_TBool_TRUE;
Error:
    return NyLPC_TBool_FALSE;
}

#define NyLPC_cHttpdConnection_TIMEOUT_ACCEPT   3000
#define NyLPC_cHttpdConnection_TIMEOUT_CLOSE    5000
#define NyLPC_cHttpdConnection_TIMEOUT_LISTEN   5000


/**
 * listenerでConnectionのソケットに接続を待ちます。
 */
NyLPC_TBool NyLPC_cHttpdConnection_listenSocket(NyLPC_TcHttpdConnection_t* i_inst,NyLPC_TiTcpListener_t* i_listener)
{
    NyLPC_Assert(i_inst->_req_status==NyLPC_cHttpdConnection_ReqStatus_LISTEN);
    //リスニング
    if(!NyLPC_iTcpListener_listen(i_listener,i_inst->_socket,NyLPC_cHttpdConnection_TIMEOUT_LISTEN)){
        return NyLPC_TBool_FALSE;
    }
    //成功したらステータス遷移
    i_inst->_req_status=NyLPC_cHttpdConnection_ReqStatus_ACCEPT;
    return NyLPC_TBool_TRUE;
}

/**
 * コネクションのソケットをacceptします。
 */
NyLPC_TBool NyLPC_cHttpdConnection_acceptSocket(NyLPC_TcHttpdConnection_t* i_inst)
{
    NyLPC_Assert(i_inst->_req_status==NyLPC_cHttpdConnection_ReqStatus_ACCEPT);

    if(!NyLPC_iTcpSocket_accept(i_inst->_socket,NyLPC_cHttpdConnection_TIMEOUT_ACCEPT)){
        NyLPC_OnErrorGoto(Error);
    }
    //HttpStreamの生成
    if(!NyLPC_cHttpStream_initialize(&i_inst->_in_stream,i_inst->_socket)){
        NyLPC_OnErrorGoto(Error_Connected);
    }
    //初回だけHEADに遷移
    i_inst->_res_status=NyLPC_cHttpdConnection_ResStatus_HEAD;
    i_inst->_req_status=NyLPC_cHttpdConnection_ReqStatus_PREFETCH;
    i_inst->_connection_message_mode=NyLPC_TcHttpdConnection_CONNECTION_MODE_CLOSE;
    return NyLPC_TBool_TRUE;
Error_Connected:
    NyLPC_iTcpSocket_close(i_inst->_socket,NyLPC_cHttpdConnection_TIMEOUT_CLOSE);
    i_inst->_req_status=NyLPC_cHttpdConnection_ReqStatus_LISTEN;
Error:
    return NyLPC_TBool_FALSE;
}

NyLPC_TBool NyLPC_cHttpdConnection_prefetch(NyLPC_TcHttpdConnection_t* i_inst)
{
    NyLPC_Assert(i_inst->_req_status==NyLPC_cHttpdConnection_ReqStatus_PREFETCH);

    //Prefetchを実行
    if(!NyLPC_cHttpRequestPrefixParser_parse(&i_inst->_pparser,&i_inst->_in_stream.super)){
        //400エラー
        sendErrorResponse(i_inst,400);
        NyLPC_OnErrorGoto(Error_Prefetch);
    }
    i_inst->_req_status=NyLPC_cHttpdConnection_ReqStatus_REQPARSE;
    return NyLPC_TBool_TRUE;
Error_Prefetch:
    NyLPC_iTcpSocket_close(i_inst->_socket,NyLPC_cHttpdConnection_TIMEOUT_CLOSE);
    i_inst->_req_status=NyLPC_cHttpdConnection_ReqStatus_LISTEN;
    return NyLPC_TBool_FALSE;
}







NyLPC_TBool NyLPC_cHttpdConnection_prevNextPrefetch(NyLPC_TcHttpdConnection_t* i_inst)
{
    NyLPC_TcHttpNullRequestHeaderParser_t parser;
    switch(i_inst->_req_status)
    {
    case NyLPC_cHttpdConnection_ReqStatus_REQPARSE:
        //リクエストパース待ちなら前段のリクエストを吸収しておく。
        NyLPC_cHttpNullRequestHeaderParser_initialize(&parser);
        //プリフェッチしたデータを流す
        NyLPC_cHttpNullRequestHeaderParser_parseInit(&parser);
        NyLPC_cHttpNullRequestHeaderParser_parseChar(&parser,"GET ",4);//決め打ち
        NyLPC_cHttpNullRequestHeaderParser_parseChar(&parser,i_inst->_pparser._url,strlen(i_inst->_pparser._url));
        //後続をストリームから取り込む
        if(NyLPC_cHttpNullRequestHeaderParser_parseStream(&parser,&(i_inst->_in_stream.super))){
            if(NyLPC_cHttpNullRequestHeaderParser_parseFinish(&parser)){
                NyLPC_cHttpNullRequestHeaderParser_finalize(&parser);
                //OK:403
                sendErrorResponse(i_inst,403);
                break;//OK
            }
        }

        NyLPC_cHttpNullRequestHeaderParser_finalize(&parser);
        //NG:400 Bad Request
        sendErrorResponse(i_inst,400);
        return NyLPC_TBool_FALSE;//吸収失敗
    case NyLPC_cHttpdConnection_ReqStatus_END:
        //リクエストがパース済みならprefetchに戻す。
        i_inst->_req_status=NyLPC_cHttpdConnection_ReqStatus_PREFETCH;
    default:
        NyLPC_Abort();
    }
    //吸収成功
    return NyLPC_TBool_TRUE;
}

void NyLPC_cHttpdConnection_closeSocket(NyLPC_TcHttpdConnection_t* i_inst)
{
    switch(i_inst->_req_status)
    {
    case NyLPC_cHttpdConnection_ReqStatus_LISTEN:
        //何も出来ない。
        break;
    case NyLPC_cHttpdConnection_ReqStatus_END:
    case NyLPC_cHttpdConnection_ReqStatus_REQPARSE:
    case NyLPC_cHttpdConnection_ReqStatus_PREFETCH:
        NyLPC_cHttpStream_finalize(&i_inst->_in_stream);
    case NyLPC_cHttpdConnection_ReqStatus_ACCEPT:
        NyLPC_iTcpSocket_close(i_inst->_socket,NyLPC_cHttpdConnection_TIMEOUT_CLOSE);
    default:
        break;
    }
    i_inst->_req_status=NyLPC_cHttpdConnection_ReqStatus_LISTEN;
    i_inst->_res_status=NyLPC_cHttpdConnection_ResStatus_CLOSED;
}

/**
 * Httpd全体で唯一のロックを取得する。
 */
void NyLPC_cHttpdConnection_lock(NyLPC_TcHttpdConnection_t* i_inst)
{
    NyLPC_cHttpd_lock((NyLPC_TcHttpd_t*)(i_inst->_parent_httpd));
}
/**
 * Httpd全体で唯一のロックを開放する。
 */
void NyLPC_cHttpdConnection_unlock(NyLPC_TcHttpdConnection_t* i_inst)
{
    NyLPC_cHttpd_unlock((NyLPC_TcHttpd_t*)(i_inst->_parent_httpd));
}
