#pragma once
////////////////////////////////////////////////////////////////////////////////
// ModMiMicSetting.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"
#include "ModBaseClass.h"


namespace MiMic
{
    class HttpdConnection;
    /**
     * This class is a module for Httpd.
     * The class provides an REST-API to set and get the on-chip configuration MiMicHttpd.
     * The class is wrapper of NyLPC_TcModMiMicSetting class.
     */
    class ModMiMicSetting :ModBaseClass
    {
    public:
        /**
         * Constructor with parameter initialization.
         */
        ModMiMicSetting(const char* i_path);
        /**
         * Default constructor.
         * Must be call {@link setParam} function after constructed.
         */
        ModMiMicSetting();
        virtual ~ModMiMicSetting();
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