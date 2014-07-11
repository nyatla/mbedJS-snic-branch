#include "RpcHandlerBase.h"
namespace MiMic
{
    class SPIHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool new1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//uuuu
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PinName pin[4];
            if(getParamsAsPin(mod,i_rpc,pin,4)){
    			addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<SPI>(new SPI(pin[0],pin[1],pin[2],pin[3])));
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool format(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//ddd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            SPI* inst=(SPI*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int p[2];
	        	if(getParamsInt(mod,i_rpc,p,2,1)){
	        		inst->format(p[0],p[1]);
					mod->putResult(i_rpc->method.id);

	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool frequency(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            SPI* inst=(SPI*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					inst->frequency(v);
					mod->putResult(i_rpc->method.id);
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool write(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            SPI* inst=(SPI*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					mod->putResult(i_rpc->method.id,"%d",(int)(inst->write(v)));
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
    };



const static struct NyLPC_TJsonRpcMethodDef func_table[]=
{
    { "_new1"		,"uuuu"	,SPIHandler::new1},
    { "format"		,"ddd"	,SPIHandler::format},
    { "frequency"	,"dd"	,SPIHandler::frequency},
    { "write"		,"dd"	,SPIHandler::write},
    { NULL      ,NULL   ,NULL}
};

const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_SPI={
    "mbedJS","SPI",func_table
};



}
