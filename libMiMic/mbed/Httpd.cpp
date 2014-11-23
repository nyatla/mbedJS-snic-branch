#include "Httpd.h"
#include "HttpdConnection.h"
#include "mbed.h"
namespace MiMic
{
    char Httpd::_shared_buf[SIZE_OF_HTTP_BUF];

    void Httpd::onRequestHandler(NyLPC_TcHttpdConnection_t* i_connection)
    {
        HttpdConnection c(i_connection);
        ((struct Httpd2*)(i_connection->_parent_httpd))->_parent->onRequest(c);
    }
    Httpd::Httpd(int i_port_number)
    {
        if(!NyLPC_cHttpd_initialize((NyLPC_TcHttpd_t*)(&this->_inst),(NyLPC_TUInt16)i_port_number)){
        	mbed_die();
        }
        this->_inst._parent=this;
        this->_inst.super.function.onRequest=onRequestHandler;
    }
    Httpd::~Httpd()
    {
        NyLPC_cHttpd_finalize((NyLPC_TcHttpd_t*)(&this->_inst));
    }
    void Httpd::loop()
    {
        NyLPC_cHttpd_loop((NyLPC_TcHttpd_t*)(&this->_inst));
    }    
    void Httpd::lock()
    {
        NyLPC_cHttpd_lock((NyLPC_TcHttpd_t*)(&this->_inst));
    }
    void Httpd::unlock()
    {
        NyLPC_cHttpd_unlock((NyLPC_TcHttpd_t*)(&this->_inst));
    }
    void Httpd::loopTask()
    {
        NyLPC_TcThread_t* th=(NyLPC_TcThread_t*)malloc(sizeof(NyLPC_TcThread_t));
        NyLPC_cThread_initialize(th,256,NyLPC_TcThread_PRIORITY_IDLE);
        NyLPC_cThread_start(th,Httpd::taskHandler,&(this->_inst));
    }
    int Httpd::taskHandler(void* i_param)
    {
        NyLPC_cHttpd_loop((NyLPC_TcHttpd_t*)i_param);
        return 0;
    }
    
}
