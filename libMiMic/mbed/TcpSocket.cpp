#pragma once
////////////////////////////////////////////////////////////////////////////////
// TcpSocket.h
////////////////////////////////////////////////////////////////////////////////

#include "TcpSocket.h"


namespace MiMic
{
    #define TIMEOUT_IN_MSEC (5*1000)
    
    TcpSocket::TcpSocket(void* i_rx_buf,unsigned short i_rx_buf_size)
    {
        this->_private_rx=NULL;
        NyLPC_cTcpSocket_initialize(&this->_inst,i_rx_buf,i_rx_buf_size);
    }
    TcpSocket::TcpSocket(unsigned short i_rx_buf_size)
    {
        this->_private_rx=malloc(i_rx_buf_size);
        NyLPC_cTcpSocket_initialize(&this->_inst,this->_private_rx,i_rx_buf_size);
    }
    
    TcpSocket::~TcpSocket()
    {
        NyLPC_cTcpSocket_finalize(&this->_inst);
        if(this->_private_rx!=NULL){
            free(this->_private_rx);
        }
    }
    bool TcpSocket::connect(const IpAddr& i_addr,unsigned short i_port)
    {
        return NyLPC_cTcpSocket_connect(&this->_inst,&(i_addr.addr.v4),i_port,TIMEOUT_IN_MSEC)?true:false;
    }
    
    bool TcpSocket::send(const void* i_tx,unsigned short i_tx_size)
    {
        int l,t;
        l=i_tx_size;
        while(l>0){
            t=NyLPC_cTcpSocket_send(&this->_inst,((const char*)i_tx)+(i_tx_size-l),l,TIMEOUT_IN_MSEC);
            if(t<0){
                return false;
            }
            l-=t;
        }
        return true;
    }
    bool TcpSocket::canRecv()
    {
        const void* rx;
        return NyLPC_cTcpSocket_precv(&this->_inst,&rx,0)>0;
    }
    int TcpSocket::precv(const void* &i_rx)
    {
        return NyLPC_cTcpSocket_precv(&this->_inst,&i_rx,TIMEOUT_IN_MSEC);
    }
    int TcpSocket::precv(const char* &i_rx)
    {
        return NyLPC_cTcpSocket_precv(&this->_inst,(const void**)&i_rx,TIMEOUT_IN_MSEC);
    }
    void TcpSocket::pseek(unsigned short i_rx_seek)
    {
        NyLPC_cTcpSocket_pseek(&this->_inst,i_rx_seek);
    }
    void TcpSocket::close()
    {
        return NyLPC_cTcpSocket_close(&this->_inst,TIMEOUT_IN_MSEC);
    }
}