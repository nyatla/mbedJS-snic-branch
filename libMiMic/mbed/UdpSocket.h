#pragma once
////////////////////////////////////////////////////////////////////////////////
// UdpSocket.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"
#include "IpAddr.h"

namespace MiMic
{
	class UdpSocket;
    /**
     * Udp Socket Class.
     * The class is used by Net constructor.
     */
    class UdpSocket
    {
    private:
        NyLPC_TiUdpSocket_t* _inst;
    public:
        /** wrapped base LPC class.*/
        NyLPC_TiUdpSocket_t* refBaseInstance(){return this->_inst;}

    public:
        /**
         * Create standard UDP socket.
         * @param i_port
         * port number.
         * @param i_nobuffer
         * false(default) -
         * UDP packets will be receive to  internal buffer. It can be access by precvFrom/precvNext function.
         * It is accepts only "Short" packet.
         * MUST BE SET NyLPC_cMiMicIpNetIf_config_UDPSOCKET_MAX 1 or more when MiMicIPNetInterface using.
         * true -
         * UDP packets will be handled to onRxHandler function.
         * It is accepts "Full size" packet.
         * MUST BE SET NyLPC_cMiMicIpNetIf_config_UDPSOCKET_NB_MAX 1 or more when MiMicIPNetInterface using.
         */
        UdpSocket(unsigned short i_port,bool i_nobuffer=false);
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
    protected:
        /**
         * callback function.
         * MUST be override when used callback constructor.
         */
        virtual void onRxHandler(const void* i_buf,const struct NyLPC_TIPv4RxInfo* i_info){};
    	static void rxhandler(NyLPC_TiUdpSocket_t* i_inst,const void* i_buf,const struct NyLPC_TIPv4RxInfo* i_info);

   };
}

