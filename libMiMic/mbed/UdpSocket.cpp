////////////////////////////////////////////////////////////////////////////////
// UdpSocket.cpp
////////////////////////////////////////////////////////////////////////////////

#include "UdpSocket.h"




namespace MiMic
{
    #define TIMEOUT_IN_MSEC (2*1000)

    UdpSocket::UdpSocket(unsigned short i_port,unsigned short i_rx_buf_size)
    {
        this->_private_rx=malloc(i_rx_buf_size);
        NyLPC_cUdpSocket_initialize(&this->_inst,i_port,this->_private_rx,i_rx_buf_size);    
    }
    UdpSocket::UdpSocket(unsigned short i_port,void* i_rx_buf,unsigned short i_rx_buf_size)
    {
        this->_private_rx=NULL;
        NyLPC_cUdpSocket_initialize(&this->_inst,i_port,i_rx_buf,i_rx_buf_size);    
    }

    UdpSocket::UdpSocket(unsigned short i_port,void* i_rx_handler)
    {
    }
    UdpSocket::~UdpSocket()
    {
        NyLPC_cUdpSocket_finalize(&this->_inst);
        if(this->_private_rx!=NULL){
            free(this->_private_rx);
        }
    }
    bool UdpSocket::canRecv()
    {
        const void* rx;
        const struct NyLPC_TIPv4RxInfo* info;
        return NyLPC_cUdpSocket_precv(&this->_inst,&rx,&info,TIMEOUT_IN_MSEC)>0;
    }

    int UdpSocket::precvFrom(const void* &i_rx,IpAddr* i_peer_host,unsigned short* i_port)
    {
        const struct NyLPC_TIPv4RxInfo* info;
        int rs=NyLPC_cUdpSocket_precv(&this->_inst,&i_rx,&info,TIMEOUT_IN_MSEC);
        if(rs>1){
            if(i_peer_host!=NULL){
                i_peer_host->setIPv4(info->peer_ip);
            }
            if(i_port!=NULL){
                *i_port=info->peer_port;
            }
        }
        return rs;
    }
    int UdpSocket::precvFrom(const char* &i_rx,IpAddr* i_peer_host,unsigned short* i_port)
    {
        const struct NyLPC_TIPv4RxInfo* info;
        int rs=NyLPC_cUdpSocket_precv(&this->_inst,(const void**)&i_rx,&info,TIMEOUT_IN_MSEC);
        if(rs>1){
            if(i_peer_host!=NULL){
                i_peer_host->setIPv4(info->peer_ip);
            }
            if(i_port!=NULL){
                *i_port=info->peer_port;
            }
        }
        return rs;
    }

    void UdpSocket::precvNext(void)
    {
        NyLPC_cUdpSocket_pseek(&this->_inst);
    }
    
    bool UdpSocket::sendTo(const IpAddr& i_host,unsigned short i_port,const void* i_tx,unsigned short i_tx_size)
    {
        int r=NyLPC_cUdpSocket_send(&this->_inst,&i_host.addr.v4,i_port,i_tx,i_tx_size,TIMEOUT_IN_MSEC);
        return (r==i_tx_size);
    
    }
    
    void UdpSocket::joinMulticast(const IpAddr& i_host)
    {
        NyLPC_cUdpSocket_joinMulticast(&this->_inst,&i_host.addr.v4);
    }
    void UdpSocket::setBroadcast(void)
    {
        NyLPC_cUdpSocket_setBroadcast(&this->_inst);
    
    }
}