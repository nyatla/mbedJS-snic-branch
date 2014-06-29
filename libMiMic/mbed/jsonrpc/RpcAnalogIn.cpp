#include "RpcHandlerBase.h"
namespace MiMic
{
    class AnalogInHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool new1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//u
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            PinName pin;
            if(getParamsAsPin(mod,i_rpc,&pin,1)){
    			addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<AnalogIn>(new AnalogIn(pin)));
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool read_u16(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            AnalogIn* inst=(AnalogIn*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v=inst->read_u16();
				mod->putResult(i_rpc->method.id,"%d",v);
			}
            return NyLPC_TBool_TRUE;
        }
        /** 10000倍したread()の値を返す*/
        static NyLPC_TBool read_fx(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=(ModJsonRpc*)i_param;
            AnalogIn* inst=(AnalogIn*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",(int)(inst->read()*10000));
			}
            return NyLPC_TBool_TRUE;
        }
    };



const static struct NyLPC_TJsonRpcMethodDef func_table[]=
{
    { "_new1"		,"u"    ,AnalogInHandler::new1},
    { "read_u16"	,"d"	,AnalogInHandler::read_u16},
    { "read_fx"	,"d"		,AnalogInHandler::read_fx},
    { NULL      ,NULL   ,NULL}
};

const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_ANALOG_IN={
    "mbedJS","AnalogIn",func_table
};



}
