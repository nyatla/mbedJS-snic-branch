#include "RpcHandlerBase.h"
namespace MiMic
{
    class I2CHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool new1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//u
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PinName pins[2];
            if(getParamsAsPin(mod,i_rpc,pins,2)){
    			addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<I2C>(new I2C(pins[0],pins[1])));
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool frequency(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2C* inst=(I2C*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					inst->frequency(v);
					mod->putResult(i_rpc->method.id);
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool read1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//ddd return dB
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2C* inst=(I2C*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int addr;
				unsigned char repeated,len;
	        	if(getParamInt(mod,i_rpc,addr,1)){
	        		if(getParamByte(mod,i_rpc,len,2)){
		        		if(getParamByte(mod,i_rpc,repeated,3)){
		    				char* data=new char[len];
		    				int r=inst->read(addr,data,len,repeated?true:false);
							mod->putResult(i_rpc->method.id,"%d,\"%.*B\"",r,(int)len,data);
							delete[] data;
		        		}
	        		}
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool read2(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2C* inst=(I2C*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int ack;
	        	if(getParamInt(mod,i_rpc,ack,1)){
					mod->putResult(i_rpc->method.id,"%d",inst->read(ack));
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool write1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//ddBd return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2C* inst=(I2C*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int addr;
				const unsigned char* data;
				unsigned char repeated,len;
	        	if(getParamInt(mod,i_rpc,addr,1)){
	        		if(getParamByteArray(mod,i_rpc,data,len,2)){
		        		if(getParamByte(mod,i_rpc,repeated,3)){
							mod->putResult(i_rpc->method.id,"%d",inst->write(addr,(const char*)data,len,repeated?true:false));
		        		}
	        		}
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool write2(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2C* inst=(I2C*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int ack;
	        	if(getParamInt(mod,i_rpc,ack,1)){
					mod->putResult(i_rpc->method.id,"%d",inst->write(ack));
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool start(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2C* inst=(I2C*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				inst->start();
				mod->putResult(i_rpc->method.id);
			}
        	return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool stop(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2C* inst=(I2C*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				inst->stop();
				mod->putResult(i_rpc->method.id);
			}
        	return NyLPC_TBool_TRUE;
        }
    };


	const static struct NyLPC_TJsonRpcMethodDef func_table[]=
	{
		{ "_new1"		,"uu"  	,I2CHandler::new1},
		{ "frequency"	,"dd"	,I2CHandler::frequency},
		{ "read1"		,"ddbb"	,I2CHandler::read1},
		{ "read2"		,"dd"	,I2CHandler::read2},
		{ "write1"		,"ddBb"	,I2CHandler::write1},
		{ "write2"		,"dd"	,I2CHandler::write2},
		{ "start"		,"d"	,I2CHandler::start},
		{ "stop"		,"d"	,I2CHandler::stop},
		{ NULL      ,NULL   ,NULL}
	};

	const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_I2C={
		"mbedJS","I2C",func_table
	};
}

