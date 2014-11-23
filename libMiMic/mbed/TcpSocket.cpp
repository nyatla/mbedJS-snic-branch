////////////////////////////////////////////////////////////////////////////////
// TcpSocket.h
////////////////////////////////////////////////////////////////////////////////

#include "TcpSocket.h"
#include "mbed.h"

namespace MiMic
{
    #define TIMEOUT_IN_MSEC (5*1000)
    
    TcpSocket::TcpSocket()
    {
        this->_inst=NyLPC_cNetIf_createTcpSocketEx(NyLPC_TSocketType_TCP_NORMAL);
    	if(this->_inst==NULL){
    		mbed_die();
    	}
    }
    TcpSocket::~TcpSocket()
    {
        NyLPC_iTcpSocket_finalize(this->_inst);
    }
    bool TcpSocket::connect(const IpAddr& i_addr,unsigned short i_port)
    {
        return NyLPC_iTcpSocket_connect(this->_inst,&(i_addr.addr.v4),i_port,TIMEOUT_IN_MSEC)?true:false;
    }
    
    bool TcpSocket::send(const void* i_tx,unsigned short i_tx_size)
    {
        int l,t;
        l=i_tx_size;
        while(l>0){
            t=NyLPC_iTcpSocket_send(this->_inst,((const char*)i_tx)+(i_tx_size-l),l,TIMEOUT_IN_MSEC);
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
        return NyLPC_iTcpSocket_precv(this->_inst,&rx,0)>0;
    }
    int TcpSocket::precv(const void* &i_rx)
    {
        return NyLPC_iTcpSocket_precv(this->_inst,&i_rx,TIMEOUT_IN_MSEC);
    }
    int TcpSocket::precv(const char* &i_rx)
    {
        return NyLPC_iTcpSocket_precv(this->_inst,(const void**)&i_rx,TIMEOUT_IN_MSEC);
    }
    void TcpSocket::pseek(unsigned short i_rx_seek)
    {
        NyLPC_iTcpSocket_pseek(this->_inst,i_rx_seek);
    }
    void TcpSocket::close()
    {
        return NyLPC_iTcpSocket_close(this->_inst,TIMEOUT_IN_MSEC);
    }
}
