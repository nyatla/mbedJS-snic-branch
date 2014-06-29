#pragma once
////////////////////////////////////////////////////////////////////////////////
// ModBaseClass.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"


namespace MiMic
{
    class HttpdConnection;

    class ModBaseClass
    {
    protected:
        char* _path;
    public:
        /**
         * @param i_path
         * target path
         * <pre>
         * ex.setParam("root")
         * </pre>
         */
        ModBaseClass(const char* i_path);
        ModBaseClass();
        virtual ~ModBaseClass();
    protected:
        /**
         * @param i_path
         * target path
         * <pre>
         * ex.setParam("")
         * </pre>
         */
        void setParam(const char* i_path);
        virtual bool canHandle(HttpdConnection& i_connection);
    };
}