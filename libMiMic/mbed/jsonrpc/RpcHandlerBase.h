#include "mod/ModJsonRpc.h"
#include "MbedJsApi.h"
#include "mbed.h"
namespace MiMic
{

	/**
	 * JSONRPCハンドラのベースクラス。
	 * よく使う関数群を定義します。
	 */
    class RpcHandlerBase
    {
    public:
    	/**
    	 * MiMicRPCの定義するピンIDをmbedピンIDへ変換します。
    	 * ターゲットごとに実装します。
    	 */
    	static PinName pinId2PinName(unsigned int i_id);
    	static PinMode pinmodeId2PinMode(unsigned int i_id);
    	static PortName portId2PortName(unsigned int i_id);

    	/**
    	 * Websocketコネクションに同期したオブジェクトリストにnewオブジェクトを登録します。
    	 * 登録するオブジェクトは、{@link BasicRpcObject}でラップする必要があります。
    	 */
        static void addNewObjectBatch(ModJsonRpc* i_mod,unsigned int i_id,ModJsonRpc::BasicRpcObject* i_new_object)
        {
            int i=i_mod->addObject(i_new_object);
            if(i<0){
                delete i_new_object;
                i_mod->putError(i_id,ModJsonRpc::INVALID_PARAMS);
                return;
            }
            i_mod->putResult(i_id,"%d",i);
            return;
        }
        /** i_idx番目のRPCパラメータをiidとしてインスタンスを取得します。*/
        static void* getObjectBatch(ModJsonRpc* i_mod,const union NyLPC_TJsonRpcParserResult* i_rpc,int i_idx=0)
        {
        	NyLPC_TInt32 v;
            if(!NyLPC_TJsonRpcParserResult_getInt32(i_rpc,i_idx,&v)){
            	i_mod->putError(i_rpc->method.id,ModJsonRpc::INVALID_PARAMS);
                return NULL;
            }
            void* ret=i_mod->getObject(v);
        	if(ret==NULL){
                i_mod->putError(i_rpc->method.id,ModJsonRpc::INTERNAL_ERROR);
                return NULL;
        	}
        	return ret;
        }

    	/** u...パラメータをN個のPinIDと解釈して返す。失敗した場合は終了処理も行う。
    	 */
    	static bool getParamsAsPin(ModJsonRpc* i_mod,const union NyLPC_TJsonRpcParserResult* i_rpc,PinName* o_pins,int i_num_of_pins)
    	{
            for(int i=0;i<i_num_of_pins;i++){
            	NyLPC_TUInt32 p;
                if(!NyLPC_TJsonRpcParserResult_getUInt32(i_rpc,i,&p)){
                	i_mod->putError(i_rpc->method.id,ModJsonRpc::INVALID_PARAMS);
                    return false;
                }
                o_pins[i]=pinId2PinName(p);
            }
    		return true;
    	}
    	static bool getParamInt(ModJsonRpc* i_mod,const union NyLPC_TJsonRpcParserResult* i_rpc,int& o_val,int i_idx)
    	{
			if(!NyLPC_TJsonRpcParserResult_getInt32(i_rpc,i_idx,((NyLPC_TInt32*)&o_val))){
				i_mod->putError(i_rpc->method.id,ModJsonRpc::INVALID_PARAMS);
				return false;
			}
    		return true;
    	}
    	static bool getParamUInt(ModJsonRpc* i_mod,const union NyLPC_TJsonRpcParserResult* i_rpc,unsigned int& o_val,int i_idx)
    	{
			if(!NyLPC_TJsonRpcParserResult_getUInt32(i_rpc,i_idx,((NyLPC_TUInt32*)&o_val))){
				i_mod->putError(i_rpc->method.id,ModJsonRpc::INVALID_PARAMS);
				return false;
			}
    		return true;
    	}
    	static bool getParamsInt(ModJsonRpc* i_mod,const union NyLPC_TJsonRpcParserResult* i_rpc,int* o_val,int i_num_ofparams,int i_start=0)
    	{
            for(int i=0;i<i_num_ofparams;i++){
                if(!NyLPC_TJsonRpcParserResult_getInt32(i_rpc,i_start+i,(NyLPC_TInt32*)(o_val+i))){
                	i_mod->putError(i_rpc->method.id,ModJsonRpc::INVALID_PARAMS);
                    return false;
                }
            }
    		return true;
    	}
    	static bool getParamsUInt(ModJsonRpc* i_mod,const union NyLPC_TJsonRpcParserResult* i_rpc,unsigned int* o_val,int i_num_ofparams,int i_start=0)
    	{
            for(int i=0;i<i_num_ofparams;i++){
                if(!NyLPC_TJsonRpcParserResult_getUInt32(i_rpc,i_start+i,(NyLPC_TUInt32*)(o_val+i))){
                	i_mod->putError(i_rpc->method.id,ModJsonRpc::INVALID_PARAMS);
                    return false;
                }
            }
    		return true;
    	}

    };

}
