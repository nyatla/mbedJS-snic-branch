#pragma once
////////////////////////////////////////////////////////////////////////////////
// UdpSocket.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"
#include "IpAddr.h"

namespace MiMic
{
    /**
     * Udp Socket Class.
     * The class is used by Net constructor.
     */
    class UdpSocket
    {
    private:
        NyLPC_TcUdpSocket_t _inst;
        void* _private_rx;
    public:
        /** wrapped base LPC class.*/
        NyLPC_TcUdpSocket_t* refBaseInstance(){return &this->_inst;}
    
    public:
        /**
         * @param i_port
         * port number.
         * @param i_rx_buf_size
         * Size of the receive memory to allocate on heap
         */
        UdpSocket(unsigned short i_port,unsigned short i_rx_buf_size=(unsigned short)512);
        /**
         * @param i_port
         * port number.
         * @param i_rx_buffer
         * allocated memory for receiving. 
         * @param i_rx_buf_size
         * Size of the i_rx_buf
         */
        UdpSocket(unsigned short i_port,void* i_rx_buf,unsigned short i_rx_buf_size);
        /**
         * This constructor accepts "large" packet by asynchronous handler.
         * Must be override "onRxHandler" function.
         */
        UdpSocket(unsigned short i_port,void* i_rx_handler);
        virtual ~UdpSocket();
        /**
         * This function return recieved data and size.
         * The function sets the head of the oldest readable buffer. 
         * A position is not changed until precvnext was called.         
         * @param i_host_addr
         * must be IPv4 address format.
         */
        int precvFrom(const void* &i_rx,IpAddr* i_peer_host=NULL,unsigned short* i_port=NULL);
        int precvFrom(const char* &i_rx,IpAddr* i_peer_host=NULL,unsigned short* i_port=NULL);
        /**
         * This function moves rx buffer to next packet.
         */
        void precvNext(void);
        /**
         * true if precv has data.
         * This can avoid the block of precv.
         */
        bool canRecv();
        
        bool sendTo(const IpAddr& i_host,unsigned short i_port,const void* i_tx,unsigned short i_tx_size);
        void joinMulticast(const IpAddr& i_host);
        void setBroadcast(void);
   };
}

