#pragma once
////////////////////////////////////////////////////////////////////////////////
// TcpSocket.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"

namespace MiMic
{
    /**
     * This class hold IP address.
     */
    class IpAddr
    {
    public:
        union TAddrs{
            struct NyLPC_TIPv4Addr v4;
        }addr;
    public:
        IpAddr()
        {this->setIPv4(0,0,0,0);}  
        IpAddr(unsigned char p4,unsigned char p3,unsigned char p2,unsigned char p1)
        {this->setIPv4(p4,p3,p2,p1);}
        
        void setIPv4(unsigned char p4,unsigned char p3,unsigned char p2,unsigned char p1)
        {NyLPC_TIPv4Addr_set(&this->addr.v4,p4,p3,p2,p1);}        
        void setIPv4(const struct NyLPC_TIPv4Addr& v4)
        {this->addr.v4=v4;}
    };
}