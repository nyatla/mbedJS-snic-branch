#pragma once
#include "NyLPC_net.h"
#include "ModBaseClass.h"
#include "HttpdConnection.h"
#include "Httpd.h"

namespace MiMic
{
    class HttpdConnection;

    /**
     * This class is httpd module.
     * The class provides 3 services.
     * <ul>
     * <li>create a blank file.</li>
     * <li>remove a file.</li>
     * <li>update a file.</li>
     * </ul>
     */
    class ModFileIo:ModBaseClass
    {
    public:

        ModFileIo(const char* i_path);
        ModFileIo();
        virtual ~ModFileIo();
        void setParam(const char* i_path);
        bool execute(HttpdConnection& i_connection);
    };
}