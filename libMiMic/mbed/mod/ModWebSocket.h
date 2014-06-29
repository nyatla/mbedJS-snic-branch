#pragma once
#include "NyLPC_net.h"
#include "ModBaseClass.h"
#include "HttpdConnection.h"
#include "Httpd.h"
#include "Net.h"

namespace MiMic
{
    class HttpdConnection;

    /**
     * This class is Websocket module.
     * The class provides 3 services.
     * <ul>
     * <li>d.xml - a device description.</li>
     * <li>control/xx - soap handler</li>
     * <li>event/xx -event handler.</li>
     * </ul>
     */
    class ModWebSocket:ModBaseClass
    {
    private:
    protected:
        NyLPC_TcModWebSocket_t* _mod;
    public:
        ModWebSocket();
        ModWebSocket(const char* i_path);
        virtual ~ModWebSocket();
        void setParam(const char* i_path);
        bool isStarted();
        /**
         * This function executes websocket negotiation to the current connection. 
         * Should be handle websocket session if function successful.
         * @return
         * true if negotiation successful;otherwishe false.
         */
        bool execute(HttpdConnection& i_connection);
        /**
         * Write data to websocket stream.
         * @return
         * true if successful;otherwishe false and connection closed.
         */
        bool write(const void* i_tx_buf,int i_tx_size);
        /**
         * This function sends data to websocket stream.
         * @param i_fmt
         * printf like format text.
         * @param ...
         * argument list for i_fmt
         */
        bool writeFormat(const char* i_fmt,...);
        
        /**
         * This function receives data from websocket stream.
         * @return
         * <ul>
         * <li>r<0  Error. The socket already closed.
         * <li>r==0 Timeout. The connection is continued.
         * <li>r>0  Success. Received data size.
         * </ul>
         * If an Error found, application should be call close and exit handler.
         */
        int read(void* i_rx_buf,int i_rx_size);
        /**
         * This function terminates websocket connection.
         * Should be called if execute function successful.
         */
        void close();
        /**
         * This function returns read function status.
         * This is to confirm that "read" can call without blocking.
         * @return
         * true if can be call "read" with no-wait. otherwise, "read" might be wait.
         */
        bool canRead();


    };
}