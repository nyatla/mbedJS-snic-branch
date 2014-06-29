#pragma once
////////////////////////////////////////////////////////////////////////////////
// ModRemoteMcu.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"
#include "ModBaseClass.h"


namespace MiMic
{
    class HttpdConnection;
    /**
     * This class is a module for Httpd.
     * The class provides an REST-API to execute MiMic ByteCode.
     * The class is wrapper of NyLPC_TcModRemoteMcu class.
     */
    class ModRemoteMcu:ModBaseClass
    {
    public:
        /**
         * Constructor with parameter initialization.
         */
        ModRemoteMcu(const char* i_path);
        /**
         * Default constructor.
         * Must be call {@link setParam} function after constructed.
         */
        ModRemoteMcu();
        virtual ~ModRemoteMcu();
        void setParam(const char* i_path);
        /**
          * This function processes a request. 
          * The function checks whether a connection has a target request.
          * If necessary, it will transmit a response.
          * @return
          * TRUE if request was processed. otherwise FALSE.
          */
        bool execute(HttpdConnection& i_connection);
    };

}