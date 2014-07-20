#include "RpcHandlerBase.h"
namespace MiMic
{
    class PortOutHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool new1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//uu
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            unsigned int port;
            unsigned int mask;
            if(getParamUInt(mod,i_rpc,port,0)){
                if(getParamUInt(mod,i_rpc,mask,1)){
                 	addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<PortOut>(new PortOut(portId2PortName(port),(int)mask)));
                }
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool read(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PortOut* inst=(PortOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",(int)(inst->read()));
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool write(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PortOut* inst=(PortOut*)getObjectBatch(mod,i_rpc);
            if(inst!=NULL){
            	unsigned int v;
				if(getParamUInt(mod,i_rpc,v,1)){
					inst->write((int)v);
					mod->putResult(i_rpc->method.id);
				}
			}
            return NyLPC_TBool_TRUE;
        }
    };



const static struct NyLPC_TJsonRpcMethodDef func_table[]=
{
    { "_new1"	,"uu"   ,PortOutHandler::new1},
    { "read"	,"d"	,PortOutHandler::read},
    { "write"	,"du"	,PortOutHandler::write},
    { NULL      ,NULL   ,NULL}
};

const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_PORT_OUT={
    "mbedJS","PortOut",func_table
};



}
