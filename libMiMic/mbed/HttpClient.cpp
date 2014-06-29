#include "HttpClient.h"

namespace MiMic
{
    HttpClient::HttpClient()
    {
        this->_private_tcp_rx_buf=malloc(256);
        NyLPC_cHttpClient_initialize(&this->_inst,this->_private_tcp_rx_buf,256);
    }
    HttpClient::~HttpClient()
    {
        NyLPC_cHttpClient_finalize(&this->_inst);
        if(this->_private_tcp_rx_buf!=NULL){
            free(this->_private_tcp_rx_buf);
        }
    }
    bool HttpClient::connect(const IpAddr& i_host,unsigned short i_port)
    {
        return NyLPC_cHttpClient_connect(&this->_inst,&i_host.addr.v4,i_port)?true:false;
    }
    /**
     * This function sends a request to server and prevent to accept status code.
     * Must call getStatus after successful.
     * If request has content body(i_content_length!=0), call writeX function to send request body in before to call getStatus
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
    bool HttpClient::sendMethod(NyLPC_THttpMethodType i_method,const char* i_path,int i_content_length,const char* i_mimetype,const char* i_additional_header)
    {
        return NyLPC_cHttpClient_sendMethod(&this->_inst,i_method,i_path,i_content_length,i_mimetype,i_additional_header)?true:false;
    }
    /**
     * This function returns status code.
     * Must call after the sendMethod was successful.
     * @return
     * Error:0,otherwise HTTP status code.
     */
    int HttpClient::getStatus()
    {
        return NyLPC_cHttpClient_getStatus(&this->_inst);
    }
    /**
     * Close current connection.
     */
    void HttpClient::close()
    {
        NyLPC_cHttpClient_close(&this->_inst);
    }
    /**
     * Read request body from http stream.
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
    bool HttpClient::read(void* i_rx_buf,int i_rx_buf_len,short &i_read_len)
    {
        return NyLPC_cHttpClient_read(&this->_inst,i_rx_buf,i_rx_buf_len,&i_read_len)?true:false;        
    }
    bool HttpClient::write(const void* i_tx_buf,int i_tx_len)
    {
        return NyLPC_cHttpClient_write(&this->_inst,i_tx_buf,i_tx_len)?true:false;
    }

}