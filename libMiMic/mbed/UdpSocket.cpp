////////////////////////////////////////////////////////////////////////////////
// UdpSocket.cpp
////////////////////////////////////////////////////////////////////////////////

#include "UdpSocket.h"
#include "mbed.h"



namespace MiMic
{
    #define TIMEOUT_IN_MSEC (2*1000)

	void UdpSocket::rxhandler(NyLPC_TiUdpSocket_t* i_inst,const void* i_buf,const struct NyLPC_TIPv4RxInfo* i_info)
	{
		UdpSocket* sock=(UdpSocket*)i_inst->_tag;
		sock->onRxHandler(i_buf,i_info);
	}

    UdpSocket::UdpSocket(unsigned short i_port,bool i_nobuffer)
    {
    	if(i_nobuffer){
        	this->_inst=NyLPC_cNet_createUdpSocketEx(i_port,NyLPC_TSocketType_UDP_NOBUF);
    	}else{
    		this->_inst=NyLPC_cNet_createUdpSocketEx(i_port,NyLPC_TSocketType_UDP_NORMAL);
    	}
    	if(this->_inst==NULL){
    		mbed_die();
    	}
    }
    UdpSocket::~UdpSocket()
    {
        NyLPC_iUdpSocket_finalize(this->_inst);
    }
    bool UdpSocket::canRecv()
    {
        const void* rx;
        const struct NyLPC_TIPv4RxInfo* info;
        return NyLPC_iUdpSocket_precv(this->_inst,&rx,&info,TIMEOUT_IN_MSEC)>0;
    }

    int UdpSocket::precvFrom(const void* &i_rx,IpAddr* i_peer_host,unsigned short* i_port)
    {
        const struct NyLPC_TIPv4RxInfo* info;
        int rs=NyLPC_iUdpSocket_precv(this->_inst,&i_rx,&info,TIMEOUT_IN_MSEC);
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
        int rs=NyLPC_iUdpSocket_precv(this->_inst,(const void**)&i_rx,&info,TIMEOUT_IN_MSEC);
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
        NyLPC_iUdpSocket_pseek(this->_inst);
    }
    
    bool UdpSocket::sendTo(const IpAddr& i_host,unsigned short i_port,const void* i_tx,unsigned short i_tx_size)
    {
        int r=NyLPC_iUdpSocket_send(this->_inst,&i_host.addr.v4,i_port,i_tx,i_tx_size,TIMEOUT_IN_MSEC);
        return (r==i_tx_size);
    
    }
    
    void UdpSocket::joinMulticast(const IpAddr& i_host)
    {
        NyLPC_iUdpSocket_joinMulticast(this->_inst,&i_host.addr.v4);
    }
    void UdpSocket::setBroadcast(void)
    {
        NyLPC_iUdpSocket_setBroadcast(this->_inst);
    
    }
}
