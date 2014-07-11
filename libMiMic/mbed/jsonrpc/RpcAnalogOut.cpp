#include "RpcHandlerBase.h"
namespace MiMic
{
    class AnalogOutHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool new1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//u
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PinName pin;
            if(getParamsAsPin(mod,i_rpc,&pin,1)){
    			addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<AnalogOut>(new AnalogOut(pin)));
            }
            return NyLPC_TBool_TRUE;
        }
        /** 10000倍したread()の値を返す*/
        static NyLPC_TBool read_fx(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            AnalogOut* inst=(AnalogOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",(int)(inst->read()*10000));
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool write_fx(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            AnalogOut* inst=(AnalogOut*)getObjectBatch(mod,i_rpc);
            if(inst!=NULL){
            	int v;
				if(getParamInt(mod,i_rpc,v,1)){
					inst->write((float)v/10000.0f);
					mod->putResult(i_rpc->method.id);
				}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool write_u16(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            AnalogOut* inst=(AnalogOut*)getObjectBatch(mod,i_rpc);
            if(inst!=NULL){
            	unsigned int v;
				if(getParamUInt(mod,i_rpc,v,1)){
					inst->write((unsigned int)v);
					mod->putResult(i_rpc->method.id);
				}
			}
            return NyLPC_TBool_TRUE;
        }
    };



const static struct NyLPC_TJsonRpcMethodDef func_table[]=
{
    { "_new1"		,"u"    ,AnalogOutHandler::new1},
    { "read_fx"		,"d"	,AnalogOutHandler::read_fx},
    { "write_fx"	,"dd"	,AnalogOutHandler::write_fx},
    { "write_u16"	,"du"	,AnalogOutHandler::write_u16},
    { NULL      ,NULL   ,NULL}
};

const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_ANALOG_OUT={
    "mbedJS","AnalogOut",func_table
};



}
