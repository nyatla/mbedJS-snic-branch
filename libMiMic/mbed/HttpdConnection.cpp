#include "HttpdConnection.h"
namespace MiMic
{

    int HttpdConnection::getMethodType()
    {
        return NyLPC_cHttpdConnection_getMethod(this->_ref_inst);
    }
    
    bool HttpdConnection::isMethodType(int i_method_type)
    {
        return NyLPC_cHttpdConnection_getMethod(this->_ref_inst)==i_method_type;
    }
    
    HttpdConnection::HttpdConnection(NyLPC_TcHttpdConnection* i_ref_inst)
    {
        this->_ref_inst=i_ref_inst;
    }
    bool HttpdConnection::sendError(unsigned short i_status_code)
    {
        return NyLPC_TBool_TRUE==NyLPC_cHttpdConnection_sendResponseHeader2(this->_ref_inst,i_status_code,"text/html",0,NULL);
    }
    bool HttpdConnection::sendHeader(unsigned short i_status_code,const char* i_content_type,const char* i_additional_header)
    {
        return NyLPC_TBool_TRUE==NyLPC_cHttpdConnection_sendResponseHeader(this->_ref_inst,i_status_code,i_content_type,i_additional_header);
    }
    bool HttpdConnection::sendHeader(unsigned short i_status_code,const char* i_content_type,const char* i_additional_header,unsigned int i_length)
    {
        return NyLPC_TBool_TRUE==NyLPC_cHttpdConnection_sendResponseHeader2(this->_ref_inst,i_status_code,i_content_type,i_length,i_additional_header);
    }
    bool HttpdConnection::sendBody(const void* i_data,NyLPC_TUInt32 i_size)
    {
       return NyLPC_TBool_TRUE==NyLPC_cHttpdConnection_sendResponseBody(this->_ref_inst,i_data,i_size);
    }
    bool HttpdConnection::sendBodyF(const char* i_fmt,...)
    {
        va_list a;
        if(this->_ref_inst->_res_status!=NyLPC_cHttpdConnection_ResStatus_BODY)
        {
            NyLPC_OnErrorGoto(Error);
        }
        //Bodyの書込み
        va_start(a,i_fmt);
        if(!NyLPC_cHttpBodyWriter_formatV(&(this->_ref_inst->_body_writer),i_fmt,a)){
            NyLPC_OnErrorGoto(Error_Send);
        }
        va_end(a);
        return true;
    Error_Send:
        va_end(a);
        NyLPC_cHttpBodyWriter_finalize(&(this->_ref_inst->_in_stream));
    Error:
        this->_ref_inst->_res_status=NyLPC_cHttpdConnection_ResStatus_ERROR;
        return false;
    }
    void HttpdConnection::lockHttpd()
    {
        NyLPC_cHttpdConnection_lock(this->_ref_inst);
    }
    void HttpdConnection::unlockHttpd()
    {
        NyLPC_cHttpdConnection_unlock(this->_ref_inst);
    }
    void HttpdConnection::breakPersistentConnection()
    {
        NyLPC_cHttpdConnection_setConnectionMode(this->_ref_inst,NyLPC_TcHttpdConnection_CONNECTION_MODE_CLOSE);
    }
}