#pragma once
////////////////////////////////////////////////////////////////////////////////
// TcpSocket.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"
#include "IpAddr.h"

namespace MiMic
{
    /**
     * Tcp Socket Class.
     * The class is used by Net constructor.
     */
    class TcpSocket
    {
    private:
        NyLPC_TcTcpSocket_t _inst;
        void* _private_rx;
    public:
        /** wrapped base LPC class.*/
        NyLPC_TcTcpSocket_t* refBaseInstance(){return &this->_inst;}
    
    public:
        TcpSocket(unsigned short i_rx_buf_size=(unsigned short)512);
        TcpSocket(void* i_rx_buf,unsigned short i_rx_buf_size);
        virtual ~TcpSocket();
        /**
         * @param i_host_addr
         * must be IPv4 address format.
         */
        bool connect(const IpAddr& i_addr,unsigned short i_port);
        bool send(const void* i_tx,unsigned short i_tx_size);
        /**
         * This function return recieved data and size.
         * The function sets the head of the readable buffer which can always be read. 
         * A position is not changed until pseek was called.         
         * @param i_rx
         * address of variable which accepts received data pointer.
         * @retrun
         * n<-1 Error
         * n==0 Timeout (connection still established)
         * n>0  Success. readable data size in i_rx.
         */
        int precv(const void* &i_rx);
        int precv(const char* &i_rx);
        /**
         * true if precv has data.
         * This can avoid the block of precv.
         */
        bool canRecv();

        /**
         * This function seek rx pointer to next.
         * @param i_rx_size
         * seek size. Must be returned value which is small or equal by the precv.
         */
        void pseek(unsigned short i_rx_seek);
        void close();
   };

}
