#include "RpcHandlerBase.h"

namespace MiMic
{
    class DigitalOutHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool new1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
            //u
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PinName pin;
            if(getParamsAsPin(mod,i_rpc,&pin,1)){
                addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<DigitalOut>(new DigitalOut(pin)));
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool new2(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
            //ud return iid
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PinName pin;
            if(getParamsAsPin(mod,i_rpc,&pin,1)){
                int v;
                if(getParamInt(mod,i_rpc,v,1)){
                    addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<DigitalOut>(new DigitalOut(pin,v)));
                }
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool write(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
            //dd return void
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            DigitalOut* inst=(DigitalOut*)getObjectBatch(mod,i_rpc);
            if(inst!=NULL){
                int v;
                if(getParamInt(mod,i_rpc,v,1)){
                    inst->write(v);
                    mod->putResult(i_rpc->method.id);
                }
            }
            return NyLPC_TBool_TRUE;            
        }
        static NyLPC_TBool read(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
            //d return d
        	ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            DigitalOut* inst=(DigitalOut*)getObjectBatch(mod,i_rpc);
            if(inst!=NULL){
                mod->putResult(i_rpc->method.id,"%d",(int)(inst->read()));
            }
            return NyLPC_TBool_TRUE;
        }
    };
    
    
    
const static struct NyLPC_TJsonRpcMethodDef func_table[]=
{
    { "_new1"   ,"u"    ,DigitalOutHandler::new1},
    { "_new2"   ,"ud"   ,DigitalOutHandler::new2},
    { "write"   ,"dd"   ,DigitalOutHandler::write},
    { "read"    ,"d"    ,DigitalOutHandler::read},
    { NULL      ,NULL   ,NULL}
};

const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_DIGITAL_OUT={
    "mbedJS","DigitalOut",func_table
};



}
