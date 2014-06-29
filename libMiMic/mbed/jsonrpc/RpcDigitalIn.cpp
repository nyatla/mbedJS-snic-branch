#include "RpcHandlerBase.h"

namespace MiMic
{
    class DigitalInHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool new1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//u
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            PinName pin;
            if(getParamsAsPin(mod,i_rpc,&pin,1)){
    			addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<DigitalIn>(new DigitalIn(pin)));
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool read(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            DigitalIn* inst=(DigitalIn*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",(int)(inst->read()));
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool mode(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            DigitalIn* inst=(DigitalIn*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					inst->mode(pinmodeId2PinMode(v));
					mod->putResult(i_rpc->method.id,"");
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
    };



const static struct NyLPC_TJsonRpcMethodDef func_table[]=
{
    { "_new1"   ,"u"    ,DigitalInHandler::new1},
    { "read"    ,"d"    ,DigitalInHandler::read},
    { "mode"   ,"dd"	,DigitalInHandler::mode},
    { NULL      ,NULL   ,NULL}
};

const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_DIGITAL_IN={
    "mbedJS","DigitalIn",func_table
};



}
