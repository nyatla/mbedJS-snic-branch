#ifndef NYLPC_CJSONRPCPARSER_H_
#define NYLPC_CJSONRPCPARSER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "NyLPC_stdlib.h"


#define NyLPC_TJsonRpcErrorCode_PARSE_ERROR			(-32700)
#define NyLPC_TJsonRpcErrorCode_INVALID_REQUEST		(-32600)
#define NyLPC_TJsonRpcErrorCode_METHOD_NOT_FOUND	(-32601)
#define NyLPC_TJsonRpcErrorCode_INVALID_PARAMS		(-32602)
#define NyLPC_TJsonRpcErrorCode_INTERNAL_ERROR		(-32603)
#define NyLPC_TJsonRpcErrorCode_SERVER_ERROR_BASE	(-32000)


/** 型定義*/
union NyLPC_TJsonRpcParserResult;

/**
 * RPC関数ハンドラ型です。
 * @param
 */
typedef NyLPC_TBool (*NyLPC_TJsonRpcHandler)(const union NyLPC_TJsonRpcParserResult* i_rpc,void* i_param);

/**
 * JSON RPC定義テーブルの一要素。
 * メソッド名とパラメータパターンのセットを定義します。
 * この構造体は配列としてNyLPC_TJsonRpcFunctionTableから参照されます。
 */
struct NyLPC_TJsonRpcMethodDef
{
	/**
	 * 関数名
	 */
	const char* name;
	/**
	 * パラメータパターン
	 * <ul>
	 * <li>s - 文字列</li>
	 * <li>i - signed int32</li>
	 * <li>u - unsigned int32</li>
	 * <li>b - unsigned int8</li>
	 * <li>B - BSTR (string ByteArray)</li>
	 * </ul>
	 */
	const char* param_patt;
	/**
	 * 外部のDispatch関数で使われるRPC関数ハンドラ。
	 */
	NyLPC_TJsonRpcHandler handler;
};


/**
 * JsonRPCクラスの定義テーブル。
 * JsonRPCは、 [namespace]::[]
 */
struct NyLPC_TJsonRpcClassDef
{
	const char* names_pace;
	const char* class_name;
	const struct NyLPC_TJsonRpcMethodDef* functions;
};





#define NyLPC_TJsonRpcParserResult_TYPE_UNKNOWN	0
#define NyLPC_TJsonRpcParserResult_TYPE_RESULT	1
#define NyLPC_TJsonRpcParserResult_TYPE_METHOD	2

/** NyLPC_TJsonRpcParserResultが格納できる引数の最大数。パラメータの最大数に等しくなります。 */
#define NyLPC_TJsonRpcParserResult_NUMBER_OF_PARAM_INDEX 32
/**
 * NyLPC_TJsonRpcParserResultが格納できるパラメータの総バイト数。
 * (文字数+1)+(uint32|int32)*4+uint8の合計値です。
 */
#define NyLPC_TJsonRpcParserResult_PARAM_BUF 256

/**
 * JSONRPC構文のパース結果を格納します。
 * 開発メモ
 * 更新する場合は、param_indexまでの構造体のレイアウトを破壊しないようにしてください。
 */
union NyLPC_TJsonRpcParserResult
{
	NyLPC_TUInt8 	type;				//タイプ
	/**　この構造体は type==NyLPC_TJsonRpcParserResult_TYPE_METHODのときに有効です。 */
	struct{
		NyLPC_TUInt8 	_type;			//タイプ
		NyLPC_TUInt8 	func_number;	//確定した関数番号。
		NyLPC_TUInt8 	_padding[2];	//パディング
		NyLPC_TInt32 	id;				//idパラメータの値
		NyLPC_TChar		param_buf[NyLPC_TJsonRpcParserResult_PARAM_BUF];
		NyLPC_TUInt8	param_index[NyLPC_TJsonRpcParserResult_NUMBER_OF_PARAM_INDEX];
		/** 関数の含まれるクラスへのポインタ。 */
		const struct NyLPC_TJsonRpcClassDef* class_def;
	}method;
	struct{
		NyLPC_TUInt8 	_type;			//タイプ
		NyLPC_TUInt8 	_padding[3];	//パディング
		NyLPC_TInt32 	id;				//idパラメータの値
		NyLPC_TChar		param_buf[NyLPC_TJsonRpcParserResult_PARAM_BUF];
		NyLPC_TUInt8	param_index[NyLPC_TJsonRpcParserResult_NUMBER_OF_PARAM_INDEX];
	}result;
	struct{
		NyLPC_TUInt8 	_type;		//タイプ
		NyLPC_TInt8 	_padding[3];//
		NyLPC_TInt32 	_id;		//idパラメータの値
		NyLPC_TChar		param_buf[NyLPC_TJsonRpcParserResult_PARAM_BUF];
		NyLPC_TUInt8	param_index[NyLPC_TJsonRpcParserResult_NUMBER_OF_PARAM_INDEX];
	}raw;
};
/**
 * Resultに格納される結果がハンドラを持っているかを返します。
 */
#define NyLPC_TJsonRpcParserResult_hasMethodHandler(i_struct) ((i_struct)->method.class_def->functions[(i_struct)->method.func_number].handler!=NULL)
/**
 * Resultに含まれるハンドラを呼び出します。
 * @param i_struct
 * Result構造体
 * @param i_param
 * ハンドラに引き渡すパラメータ
 * @return
 * FALSEの場合、ループを終了してください。
 */
#define NyLPC_TJsonRpcParserResult_callMethodHandler(i_struct,i_param) (i_struct)->method.class_def->functions[(i_struct)->method.func_number].handler((i_struct),(i_param))

/**
 * i_idx番目のパラメータをuint32としてo_valへ取り出します。
 */
NyLPC_TBool NyLPC_TJsonRpcParserResult_getUInt32(const union NyLPC_TJsonRpcParserResult* i_struct, NyLPC_TInt16 i_idx, NyLPC_TUInt32* o_val);
/**
 * i_idx番目のパラメータをint32としてo_valへ取り出します。
 */
NyLPC_TBool NyLPC_TJsonRpcParserResult_getInt32(const union NyLPC_TJsonRpcParserResult* i_struct, NyLPC_TInt16 i_idx, NyLPC_TInt32* o_val);
/**
 * i_idx番目のパラメータをchar[]としてo_valへ取り出します。
 */
NyLPC_TBool NyLPC_TJsonRpcParserResult_getStr(const union NyLPC_TJsonRpcParserResult* i_struct, NyLPC_TInt16 i_idx, const NyLPC_TChar** o_val, NyLPC_TUInt8* o_len);
/**
* i_idx番目のパラメータをuchar[]としてo_valへ取り出します。
*/
NyLPC_TBool NyLPC_TJsonRpcParserResult_getByteArray(const union NyLPC_TJsonRpcParserResult* i_struct, NyLPC_TInt16 i_idx, const NyLPC_TUInt8** o_val, NyLPC_TUInt8* o_len);

/**
 * i_idx番目のパラメータをuint8としてo_valへ取り出します。
 */
NyLPC_TBool NyLPC_TJsonRpcParserResult_getByte(const union NyLPC_TJsonRpcParserResult* i_struct, NyLPC_TInt16 i_idx, NyLPC_TUInt8* o_val);

/********************************************************************************
 *
 * NyLPC_TcJsonRpcParser
 *
 ********************************************************************************/
/**
 * JSONRPCの型定義定数です。
 */
#define NyLPC_cJsonRpcParser_TYPE_INT32		'd'
#define NyLPC_cJsonRpcParser_TYPE_UINT32	'u'
#define NyLPC_cJsonRpcParser_TYPE_STRING	's'
#define NyLPC_cJsonRpcParser_TYPE_BYTE		'b'
#define NyLPC_cJsonRpcParser_TYPE_BSTRING	'B'

/**
* JsonRPCメッセージをパースします。パース出来るメッセージは以下の通りです。
* クラスは、RPC関数定義テーブルに従ってメッセージを分析し、テーブルに存在する関数のみを返却することができます。
* <p>
* メッセージ形式
* <pre>
* METHOD:
* {"method":METHOD,"version":VERSION,"params":PARAMS,"id":ID}
* METHOD VERSION as string
* ID as uint32
* PARAMS as Array of (string|uint32|int32)
* </pre>
* </p>
*/
typedef struct NyLPC_TcJsonRpcParser NyLPC_TcJsonRpcParser_t;




/**　パーサの状態値*/
typedef NyLPC_TUInt8 NyLPC_TcJsonRpcParser_TStatus;

#define NyLPC_TcJsonRpcParser_ST_START			0x01	//開始ブランケット受信待ち
#define NyLPC_TcJsonRpcParser_ST_END			0x02	//終了受信済
#define NyLPC_TcJsonRpcParser_ST_ERROR			0x03	//エラー発生
#define NyLPC_TcJsonRpcParser_ST_NAME_Q			0x04	//名前クオート受信待ち
#define NyLPC_TcJsonRpcParser_ST_NAME_STR		0x05	//名前受信中
#define NyLPC_TcJsonRpcParser_ST_NV_SEP			0x06	//名前と値のセパレータ待ち
#define NyLPC_TcJsonRpcParser_ST_VAL			0x07	//値開始待ち
#define NyLPC_TcJsonRpcParser_ST_VAL_STR		0x08	//文字列受信
#define NyLPC_TcJsonRpcParser_ST_VAL_UINT		0x09	//UINT受信中
#define NyLPC_TcJsonRpcParser_ST_VAL_INT		0x10	//INT受信中
#define NyLPC_TcJsonRpcParser_ST_NEXT			0x11	//次のNAMEもしくは終了ブランケット
#define NyLPC_TcJsonRpcParser_ST_PARAMS			0x12	//PARAM要素パース中



/** NyLPC_TcJsonRpcParserの定数値です。　字句解析ワークメモリの長さ。256未満8*n-4の数を指定してください。 */
#define NyLPC_TcJsonRpcParser_WORK_MAX (48-4)

/**
 * クラス構造体です。
 */
struct NyLPC_TcJsonRpcParser
{
	const struct NyLPC_TJsonRpcClassDef** _class_def;
	union{
		struct{
			NyLPC_TChar 	buf[NyLPC_TcJsonRpcParser_WORK_MAX];	//文字解析メモリ
			NyLPC_TUInt8 	n;										//字句解析の文字数
			NyLPC_TUInt8 	st;										//字句解析のサブステータス
			NyLPC_TUInt8 	vt;										//数字エスケープ解釈のテンポラリ
			NyLPC_TUInt8 	_padding;								//
		}str;
		struct{
			NyLPC_TInt32 v;
			NyLPC_TInt8	s;
		}int32;
		NyLPC_TUInt32 uint32;
	}_work;
	NyLPC_TcJsonRpcParser_TStatus 	_st;		//パーサステータス
	NyLPC_TUInt8 	_pst;		//PARAMS解析ステータス
	NyLPC_TUInt8	_name_id;	//解析中のNAME_ID
	NyLPC_TUInt8	_pcounter;	//パラメタ解析に使うワークカウンタ
	union NyLPC_TJsonRpcParserResult* _result;//出力格納先
};






/**
 * インスタンスを初期化します。
 * @param i_class_def
 * クラステーブルの配列です。NULLで終端します。
 */
void NyLPC_cJsonRpcParser_initialize(
	NyLPC_TcJsonRpcParser_t* i_inst,
	const struct NyLPC_TJsonRpcClassDef** i_class_def);

#define NyLPC_cJsonRpcParser_finalize(i)
/**
 * パーサの状態を初期化します。
 * @param i_result
 * パース結果の出力先構造体のアドレスです。
 */
void NyLPC_cJsonRpcParser_initParser(NyLPC_TcJsonRpcParser_t* i_inst, union NyLPC_TJsonRpcParserResult* i_result);
/**
 * パーサに文字列を入力します。入力後は、NyLPC_cJsonRpcParser_getStatusでパーサの状態をチェックしてください。
　*/
void NyLPC_cJsonRpcParser_putChar(NyLPC_TcJsonRpcParser_t* i_inst, char i_c);

#define NyLPC_cJsonRpcParser_getStatus(i) ((i)->_st)
/** クラス定義テーブルを返します。*/
#define NyLPC_cJsonRpcParser_getClassDef(i) ((i)->_class_def)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CJSONRPCPARSER4_H_ */
