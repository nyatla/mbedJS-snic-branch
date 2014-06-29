#include "RpcHandlerBase.h"
namespace MiMic
{
    class PortInHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool new1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//ud
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            unsigned int port;
            int mask;
            if(getParamUInt(mod,i_rpc,port,0)){
                if(getParamInt(mod,i_rpc,mask,1)){
                 	addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<PortIn>(new PortIn(portId2PortName(port),mask)));
                }
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool read(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            PortIn* inst=(PortIn*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",(int)(inst->read()));
			}
            return NyLPC_TBool_TRUE;
        }
    };



const static struct NyLPC_TJsonRpcMethodDef func_table[]=
{
    { "_new1"	,"ud"   ,PortInHandler::new1},
    { "read"	,"d"	,PortInHandler::read},
    { NULL      ,NULL   ,NULL}
};

const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_PORT_IN={
    "mbedJS","PortIn",func_table
};



}
