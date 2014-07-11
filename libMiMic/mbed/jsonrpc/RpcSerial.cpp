#include "RpcHandlerBase.h"
namespace MiMic
{
    class SerialHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool new1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//uu
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PinName pins[2];
            if(getParamsAsPin(mod,i_rpc,pins,2)){
    			addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<Serial>(new Serial(pins[0],pins[1])));
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool format(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dddd return none
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            Serial* inst=(Serial*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int p[3];
	        	if(getParamsInt(mod,i_rpc,p,3,1)){
	        		inst->format(p[0],(SerialBase::Parity)p[1],p[2]);
	        		mod->putResult(i_rpc->method.id);
				}
			}
			return NyLPC_TBool_TRUE;
        };
        static NyLPC_TBool readable(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            Serial* inst=(Serial*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",inst->readable());
			}
			return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool writeable(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            Serial* inst=(Serial*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",inst->writeable());
			}
			return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool send_break(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
         	//d return none
             ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
             Serial* inst=(Serial*)getObjectBatch(mod,i_rpc);
 			if(inst!=NULL){
 				mod->putResult(i_rpc->method.id);
 			}
 			return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool putc(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            Serial* inst=(Serial*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
	        		mod->putResult(i_rpc->method.id,"%d",inst->putc(v));
	        	}
			}
        	return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool puts(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//ds return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            Serial* inst=(Serial*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				const char* s;
	        	if(getParamString(mod,i_rpc,s,1)){
	        		mod->putResult(i_rpc->method.id,"%d",inst->puts(s));
				}
			}
			return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool getc(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            Serial* inst=(Serial*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",inst->getc());
			}
        	return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool gets(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return s
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            Serial* inst=(Serial*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				unsigned char l;
	        	if(getParamByte(mod,i_rpc,l,1)){
	        		char* b=new char[l];
	        		inst->gets(b,l);
	        		mod->putResult(i_rpc->method.id,"\"%s\"",b);
	        		delete[] b;
				}
			}
			return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool baud(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            Serial* inst=(Serial*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
	        		inst->baud(v);
	        		mod->putResult(i_rpc->method.id);
	        	}
			}
        	return NyLPC_TBool_TRUE;
        }
	};



	const static struct NyLPC_TJsonRpcMethodDef func_table[]=
	{
		{ "_new1"		,"uu"  	,SerialHandler::new1},
		{ "format"		,"dddd"	,SerialHandler::format},
		{ "readable"	,"d"	,SerialHandler::readable},
		{ "writeable"	,"d"	,SerialHandler::writeable},
		{ "send_break"	,"d"	,SerialHandler::send_break},
		{ "putc"		,"dd"	,SerialHandler::putc},
		{ "puts"		,"ds"	,SerialHandler::puts},
		{ "getc"		,"d"	,SerialHandler::getc},
		{ "gets"		,"db"	,SerialHandler::gets},
		{ "baud"		,"dd"	,SerialHandler::baud},
		{ NULL      ,NULL   ,NULL}
	};

	const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_SERIAL={
		"mbedJS","Serial",func_table
	};



}
