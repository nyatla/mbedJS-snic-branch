#include "RpcHandlerBase.h"
namespace MiMic
{
    class I2CSlaveHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool new1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//u
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            PinName pins[2];
            if(getParamsAsPin(mod,i_rpc,pins,2)){
    			addNewObjectBatch(mod,i_rpc->method.id,new ModJsonRpc::RpcObject<I2CSlave>(new I2CSlave(pins[0],pins[1])));
            }
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool frequency(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2CSlave* inst=(I2CSlave*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int v;
	        	if(getParamInt(mod,i_rpc,v,1)){
					inst->frequency(v);
					mod->putResult(i_rpc->method.id);
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool receive(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2CSlave* inst=(I2CSlave*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",inst->receive());
			}
        	return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool read1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//db return dB
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2CSlave* inst=(I2CSlave*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				unsigned char len;
				if(getParamByte(mod,i_rpc,len,1)){
					char* data=new char[len];
					int r=inst->read(data,len);
					mod->putResult(i_rpc->method.id,"%d,\"%.*B\"",r,(int)len,data);
					delete[] data;
				}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool read2(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2CSlave* inst=(I2CSlave*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				mod->putResult(i_rpc->method.id,"%d",inst->read());
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool write1(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//ddBd return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2CSlave* inst=(I2CSlave*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				const unsigned char* data;
				unsigned char len;
				if(getParamByteArray(mod,i_rpc,data,len,1)){
					mod->putResult(i_rpc->method.id,"%d",inst->write((const char*)data,len));
				}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool write2(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2CSlave* inst=(I2CSlave*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int data;
	        	if(getParamInt(mod,i_rpc,data,1)){
					mod->putResult(i_rpc->method.id,"%d",inst->write(data));
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool address(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//dd return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2CSlave* inst=(I2CSlave*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				int addr;
	        	if(getParamInt(mod,i_rpc,addr,1)){
	        		inst->address(addr);
					mod->putResult(i_rpc->method.id);
	        	}
			}
            return NyLPC_TBool_TRUE;
        }
        static NyLPC_TBool stop(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            I2CSlave* inst=(I2CSlave*)getObjectBatch(mod,i_rpc);
			if(inst!=NULL){
				inst->stop();
				mod->putResult(i_rpc->method.id);
			}
        	return NyLPC_TBool_TRUE;
        }
    };


	const static struct NyLPC_TJsonRpcMethodDef func_table[]=
	{
		{ "_new1"		,"uu"  	,I2CSlaveHandler::new1},
		{ "frequency"	,"dd"	,I2CSlaveHandler::frequency},
		{ "receive"		,"d"	,I2CSlaveHandler::receive},
		{ "read1"		,"db"	,I2CSlaveHandler::read1},
		{ "read2"		,"d"	,I2CSlaveHandler::read2},
		{ "write1"		,"dB"	,I2CSlaveHandler::write1},
		{ "write2"		,"dd"	,I2CSlaveHandler::write2},
		{ "address"		,"dd"	,I2CSlaveHandler::address},
		{ "stop"		,"d"	,I2CSlaveHandler::stop},
		{ NULL      ,NULL   ,NULL}
	};

	const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_I2C_SLAVE={
		"mbedJS","I2CSlave",func_table
	};
}
