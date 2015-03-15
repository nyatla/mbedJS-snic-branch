#pragma once
////////////////////////////////////////////////////////////////////////////////
// Net.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"
#include "netif/INetIf.h"

namespace MiMic
{
    class NetConfig;
    class Net
    {
    public:
        /**
         * Path of upnp services on HTTP server.
         */
        const static char* UPNP_ROOT_PATH;
    private:
        NyLPC_TcMDnsServer_t* _mdns;
        NyLPC_TcUPnP_t* _upnp;
    public:
        /**
         * The constructor.
         * Must be call after the RTOS started.
         */
        Net(INetif& i_netif);
        virtual ~Net();
        /**
         * This function starts networking with configulation. 
         * @param i_cfg
         * configuration parameter.
         * Must be hold until instance is freed.
         * This may be changed by initializer it has DHCP or AUTOIP flag.
         */
        void start(NetConfig& i_cfg);
        void stop();
    public:
        /**
         * UPnP instance
         */
        const NyLPC_TcUPnP_t* refUPnPInstance()const{return this->_upnp;}
    };
}
