#pragma once
////////////////////////////////////////////////////////////////////////////////
// ModRomFiles.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"


namespace MiMic
{
    class HttpdConnection;
    /**
     * This class is a module for Httpd.
     * The class parses a request path to buffer from connection.
     */
    class ModUrl
    {
    private:
        int _method_type;
    public:
        const static int METHOD_GET =NyLPC_THttpMethodType_GET;
        const static int METHOD_HEAD=NyLPC_THttpMethodType_HEAD;
        const static int METHOD_POST=NyLPC_THttpMethodType_POST;
    public:
        /**
         * Constructor.
         */
        ModUrl();
        virtual ~ModUrl();

        /**
          * This function processes a request.
          * @param i_connection
          * @param o_url_buf
          * Output parametor.
          * Address of buffer which accept URL string.
          * The string will be "" (zero text) if URL too long or parsing failed. 
          * @param i_buf_len
          * Size of o_url buffer in byte.
          * @param o_method_type
          * Address of variable which accept HTTP-Method type.
          * Can be omitted.
          * @return
          * TRUE if request was processed. otherwise FALSE.
          * The value describes processing status. It is not parsing error status.
          */
        bool execute(HttpdConnection& i_connection,char* o_url_buf,int i_buf_len,int* o_method_type=NULL);
    };

}