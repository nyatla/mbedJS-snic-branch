#include "RpcHandlerBase.h"
namespace MiMic
{
    class BusInOutHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool new1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//uuuuuuuuuuuuuuuu
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PinName pin[16];
            if(getParamsAsPin(mod,i_rpc,pin,16)){
    			addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<BusInOut>(new BusInOut(pin)));
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool read(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            BusInOut* inst=(BusInOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",(int)(inst->read()));
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool write(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            BusInOut* inst=(BusInOut*)getObjectBatch(mod,i_rpc);
            if(inst!=NULL){
            	int v;
				if(getParamInt(mod,i_rpc,v,1)){
					inst->write(v);
					mod->putResult(i_rpc->method.id);
				}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool output(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            BusInOut* inst=(BusInOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				inst->output();
				mod->putResult(i_rpc->method.id);
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool input(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            BusInOut* inst=(BusInOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				inst->input();
				mod->putResult(i_rpc->method.id);
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool mode(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            BusInOut* inst=(BusInOut*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					inst->mode(pinmodeId2PinMode(v));
					mod->putResult(i_rpc->method.id);
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
    };



const static struct NyLPC_TJsonRpcMethodDef func_table[]=
{
    { "_new1"	,"uuuuuuuuuuuuuuuuu"    ,BusInOutHandler::new1},
    { "read"	,"d"	,BusInOutHandler::read},
    { "write"	,"dd"	,BusInOutHandler::write},
    { "output"	,"d"	,BusInOutHandler::output},
    { "input"	,"d"	,BusInOutHandler::input},
    { "mode"	,"dd"	,BusInOutHandler::mode},
    { NULL      ,NULL   ,NULL}
};

const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_BUS_IN_OUT={
    "mbedJS","BusInOut",func_table
};



}
