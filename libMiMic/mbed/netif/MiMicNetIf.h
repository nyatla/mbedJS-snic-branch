#pragma once
////////////////////////////////////////////////////////////////////////////////
// Httpd.h
////////////////////////////////////////////////////////////////////////////////
#include "NyLPC_net.h"
#include "../netif/mimicip/NyLPC_cMiMicIpNetIf.h"
#include "INetIf.h"
namespace MiMic
{
    class MiMicNetIf : public INetif
    {
    public:
    	const NyLPC_TiNetInterface_Interface* _inst;
    public:
    	MiMicNetIf();
        virtual ~MiMicNetIf();
    public:
        virtual const NyLPC_TiNetInterface_Interface* getInterface();

    };
}
