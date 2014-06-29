#include "ModWebSocket.h"
#include "../net/httpd/mod/NyLPC_cModWebSocket_protected.h"
#include "HttpdConnection.h"

namespace MiMic
{
    ModWebSocket::ModWebSocket(const char* i_path):ModBaseClass(i_path)
    {
        this->_mod=NULL;
    }
    ModWebSocket::ModWebSocket()
    {
        this->_mod=NULL;
    }
    ModWebSocket::~ModWebSocket()
    {
        if(this->_mod!=NULL){
            NyLPC_cModWebSocket_finalize(this->_mod);
            free(this->_mod);
            this->_mod=NULL;
        }
    }
    bool ModWebSocket::isStarted(){
        return this->_mod!=NULL;
    }
    
    void ModWebSocket::setParam(const char* i_path)
    {
        ModBaseClass::setParam(i_path);
    }
    
    bool ModWebSocket::execute(HttpdConnection& i_connection)
    {
        i_connection.lockHttpd();
        if(this->_mod!=NULL){
            i_connection.unlockHttpd();
            return false;
        }
        this->_mod=(NyLPC_TcModWebSocket_t*)malloc(sizeof(NyLPC_TcModWebSocket_t));
        i_connection.unlockHttpd();

        if(this->_mod==NULL){
            return false;
        }
        //initialize websocket
        NyLPC_cModWebSocket_initialize(this->_mod,this->_path);        
        if(NyLPC_cModWebSocket_canHandle(this->_mod,i_connection._ref_inst)){
            if(NyLPC_cModWebSocket_execute(this->_mod,i_connection._ref_inst)){
                return true;
            }
        }
        NyLPC_cModWebSocket_finalize(this->_mod);
        free(this->_mod);
        i_connection.lockHttpd();
        this->_mod=NULL;
        i_connection.unlockHttpd();
        return false;
    }
    bool ModWebSocket::write(const void* i_tx_buf,int i_tx_size)
    {
        if(this->_mod==NULL){
            return false;
        }
        return NyLPC_cModWebSocket_write(this->_mod,i_tx_buf,i_tx_size)?true:false;
    }



    bool ModWebSocket::writeFormat(const char* i_fmt,...)
    {
        bool ret;
        va_list a;
        //ストリームの状態を更新する。
        va_start(a,i_fmt);
        ret=NyLPC_cModWebSocket_writeFormatV(this->_mod,i_fmt,a)?true:false;
        va_end(a);
        return ret;
    }
    
    int ModWebSocket::read(void* i_rx_buf,int i_rx_size)
    {
        if(this->_mod==NULL){
            return false;
        }
        //write here!
        return NyLPC_cModWebSocket_read(this->_mod,i_rx_buf,i_rx_size);
    }
    bool ModWebSocket::canRead()
    {
        if(this->_mod==NULL){
            return false;
        }
        return NyLPC_cModWebSocket_canRead(this->_mod)?true:false;
    }

    void ModWebSocket::close()
    {
        if(this->_mod==NULL){
            return;
        }
        NyLPC_cModWebSocket_finalize(this->_mod);
        free(this->_mod);
        this->_mod=NULL;
        return;
    }
}