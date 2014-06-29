#pragma once

#include "ModBaseClass.h"
#include "HttpdConnection.h"
#include "NyLPC_cHttpdConnection_protected.h"



namespace MiMic
{
    ModBaseClass::ModBaseClass(const char* i_path)
    {
        this->_path=NULL;
        this->setParam(i_path);
    }
    ModBaseClass::ModBaseClass()
    {
        this->_path=NULL;
    }
    ModBaseClass::~ModBaseClass()
    {
        if(this->_path!=NULL){
            free(this->_path);
        }
    }
    void ModBaseClass::setParam(const char* i_path)
    {
        if(this->_path!=NULL){
            free(this->_path);            
        }
        this->_path=(char*)malloc(strlen(i_path)+1);
        if(this->_path==NULL){
            exit(-1);
        }
        strcpy(this->_path,i_path);
    }
    bool ModBaseClass::canHandle(HttpdConnection& i_connection)
    {
        if(this->_path==NULL){
            return false;
        }
        //connectonの状態を確認
        if(!NyLPC_cHttpdConnection_getReqStatus(i_connection._ref_inst)==NyLPC_cHttpdConnection_ReqStatus_REQPARSE)
        {
            return NyLPC_TBool_FALSE;
        }        
        const NyLPC_TChar* in_url;
        in_url=NyLPC_cHttpdConnection_getUrlPrefix(i_connection._ref_inst);
        size_t base_url_len=strlen(this->_path);
        if(strlen(in_url)-2<base_url_len){
            return false;
        }
        if(in_url[0]!='/' || strncmp(in_url+1,this->_path,base_url_len)!=0 || in_url[base_url_len+1]!='/'){
            return false;
        }
        return true;
    }        
}