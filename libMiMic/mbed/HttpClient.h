#pragma once
////////////////////////////////////////////////////////////////////////////////
// HttpClient.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"
#include "IpAddr.h"

namespace MiMic
{
    class HttpClient
    {
    private:
        void* _private_tcp_rx_buf;
    protected:
        NyLPC_TcHttpClient_t _inst;
    public:
        const static NyLPC_THttpMethodType HTTP_GET=NyLPC_THttpMethodType_GET;
        const static NyLPC_THttpMethodType HTTP_POST=NyLPC_THttpMethodType_POST;
        const static NyLPC_THttpMethodType HTTP_HEAD=NyLPC_THttpMethodType_HEAD;
        const static unsigned int CONTENT_CHUNKED=NyLPC_cHttpHeaderWriter_CONTENT_LENGTH_UNLIMITED;
        
    public:
        HttpClient();
        virtual ~HttpClient();
    public:
        bool connect(const IpAddr& i_host,unsigned short i_port);
        /**
         * This function sends a request to server and prevent to accept status code.
         * Must call getStatus after successful.
         * If request has content body(i_content_length!=0), call writeX function to send request body in before to call getStatus
         * @param i_content_length
         * size of request body.
         * Specify CONTENT_CHUNKED if the size is unknown.
         * @return
         * true if successful,
         * otherwise error. Connection is closed.
         * @example
         * <code>
         * //GET
         * </code>
         * <code>
         * //POST
         *
         * </code>
         */
        bool sendMethod(NyLPC_THttpMethodType i_method,const char* i_path,int i_content_length=0,const char* i_mimetype=NULL,const char* i_additional_header=NULL);
        /**
         * This function returns status code.
         * Must call after the sendMethod was successful.
         * @return
         * Error:0,otherwise HTTP status code.
         */
        int getStatus();
        /**
         * Close current connection.
         */
        void close();
        /**
         * Read request body from http stream.
         * This function must be call repeatedly until the end of the stream or an error.
         * @param i_rx_buf
         * A buffer which accepts received data.
         * @param i_rx_buf_len
         * size of i_rx_buf in byte. 
         * @param i_read_len
         * pointer to variable which accept received data size in byte.
         * It is enabled in retrurn true. 
         * n>0 is datasize. n==0 is end of stream.
         * @return
         * true if successful,otherwise false
         */
        bool read(void* i_rx_buf,int i_rx_buf_len,short &i_read_len);
        /**
         * Write request body to connected http stream.
         * This function must be call repeatedly until the end of the request content or an error.
         * Transmission of the request body is completed by to call the AAA in the case of chunked transfer.
         * @param i_tx_buf
         * @param i_tx_len
         * @return
         * true if successful.
         */
        bool write(const void* i_tx_buf,int i_tx_len);
    };
}
