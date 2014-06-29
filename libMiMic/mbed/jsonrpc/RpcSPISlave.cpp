#include "RpcHandlerBase.h"
namespace MiMic
{
    class SPISlaveHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool new1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//uuuu
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            PinName pin[4];
            if(getParamsAsPin(mod,i_rpc,pin,4)){
    			addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<SPISlave>(new SPISlave(pin[0],pin[1],pin[2],pin[3])));
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool format(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//ddd return void
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            SPISlave* inst=(SPISlave*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int p[2];
	        	if(getParamsInt(mod,i_rpc,p,2,1)){
	        		inst->format(p[0],p[1]);
					mod->putResult(i_rpc->method.id,"");
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool frequency(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            SPISlave* inst=(SPISlave*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					inst->frequency(v);
					mod->putResult(i_rpc->method.id,"");
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool read(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return d
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            SPISlave* inst=(SPISlave*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",(int)(inst->read()));
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool receive(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return d
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            SPISlave* inst=(SPISlave*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",(int)(inst->receive()));
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool reply(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return d
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            SPISlave* inst=(SPISlave*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
				if(getParamInt(mod,i_rpc,v,1)){
					inst->reply(v);
					mod->putResult(i_rpc->method.id,"");
				}
			}
            return NyLPC_TBool_TRUE;
        }
    };



const static struct NyLPC_TJsonRpcMethodDef func_table[]=
{
    { "_new1"		,"uuuu"	,SPISlaveHandler::new1},
    { "format"		,"ddd"	,SPISlaveHandler::format},
    { "frequency"	,"dd"	,SPISlaveHandler::frequency},
    { "read"		,"d"	,SPISlaveHandler::read},
    { "receive"		,"d"	,SPISlaveHandler::receive},
    { "reply"		,"dd"	,SPISlaveHandler::reply},
    { NULL      ,NULL   ,NULL}
};

const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_SPI_SLAVE={
    "mbedJS","SPISlave",func_table
};



}
