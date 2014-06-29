#include "RpcHandlerBase.h"
namespace MiMic
{
    class PortOutHandler :RpcHandlerBase
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
                 	addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<PortOut>(new PortOut(portId2PortName(port),mask)));
                }
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool read(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            PortOut* inst=(PortOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",(int)(inst->read()));
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool write(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            PortOut* inst=(PortOut*)getObjectBatch(mod,i_rpc);
            if(inst!=NULL){
            	int v;
				if(getParamInt(mod,i_rpc,v,1)){
					inst->write(v);
					mod->putResult(i_rpc->method.id,"");
				}
			}
            return NyLPC_TBool_TRUE;
        }
    };



const static struct NyLPC_TJsonRpcMethodDef func_table[]=
{
    { "_new1"	,"ud"   ,PortOutHandler::new1},
    { "read"	,"d"	,PortOutHandler::read},
    { "write"	,"dd"	,PortOutHandler::write},
    { NULL      ,NULL   ,NULL}
};

const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_PORT_OUT={
    "mbedJS","PortOut",func_table
};



}
