#include "RpcHandlerBase.h"
/**
 * malloc出来る最大サイズを得る関数
 */
static int getFreeMemory()
{
	int size=1;
	for(;;){
		void* buf=malloc(size);
		if(buf==NULL){
			break;
		}
		free(buf);
		size*=2;
	}
	int min=size/2;
	int max=size;
	for(;;){
		int n=(max+min)/2;
		void* b=malloc(n);
		if(b){
			free(b);
			min=n;
		}else{
			max=n;
		}
		if(max-min==1){
			return min;
		}
	}
}
namespace MiMic
{
    class McuHandler :RpcHandlerBase
    {
    public:
        static NyLPC_TBool disposeObject(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
            int id;
            if(getParamInt(mod,i_rpc,id,0)){
            	mod->putResult(i_rpc->method.id,"%d",mod->removeObject(id));
            }
            return NyLPC_TBool_TRUE;
        }
        /** idに合致するオブジェクトを削除します。
         * RPC-returnは1:成功、0:失敗です。
         */
        static NyLPC_TBool getInfo(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
        {
        	//d return d
            ModJsonRpc* mod=((ModJsonRpc::TcJsonRpcEx_t*)i_param)->cppmod_ptr;
			//version,mcu,memoryの順
			mod->putResult(i_rpc->method.id,
				"\"%s\",\"%s\",\"%s\",\"%s\",%d",
				NyLPC_cMiMicEnv_getStrProperty(NyLPC_cMiMicEnv_VERSION),
				NyLPC_cMiMicEnv_getStrProperty(NyLPC_cMiMicEnv_SHORT_NAME),
				NyLPC_cMiMicEnv_getStrProperty(NyLPC_cMiMicEnv_ETHERNET_PHY),
				NyLPC_cMiMicEnv_getStrProperty(NyLPC_cMiMicEnv_MCU_NAME),
				/*getFreeMemory()*/-1);
            return NyLPC_TBool_TRUE;
        }
    };

const static struct NyLPC_TJsonRpcMethodDef func_table[]=
{
    { "getInfo"		,"" ,McuHandler::getInfo},
    { "disposeObject","d",McuHandler::disposeObject},
    { NULL      ,NULL   ,NULL}
};

const struct NyLPC_TJsonRpcClassDef MbedJsApi::RPC_MBED_MCU={
    "mbedJS","Mcu",func_table
};



}

