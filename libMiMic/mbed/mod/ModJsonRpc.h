#include "NyLPC_net.h"
#include "ModBaseClass.h"
#include "HttpdConnection.h"
#include "Httpd.h"
#include "Net.h"
#include <vector>

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
    class ModJsonRpc:ModBaseClass
    {
    public:
        class BasicRpcObject
        {
        public:
            void* obj;
            BasicRpcObject(void* i_ptr) : obj(i_ptr){};
            virtual ~BasicRpcObject(){}
        };
        template< class T > class RpcObject : public BasicRpcObject{
        public:
            RpcObject(T* i_ptr) : BasicRpcObject(i_ptr){};
            virtual ~RpcObject(){delete (static_cast<T*>(this->obj));}
        };
        template< class T > class RpcArray : public BasicRpcObject{
        public:
            RpcArray(T* i_ptr) : BasicRpcObject(i_ptr){};
            virtual ~RpcArray(){delete[] (static_cast<T*>(this->obj));}
        };
    private:
    public:
        const static int PARSE_ERROR=NyLPC_TJsonRpcErrorCode_PARSE_ERROR;
        const static int INVALID_REQUEST=NyLPC_TJsonRpcErrorCode_INVALID_REQUEST;
        const static int METHOD_NOT_FOUND=NyLPC_TJsonRpcErrorCode_METHOD_NOT_FOUND;
        const static int INVALID_PARAMS=NyLPC_TJsonRpcErrorCode_INVALID_PARAMS;
        const static int INTERNAL_ERROR=NyLPC_TJsonRpcErrorCode_INTERNAL_ERROR;
        const static int SERVER_ERROR_BASE=NyLPC_TJsonRpcErrorCode_SERVER_ERROR_BASE;
    private:
        BasicRpcObject** _objects;
        const struct NyLPC_TJsonRpcClassDef** _rpc_table;
    protected:
        NyLPC_TcModJsonRpc_t* _mod;    
    public:
        ModJsonRpc();
        /**
         * @param i_rpc_table
         * An address of Json RPC functions table.
         */
        ModJsonRpc(const char* i_path,const struct NyLPC_TJsonRpcClassDef** i_rpc_table);
        virtual ~ModJsonRpc();
        bool isStarted();
        void setParam(const char* i_path,const struct NyLPC_TJsonRpcClassDef** i_rpc_table);        
        /**
         * This function prepares Json rpc loop with websocket negotiation.
         * @return
         * true if successful;otherwishe false.
         */
        bool execute(HttpdConnection& i_connection);
        void dispatchRpc();
        
    public:
        //for development
        int addObject(BasicRpcObject* i_object);
        void* getObject(int i_oid);        
        bool putResult(unsigned int i_id,const char* i_params_fmt,...);
        bool putError(unsigned int i_id,int i_code);
    };
}
