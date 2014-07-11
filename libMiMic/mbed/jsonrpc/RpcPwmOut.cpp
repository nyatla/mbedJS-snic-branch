#include "RpcHandlerBase.h"

namespace MiMic
{
    class PwmOutHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool new1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//u
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PinName pin;
            if(getParamsAsPin(mod,i_rpc,&pin,1)){
    			addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<PwmOut>(new PwmOut(pin)));
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool write_fx(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PwmOut* inst=(PwmOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					inst->write((float)v/10000.f);
					mod->putResult(i_rpc->method.id);
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool read_fx(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PwmOut* inst=(PwmOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",(int)(inst->read()*10000));
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool period_fx(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PwmOut* inst=(PwmOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					inst->period((float)v/10000.f);
					mod->putResult(i_rpc->method.id);
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool period_ms(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PwmOut* inst=(PwmOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					inst->period_ms(v);
					mod->putResult(i_rpc->method.id);
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool period_us(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PwmOut* inst=(PwmOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					inst->period_us(v);
					mod->putResult(i_rpc->method.id);
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool pulsewidth_fx(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PwmOut* inst=(PwmOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					inst->pulsewidth((float)v/10000.0f);
					mod->putResult(i_rpc->method.id);
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool pulsewidth_ms(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PwmOut* inst=(PwmOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					inst->pulsewidth_ms(v);
					mod->putResult(i_rpc->method.id);
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool pulsewidth_us(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PwmOut* inst=(PwmOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					inst->pulsewidth_us(v);
					mod->putResult(i_rpc->method.id);
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
    };



const static struct NyLPC_TJsonRpcMethodDef func_table[]=
{
    { "_new1"   		,"u"    ,PwmOutHandler::new1},
    { "write_fx"		,"dd"   ,PwmOutHandler::write_fx},
    { "read_fx"      	,"d"    ,PwmOutHandler::read_fx},
    { "period_fx"    	,"dd"   ,PwmOutHandler::period_fx},
    { "period_ms"		,"dd"   ,PwmOutHandler::period_ms},
    { "period_us"		,"dd"	,PwmOutHandler::period_us},
    { "pulsewidth_fx"	,"dd"   ,PwmOutHandler::pulsewidth_fx},
    { "pulsewidth_ms"	,"dd"   ,PwmOutHandler::pulsewidth_ms},
    { "pulsewidth_us"	,"dd"	,PwmOutHandler::pulsewidth_us},
    { NULL      ,NULL   ,NULL}
};

const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_PWM_OUT={
    "mbedJS","PwmOut",func_table
};



}
