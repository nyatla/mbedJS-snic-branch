/*
 * NyLPC_cHttpModUtils_protected.c
 *
 *  Created on: 2013/03/05
 *      Author: nyatla
 */
#include "NyLPC_cHttpdUtils.h"
#include "NyLPC_cHttpdConnection_protected.h"



NyLPC_TBool NyLPC_cHttpdUtils_sendFixedContentBatch(NyLPC_TcHttpdConnection_t* i_connection,const NyLPC_TChar* i_content_type,const NyLPC_TChar* i_content,NyLPC_TUInt32 i_size)
{
    //HEAD or GET
    switch(NyLPC_cHttpdConnection_getMethod(i_connection))
    {
    case NyLPC_THttpMethodType_HEAD:
        //HTTP Header
        NyLPC_cHttpdConnection_sendResponseHeader2(i_connection,200,i_content_type,i_size,NULL);
        break;
    case NyLPC_THttpMethodType_GET:
        //HTTP Header
        NyLPC_cHttpdConnection_sendResponseHeader2(i_connection,200,i_content_type,i_size,NULL);
        //HTTP Body
        NyLPC_cHttpdConnection_sendResponseBody(i_connection,i_content,i_size);
        break;
    default:
        //ERROR 405
        NyLPC_cHttpdConnection_sendResponseHeader2(i_connection,405,"text/html",0,NULL);
        return NyLPC_TBool_FALSE;
    }
    return NyLPC_TBool_TRUE;
}

void NyLPC_cHttpdUtils_sendErrorResponse(NyLPC_TcHttpdConnection_t* i_connection,int i_status)
{
    NyLPC_TUInt8 mt=NyLPC_cHttpdConnection_getMethod(i_connection);
    //ConnectionをCLOSEへセット
    NyLPC_cHttpdConnection_setConnectionMode(i_connection,NyLPC_TcHttpdConnection_CONNECTION_MODE_CLOSE);
    NyLPC_cHttpdConnection_sendResponseHeader(i_connection,i_status,"text/html",NULL);
    if(mt!=NyLPC_THttpMethodType_HEAD){
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,"<!DOCTYPE html><html><head><title>MiMicHTTPD</title></head><body>Status %d</body></html>",i_status);
    }
}



/**
 * 標準的なJsonヘッダを送信する。
 */
NyLPC_TBool NyLPC_cHttpdUtils_sendJsonHeader(NyLPC_TcHttpdConnection_t* i_connection)
{
    const static char* additional_header=
        "Access-Control-Allow-Origin:*\r\n"
        "Pragma: no-cache\r\n"
        "Cache-Control: no-cache\r\n";
    const static char* content_type="application/json";
    return NyLPC_cHttpdConnection_sendResponseHeader(i_connection,200,content_type,additional_header);
}

