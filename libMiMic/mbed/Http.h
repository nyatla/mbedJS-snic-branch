#pragma once
////////////////////////////////////////////////////////////////////////////////
// Httpd.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"

namespace MiMic
{
    class Http
    {
    public:
        const static int MT_GET =NyLPC_THttpMethodType_GET;
        const static int MT_POST=NyLPC_THttpMethodType_POST;
        const static int MT_HEAD=NyLPC_THttpMethodType_HEAD;
    };
}  