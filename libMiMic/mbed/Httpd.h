#pragma once
////////////////////////////////////////////////////////////////////////////////
// Httpd.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"

namespace MiMic
{
    class HttpdConnection;
    class Httpd
    {
    private:
        struct Httpd2{
            NyLPC_TcHttpd_t super;
            Httpd* _parent;
        }_inst;
        static void onRequestHandler(NyLPC_TcHttpdConnection_t* i_connection);
        static int taskHandler(void* i_param);
    public:
        /**
         * This function create an instance with service port number.
         * @param i_port_number
         * HTTP service port number.
         */
        Httpd(int i_port_number);
        virtual ~Httpd();
        /**
         * This function starts HTTP listen loop on current task.
         * The function never return.
         * Must not use after called loopTask function.
         */
        void loop();
        /**
         * This function starts HTTP listen loop on a new task.
         */
        void loopTask();
        /**
         * This function gets a lock.
         * This function is used for exclusive process in request handler or other task.
         */
        void lock();
        /**
         * This function releases a lock.
         */
        void unlock();
        /**
         * The handler function for HTTP request.
         * Must implement the function in extended class.
         * This is called when HTTPD received HTTP request.
         * The function may be call in multiple.
         * @param i_connection
         * HTTP connection object that contain "prefetched" HTTP stream.
         */
        virtual void onRequest(HttpdConnection& i_connection)=0;
    public:
        const static int SIZE_OF_HTTP_BUF=512;
        /**
         * This buffer is a shared buffer for HTTPD modules.
         * It will be use for temporary buffer or work memory.
         * Must lock before using.
         */
        static char _shared_buf[SIZE_OF_HTTP_BUF];
    };
}
