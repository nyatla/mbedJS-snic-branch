#include "NyLPC_cModJsonRpc.h"




void NyLPC_cModJsonRpc_initialize(NyLPC_TcModJsonRpc_t* i_inst,const NyLPC_TChar* i_ref_root_path,const struct NyLPC_TJsonRpcClassDef** i_class_tbl)
{
	NyLPC_cModWebSocket_initialize(&i_inst->super,i_ref_root_path);
	NyLPC_cJsonRpcParser_initialize(&i_inst->_rpc_parser,i_class_tbl);
}
void NyLPC_cModJsonRpc_finalize(NyLPC_TcModJsonRpc_t* i_inst)
{
	NyLPC_cJsonRpcParser_finalize(&i_inst->_rpc_parser);
	NyLPC_cModWebSocket_finalize(&i_inst->super);
}
#define NyLPC_cModJsonRpc_canHandle(i,c) NyLPC_cModWebSocket_canHandle(&((i)->super),(c))

#define NyLPC_cModJsonRpc_close(i,t) NyLPC_cModWebSocket_close(&((i)->super),(t))


NyLPC_TBool NyLPC_cModJsonRpc_execute(NyLPC_TcModJsonRpc_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection)
{
	//executeで強制初期化
	NyLPC_cJsonRpcParser_initParser(&i_inst->_rpc_parser,&i_inst->_result);
	return NyLPC_cModWebSocket_execute(&((i_inst)->super),i_connection);
}

static NyLPC_TInt32 readHandler(void* i_param,NyLPC_TChar i_c)
{
	NyLPC_TcModJsonRpc_t* inst=(NyLPC_TcModJsonRpc_t*)i_param;
	NyLPC_cJsonRpcParser_putChar(&inst->_rpc_parser,i_c);
	switch(NyLPC_cJsonRpcParser_getStatus(&(inst->_rpc_parser))){
	case NyLPC_TcJsonRpcParser_ST_END:
		return 0;//終端到達。中断
	case NyLPC_TcJsonRpcParser_ST_ERROR:
		return -1;//エラー
	default:
		return 1;//次の文字をリクエスト
	}
}


NyLPC_TBool NyLPC_cModJsonRpc_processRpcMessage(NyLPC_TcModJsonRpc_t* i_inst)
{
	//終了した場合のみパーサステータスを初期化。
	switch(NyLPC_cJsonRpcParser_getStatus(&i_inst->_rpc_parser)){
	case NyLPC_TcJsonRpcParser_ST_END:
		NyLPC_cJsonRpcParser_initParser(&i_inst->_rpc_parser,&i_inst->_result);
		break;
	case NyLPC_TcJsonRpcParser_ST_ERROR:
		//パーサがエラーを検出してたら何もしない。
		return NyLPC_TBool_FALSE;
	}
	//コールバックハンドラで受信
	if(NyLPC_cModWebSocket_readCB(&(i_inst->super),readHandler,i_inst)<0){
		return NyLPC_TBool_FALSE;
	}
	return NyLPC_TBool_TRUE;
}

const union NyLPC_TJsonRpcParserResult* NyLPC_cModJsonRpc_getMessage(NyLPC_TcModJsonRpc_t* i_inst)
{
	return (NyLPC_cJsonRpcParser_getStatus(&i_inst->_rpc_parser)!=NyLPC_TcJsonRpcParser_ST_END)?NULL:(&i_inst->_result);
}

NyLPC_TBool NyLPC_cModJsonRpc_putRawMessageV(NyLPC_TcModJsonRpc_t* i_inst,const NyLPC_TChar* i_fmt,va_list i_args)
{
	return NyLPC_cModWebSocket_writeFormatV(&i_inst->super,i_fmt,i_args);
}
NyLPC_TBool NyLPC_cModJsonRpc_putRawMessage(NyLPC_TcModJsonRpc_t* i_inst,const NyLPC_TChar* i_fmt,...)
{
	NyLPC_TBool r;
	va_list a;
	va_start(a,i_fmt);
	r=NyLPC_cModWebSocket_writeFormatV(&i_inst->super,i_fmt,a);
	va_end(a);
	return r;
}



/**
 * @param i_params_fmt
 * パラメータ部のフォーマット文字列です。NyLPC_cFormatWriterと同等の構文を使用できます。
 * ここで指定した書式文字列は、"そのまま"パラメータ部に表示されます。文字列を返す場合は、次のように\"でエスケープしてください。
 * '0,1,2,-1,\"result\",3'
 */
NyLPC_TBool NyLPC_cModJsonRpc_putResult(NyLPC_TcModJsonRpc_t* i_inst,NyLPC_TUInt32 i_id,const NyLPC_TChar* i_params_fmt,...)
{
	NyLPC_TBool r;
	va_list a;
	//書き込み文字数の事前計算
	va_start(a,i_params_fmt);
	r=NyLPC_cModJsonRpc_putResultV(i_inst,i_id,i_params_fmt,a);
	va_end(a);
	return r;
}

/**
 * @param i_params_fmt
 * パラメータ部のフォーマット文字列です。NyLPC_cFormatWriterと同等の構文を使用できます。
 * ここで指定した書式文字列は、"そのまま"パラメータ部に表示されます。文字列を返す場合は、次のように\"でエスケープしてください。
 * '0,1,2,-1,\"result\",3'
 */
NyLPC_TBool NyLPC_cModJsonRpc_putResultV(NyLPC_TcModJsonRpc_t* i_inst,NyLPC_TUInt32 i_id,const NyLPC_TChar* i_params_fmt,va_list i_a)
{
	NyLPC_TBool r;
	va_list a;
	NyLPC_TInt16 l;

	//書き込み文字数の事前計算
	NyLPC_va_copy(a,i_a);
	l=NyLPC_cModWebSocket_testFormat(&i_inst->super,"{\"jsonrpc\":\"2.0\",\"result\":[");
	l+=NyLPC_cModWebSocket_testFormatV(&i_inst->super,i_params_fmt,a);
	l+=NyLPC_cModWebSocket_testFormat(&i_inst->super,"],\"id\":%d}",i_id);
	va_end(a);

	//バルク書き込み
	NyLPC_cModWebSocket_startBulkWrite(&i_inst->super,l);
	r=NyLPC_cModWebSocket_writeBulkFormat(&i_inst->super,"{\"jsonrpc\":\"2.0\",\"result\":[");
	r=r&&NyLPC_cModWebSocket_writeBulkFormatV(&i_inst->super,i_params_fmt,i_a);
	r=r&&NyLPC_cModWebSocket_writeBulkFormat(&i_inst->super,"],\"id\":%d}",i_id);
	r=r&&NyLPC_cModWebSocket_endBulkWrite(&i_inst->super);
	return r;
}

struct CodeMsgTbl{
	const char* n;
	NyLPC_TInt32 id;
};

const static struct CodeMsgTbl _table[]=
{
	{"Parse error",NyLPC_TJsonRpcErrorCode_PARSE_ERROR},
	{"Invalid Request",NyLPC_TJsonRpcErrorCode_INVALID_REQUEST},
	{"Method not found",NyLPC_TJsonRpcErrorCode_METHOD_NOT_FOUND},
	{"Invalid params",NyLPC_TJsonRpcErrorCode_INVALID_PARAMS},
	{"Internal error",NyLPC_TJsonRpcErrorCode_INTERNAL_ERROR},
	{"Server error",NyLPC_TJsonRpcErrorCode_SERVER_ERROR_BASE}
};
static const NyLPC_TChar* code2msg(NyLPC_TInt32 i_id)
{
	int i;
	for(i=0;i<6;i++){
		if(i_id==_table[i].id){
			return _table[i].n;
		}
	}
	return _table[6].n;
}

NyLPC_TBool NyLPC_cModJsonRpc_putError(NyLPC_TcModJsonRpc_t* i_inst,NyLPC_TUInt32 i_id,NyLPC_TInt32 i_code)
{
	return NyLPC_cModWebSocket_writeFormat(&i_inst->super,"{\"jsonrpc\":\"2.0\",\"error\":{\"code\": %d, \"message\": \"%s\"},\"id\":%d}",i_code,code2msg(i_code),i_id);
}
