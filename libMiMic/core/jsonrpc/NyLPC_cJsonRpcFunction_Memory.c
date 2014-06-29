#include "NyLPC_net.h"

///**
// * 1バイトをメモリに書き込む
// */
//static NyLPC_TBool write(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
//{
//	//ubb
//	NyLPC_TcModJsonRpc_t* mod=(NyLPC_TcModJsonRpc_t*)i_param;
//	NyLPC_TUInt8* addr;
//	NyLPC_TUInt8 v,mask;
//	if(NyLPC_TJsonRpcParserResult_getUInt32(i_rpc,0,((NyLPC_TUInt32*)&addr))){
//		if(NyLPC_TJsonRpcParserResult_getByte(i_rpc,1,((NyLPC_TUInt8*)&v))){
//			if(NyLPC_TJsonRpcParserResult_getByte(i_rpc,2,((NyLPC_TUInt8*)&mask))){
//				*addr=((*addr)&(~mask))|((*addr)|mask);
//				return NyLPC_cModJsonRpc_putResult(mod,i_rpc->method.id,"");
//			}
//		}
//	}
//	NyLPC_cModJsonRpc_putError(mod,i_rpc->method.id,NyLPC_TJsonRpcErrorCode_INVALID_PARAMS);
//	return NyLPC_TBool_FALSE;
//}
///**
// * 1バイトをメモリから読み込む
// */
//static NyLPC_TBool read(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
//{
//	//u
//	NyLPC_TcModJsonRpc_t* mod=(NyLPC_TcModJsonRpc_t*)i_param;
//	NyLPC_TUInt8* addr;
//	if(NyLPC_TJsonRpcParserResult_getUInt32(i_rpc,0,((NyLPC_TUInt32*)&addr))){
//		return NyLPC_cModJsonRpc_putResult(mod,i_rpc->method.id,"%u",(int)(*addr));
//	}
//	NyLPC_cModJsonRpc_putError(mod,i_rpc->method.id,NyLPC_TJsonRpcErrorCode_INVALID_PARAMS);
//	return NyLPC_TBool_FALSE;
//}
//
///**
// * write32
// */
//static NyLPC_TBool write32(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
//{
//	//uuu
//	NyLPC_TcModJsonRpc_t* mod=(NyLPC_TcModJsonRpc_t*)i_param;
//	NyLPC_TUInt32* addr;
//	NyLPC_TUInt32 v,mask;
//	if(NyLPC_TJsonRpcParserResult_getUInt32(i_rpc,0,((NyLPC_TUInt32*)&addr))){
//		if(NyLPC_TJsonRpcParserResult_getUInt32(i_rpc,1,((NyLPC_TUInt32*)&v))){
//			if(NyLPC_TJsonRpcParserResult_getUInt32(i_rpc,2,((NyLPC_TUInt32*)&mask))){
//				*addr=((*addr)&(~mask))|((*addr)|mask);
//				return NyLPC_cModJsonRpc_putResult(mod,i_rpc->method.id,"");
//			}
//		}
//	}
//	NyLPC_cModJsonRpc_putError(mod,i_rpc->method.id,NyLPC_TJsonRpcErrorCode_INVALID_PARAMS);
//	return NyLPC_TBool_FALSE;
//}
///**
// * read32
// */
//static NyLPC_TBool read32(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
//{
//	//u
//	NyLPC_TcModJsonRpc_t* mod=(NyLPC_TcModJsonRpc_t*)i_param;
//	NyLPC_TUInt32* addr;
//	NyLPC_TUInt32 v;
//	if(NyLPC_TJsonRpcParserResult_getUInt32(i_rpc,0,((NyLPC_TUInt32*)&addr))){
//		v=(*addr);
//		return NyLPC_cModJsonRpc_putResult(mod,i_rpc->method.id,"%u",v);
//	}
//	NyLPC_cModJsonRpc_putError(mod,i_rpc->method.id,NyLPC_TJsonRpcErrorCode_INVALID_PARAMS);
//	return NyLPC_TBool_FALSE;
//}
/**
 * 準備(特にやることない)
 */
static NyLPC_TBool init(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
{
	//uB
	NyLPC_TcModJsonRpc_t* mod=(NyLPC_TcModJsonRpc_t*)i_param;
	return NyLPC_cModJsonRpc_putResult(mod,i_rpc->method.id,"");
}

/**
 * メモリブロックを書き込む
 */
static NyLPC_TBool write(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
{
	//uB
	NyLPC_TcModJsonRpc_t* mod=(NyLPC_TcModJsonRpc_t*)i_param;
	NyLPC_TUInt8* addr;
	const NyLPC_TUInt8* v;
	NyLPC_TUInt8 l;
	if(NyLPC_TJsonRpcParserResult_getUInt32(i_rpc,0,((NyLPC_TUInt32*)&addr))){
		if(NyLPC_TJsonRpcParserResult_getByteArray(i_rpc,1,&v,&l)){
			memcpy(addr,v,l);
			return NyLPC_cModJsonRpc_putResult(mod,i_rpc->method.id,"");
		}
	}
	NyLPC_cModJsonRpc_putError(mod,i_rpc->method.id,NyLPC_TJsonRpcErrorCode_INVALID_PARAMS);
	return NyLPC_TBool_FALSE;
}
/**
 * メモリブロックを読み込む
 */
static NyLPC_TBool read(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param)
{
	//uu
	NyLPC_TcModJsonRpc_t* mod=(NyLPC_TcModJsonRpc_t*)i_param;
	NyLPC_TUInt8* addr;
	NyLPC_TUInt32 l;
	if(NyLPC_TJsonRpcParserResult_getUInt32(i_rpc,0,((NyLPC_TUInt32*)&addr))){
		if(NyLPC_TJsonRpcParserResult_getUInt32(i_rpc,1,((NyLPC_TUInt32*)&l))){
			return NyLPC_cModJsonRpc_putResult(mod,i_rpc->method.id,"\"%.*B\"",(int)l,addr);
		}
	}
	NyLPC_cModJsonRpc_putError(mod,i_rpc->method.id,NyLPC_TJsonRpcErrorCode_INVALID_PARAMS);
	return NyLPC_TBool_FALSE;
}



const static struct NyLPC_TJsonRpcMethodDef func_table[]=
{
	{ "init"	,""		,init},
	{ "write"	,"uB"	,write},
	{ "read"	,"uu"   ,read},
	{ NULL      ,NULL   ,NULL}
};

const struct NyLPC_TJsonRpcClassDef NyLPC_cJsonRpcFunction_Memory={
	"MiMic","Memory",func_table
};

