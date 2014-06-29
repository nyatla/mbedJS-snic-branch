#pragma once
////////////////////////////////////////////////////////////////////////////////
// HttpdConnection.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"
namespace MiMic
{
    class HttpdConnection
    {
    public:
        NyLPC_TcHttpdConnection* _ref_inst;
        HttpdConnection(NyLPC_TcHttpdConnection* i_ref_inst);
        /**
         * This function returns HTTP request type.
         * See Http::MT_xxx constant value.
         */
        int getMethodType();

        /**
         * This function checks Method type if equal with i_method_type.
         */        
        bool isMethodType(int i_method_type);

        /**
         * This function send HTTP response header to connection.
         * The function is useful for unknown length content. 
         * @param i_additional_header
         * Additional header text.
         * The text which was divided in CRLF and closed by CRLF. 
         */
        bool sendHeader(unsigned short i_status_code,const char* i_content_type,const char* i_additional_header);

        /**
         * This function send HTTP response header to connection.
         * The function is useful for known length content. 
         */
        bool sendHeader(unsigned short i_status_code,const char* i_content_type,const char* i_additional_header,unsigned int i_length);
        /**
         * This function send HTTP error response with empty body.
         */
        bool sendError(unsigned short i_status_code);
        
        /**
         * This function send formated text to response.
         * The function can be repeatedly called until the end of contents.
         */
        bool sendBody(const void* i_data,NyLPC_TUInt32 i_size);

        /**
         * The function send formated text to response.
         * The function can be repeatedly called until the end of contents.
         * @param i_fmt
         * printf like string
         * %% - '%' charactor ,%s - null terminated string ,%d - 32bit signed integer,%x - 32bit intager ,%u - 32bit unsigned integer ,%c - a charactor
         * @param ...
         * 
         */
        bool sendBodyF(const char* i_fmt,...);
        /**
         * The function gets httpd lock.
         * This lock is the only lock in the Httpd.
         * After call the function, must call unlock function by end of handler.
         */
        void lockHttpd();
        /**
         * The function releases the httpd lock.
         */
        void unlockHttpd();
        /**
         * The function breaks the persist connection if it enabled.
         */
        void breakPersistentConnection();

    };
}