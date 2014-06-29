#include "NyLPC_cJsonRpcParser.h"
#include <ctype.h>
//
//	NyLPC_TJsonRpcParserResult
//


NyLPC_TBool NyLPC_TJsonRpcParserResult_getUInt32(const union NyLPC_TJsonRpcParserResult* i_struct,NyLPC_TInt16 i_idx,NyLPC_TUInt32* o_val)
{
	if(i_struct->method.param_index[i_idx]==0xff){
		return NyLPC_TBool_FALSE;
	}
	if(i_struct->method.class_def->functions[i_struct->method.func_number].param_patt[i_idx]!=NyLPC_cJsonRpcParser_TYPE_UINT32){
		return NyLPC_TBool_FALSE;
	}
	*o_val = *((NyLPC_TUInt32*)(i_struct->method.param_buf + i_struct->method.param_index[i_idx]));
	return NyLPC_TBool_TRUE;
}
NyLPC_TBool NyLPC_TJsonRpcParserResult_getInt32(const union NyLPC_TJsonRpcParserResult* i_struct, NyLPC_TInt16 i_idx, NyLPC_TInt32* o_val)
{
	if(i_struct->method.param_index[i_idx]==0xff){
		return NyLPC_TBool_FALSE;
	}
	if (i_struct->method.class_def->functions[i_struct->method.func_number].param_patt[i_idx] != NyLPC_cJsonRpcParser_TYPE_INT32){
		return NyLPC_TBool_FALSE;
	}
	*o_val = *((NyLPC_TInt32*)(i_struct->method.param_buf + i_struct->method.param_index[i_idx]));
	return NyLPC_TBool_TRUE;
}
NyLPC_TBool NyLPC_TJsonRpcParserResult_getStr(const union NyLPC_TJsonRpcParserResult* i_struct, NyLPC_TInt16 i_idx, const NyLPC_TChar** o_val, NyLPC_TUInt8* o_len)
{
	if(i_struct->method.param_index[i_idx]==0xff){
		return NyLPC_TBool_FALSE;
	}
	if (i_struct->method.class_def->functions[i_struct->method.func_number].param_patt[i_idx] != NyLPC_cJsonRpcParser_TYPE_STRING){
		return NyLPC_TBool_FALSE;
	}
	*o_val = ((NyLPC_TChar*)(i_struct->method.param_buf + i_struct->method.param_index[i_idx]+1));
	if (o_len){
		*o_len = *((NyLPC_TUInt8*)(*o_val-1));
	}
	return NyLPC_TBool_TRUE;
}
NyLPC_TBool NyLPC_TJsonRpcParserResult_getByteArray(const union NyLPC_TJsonRpcParserResult* i_struct, NyLPC_TInt16 i_idx, const NyLPC_TUInt8** o_val, NyLPC_TUInt8* o_len)
{
	if (i_struct->method.param_index[i_idx] == 0xff){
		return NyLPC_TBool_FALSE;
	}
	if (i_struct->method.class_def->functions[i_struct->method.func_number].param_patt[i_idx] != NyLPC_cJsonRpcParser_TYPE_BSTRING){
		return NyLPC_TBool_FALSE;
	}
	*o_val = ((NyLPC_TUInt8*)(i_struct->method.param_buf + i_struct->method.param_index[i_idx] + 1));
	if (o_len){
		*o_len = *((NyLPC_TUInt8*)(*o_val - 1));
	}
	return NyLPC_TBool_TRUE;
}

NyLPC_TBool NyLPC_TJsonRpcParserResult_getByte(const union NyLPC_TJsonRpcParserResult* i_struct, NyLPC_TInt16 i_idx, NyLPC_TUInt8* o_val)
{
	if(i_struct->method.param_index[i_idx]==0xff){
		return NyLPC_TBool_FALSE;
	}
	if (i_struct->method.class_def->functions[i_struct->method.func_number].param_patt[i_idx] != NyLPC_cJsonRpcParser_TYPE_BYTE){
		return NyLPC_TBool_FALSE;
	}
	*o_val = *(i_struct->method.param_buf + i_struct->method.param_index[i_idx]);
	return NyLPC_TBool_TRUE;
}


//
//	NyLPC_cJsonRpcParser
//



#define NAME_ID_UNKNOWN		0
#define NAME_ID_VERSION		1
#define NAME_ID_METHOD		2
#define NAME_ID_RESULT		3
#define NAME_ID_PARAMS		4
#define NAME_ID_ID			5

const struct NyLPC_TTextIdTbl method_name_tbl[]=
{
	{"version",NAME_ID_VERSION},
	{"method",NAME_ID_METHOD},
	{"params",NAME_ID_PARAMS},
	{"id",NAME_ID_ID},
	{NULL,NAME_ID_UNKNOWN}
};


static const struct NyLPC_TJsonRpcClassDef* findFunction(const struct NyLPC_TJsonRpcClassDef** i_tbl, const NyLPC_TChar* i_method_path, NyLPC_TUInt8* o_function_idx);
static void putchar_params(NyLPC_TcJsonRpcParser_t* i_inst, char i_c);
static NyLPC_TUInt8 valTerminator2St(NyLPC_TChar i_c);


void NyLPC_cJsonRpcParser_initialize(
	NyLPC_TcJsonRpcParser_t* i_inst,
	const struct NyLPC_TJsonRpcClassDef** i_class_def)
{
	i_inst->_class_def=i_class_def;
}
void NyLPC_cJsonRpcParser_initParser(NyLPC_TcJsonRpcParser_t* i_inst,union NyLPC_TJsonRpcParserResult* i_result)
{
	i_inst->_result=i_result;
	i_result->type=NyLPC_TJsonRpcParserResult_TYPE_UNKNOWN;
	i_inst->_st = NyLPC_TcJsonRpcParser_ST_START;
	i_inst->_pcounter = 0;
	memset(i_result->raw.param_index, 0xff, NyLPC_TJsonRpcParserResult_NUMBER_OF_PARAM_INDEX);
}



static const struct NyLPC_TJsonRpcClassDef* findFunction(const struct NyLPC_TJsonRpcClassDef** i_tbl,const NyLPC_TChar* i_method_path,NyLPC_TUInt8* o_function_idx)
{
	const NyLPC_TChar* ns;
	const NyLPC_TChar* class_;
	const NyLPC_TChar* method;
	NyLPC_TUInt8 fidx;
	int l;
	//namespaceの抽出
	ns=i_method_path;
	class_=strchr(i_method_path,':');
	if(class_==NULL){
		return NULL;
	}
	class_++;
	//functionの抽出
	method=strchr(class_,':');
	if(method==NULL){
		return NULL;
	}
	method++;
	while((*i_tbl)!=NULL){
		l=class_-ns-1;
		if(strncmp((*i_tbl)->names_pace,ns,l)!=0 || (*(ns+l))!=':'){
			i_tbl++;
			continue;
		}
		l=method-class_-1;
		if(strncmp((*i_tbl)->class_name,class_,l)!=0 || (*(class_+l))!=':'){
			i_tbl++;
			continue;
		}
		fidx=0;
		while(((*i_tbl)->functions+fidx)->name!=NULL){
			if(strcmp(((*i_tbl)->functions+fidx)->name,method)!=0){
				fidx++;
				continue;
			}
			*o_function_idx=fidx;
			return (*i_tbl);
		}
		break;
	}
	return NULL;
}
/**
 * putchar_paramのステータス値
 */
#define PARAM_ST_START			 1
#define PARAM_ST_VAL			 2
#define PARAM_ST_INT			 3
#define PARAM_ST_UINT			 4
#define PARAM_ST_STR			 5
#define PARAM_ST_STR_ESCAPE_HEX	 6
#define PARAM_ST_STR_ESCAPE		 7
#define PARAM_ST_BYTE			 8
#define PARAM_ST_BSTR			 9
#define PARAM_ST_NEXT			11


/**
 * シングルエスケープ文字列をエスケープされた文字列へ変換します。
 */
static NyLPC_TBool convertSingleCharEscape(const char i_s, char* o_out)
{
	switch (i_s){
	case '0':	*o_out = '\0'; break;
	case 'r':	*o_out = '\r'; break;
	case 'n':	*o_out = '\n'; break;
	case 't':	*o_out = '\t'; break;
	case '\'':	*o_out = '\''; break;
	case '\"':	*o_out = '\"'; break;
	case '\\':	*o_out = '\\'; break;
	default:
		return NyLPC_TBool_FALSE;
	}
	return NyLPC_TBool_TRUE;
}
/**
 * 符号付整数値を構成する文字セットであるかを返します。
 */
static NyLPC_TBool isSignedCharSet(const char i_c)
{
	return (strchr("-0123456789", i_c) != NULL);
}
/**
 * 整数を構成する文字セットであるかを返します。
 */
#define isUnSignedCharSet(i_c) isdigit(i_c)

static void putchar_params(NyLPC_TcJsonRpcParser_t* i_inst, char i_c)
{
	switch (i_inst->_pst){
	case PARAM_ST_START:
		if (i_c == '['){
			//パース開始
			i_inst->_work.str.n = 0;
			i_inst->_pst = PARAM_ST_VAL;
			return;
		}
		if (strchr(" ", i_c) != NULL){
			//無視
			return;
		}
		//エラー
		goto ERROR;
	case PARAM_ST_VAL:
		if (i_inst->_pcounter >= NyLPC_TJsonRpcParserResult_NUMBER_OF_PARAM_INDEX){
			goto ERROR;
		}
		if (strchr(" ", i_c) != NULL){
			//無視
			return;
		}
		if (strchr("]", i_c) != NULL){
			i_inst->_st = NyLPC_TcJsonRpcParser_ST_NEXT;
			i_inst->_pcounter = 0xff;
			return;
		}
		switch (i_inst->_result->method.class_def->functions[i_inst->_result->method.func_number].param_patt[i_inst->_pcounter]){
		case NyLPC_cJsonRpcParser_TYPE_INT32:
			if(!isSignedCharSet(i_c)){
				goto ERROR;
			}
			i_inst->_pst = PARAM_ST_INT;
			i_inst->_work.int32.s = i_c == '-' ? -1 : 1;
			i_inst->_work.int32.v = i_c == '-' ? 0 : (i_c - '0');
			i_inst->_work.str.n = ((i_inst->_work.str.n + 3) / 4) * 4;
			break;
		case NyLPC_cJsonRpcParser_TYPE_UINT32:
			if (!isUnSignedCharSet(i_c)){
				goto ERROR;
			}
			i_inst->_pst = PARAM_ST_UINT;
			i_inst->_work.uint32 = (i_c - '0');
			//開始位置を4バイト境界に
			i_inst->_work.str.n = ((i_inst->_work.str.n + 3) / 4) * 4;
			break;
		case NyLPC_cJsonRpcParser_TYPE_BYTE:
			if (!isUnSignedCharSet(i_c)){
				goto ERROR;
			}
			i_inst->_pst = PARAM_ST_BYTE;
			i_inst->_work.uint32 = (i_c - '0');
			break;
		case NyLPC_cJsonRpcParser_TYPE_STRING:
			if (i_c != '"'){
				goto ERROR;
			}
			i_inst->_result->method.param_index[i_inst->_pcounter] = i_inst->_work.str.n;
			i_inst->_pcounter++;
			if (i_inst->_work.str.n >= NyLPC_TJsonRpcParserResult_PARAM_BUF){
				goto ERROR;
			}
			i_inst->_work.str.n++;//文字数の記憶領域
			i_inst->_pst = PARAM_ST_STR;
			return;//開始時に1バイト予約するから手順が違う
		case NyLPC_cJsonRpcParser_TYPE_BSTRING:
			if (i_c != '"'){
				goto ERROR;
			}
			i_inst->_result->method.param_index[i_inst->_pcounter] = i_inst->_work.str.n;
			i_inst->_pcounter++;
			if (i_inst->_work.str.n >= NyLPC_TJsonRpcParserResult_PARAM_BUF){
				goto ERROR;
			}
			i_inst->_work.str.n++;//文字数の記憶領域
			i_inst->_pst = PARAM_ST_BSTR;
			i_inst->_work.str.st = 0;
			return;//開始時に1バイト予約するから手順が違う
		default:
			goto ERROR;
		}
		i_inst->_result->method.param_index[i_inst->_pcounter] = i_inst->_work.str.n;
		i_inst->_pcounter++;
		return;
	case PARAM_ST_STR_ESCAPE_HEX:
		//16進数構成文字のみ
		if (!isxdigit((int)i_c)){
			goto ERROR;
		}
		i_inst->_work.str.vt = (i_inst->_work.str.vt << 4) | NyLPC_ctox(i_c);
		i_inst->_work.str.st++;
		//2文字目で値を確定
		if (i_inst->_work.str.st>=2){
			if (i_inst->_work.str.n >= NyLPC_TJsonRpcParserResult_PARAM_BUF){
				goto ERROR;
			}
			i_inst->_result->method.param_buf[i_inst->_work.str.n] = i_inst->_work.str.vt;
			i_inst->_work.str.n++;
			i_inst->_pst = PARAM_ST_STR;
		}
		return;
	case PARAM_ST_STR_ESCAPE:
		if(i_c=='x'){
			i_inst->_pst = PARAM_ST_STR_ESCAPE_HEX;
			i_inst->_work.str.st = 0;
			i_inst->_work.str.vt = 0;
		}
		else{
			if (i_inst->_work.str.n >= NyLPC_TJsonRpcParserResult_PARAM_BUF){
				goto ERROR;
			}
			if (!convertSingleCharEscape(i_c,&i_inst->_result->method.param_buf[i_inst->_work.str.n])){
				goto ERROR;
			}
			i_inst->_work.str.n++;
			i_inst->_pst = PARAM_ST_STR;
		}
		return;
	case PARAM_ST_STR:
		if (i_c == '"'){
			//完了
			if (i_inst->_work.str.n >= NyLPC_TJsonRpcParserResult_PARAM_BUF){
				goto ERROR;
			}
			i_inst->_result->method.param_buf[i_inst->_work.str.n] = '\0';
			i_inst->_work.str.n++;
			//文字数を更新
			*((NyLPC_TUInt8*)(i_inst->_result->method.param_buf+i_inst->_result->method.param_index[i_inst->_pcounter - 1])) = i_inst->_work.str.n - i_inst->_result->method.param_index[i_inst->_pcounter - 1]-2;
			i_inst->_pst = PARAM_ST_NEXT;
			return;
		}
		else if (i_c=='\\'){
			i_inst->_pst = PARAM_ST_STR_ESCAPE;
		}
		else{
			if (i_inst->_work.str.n >= NyLPC_TJsonRpcParserResult_PARAM_BUF){
				goto ERROR;
			}
			i_inst->_result->method.param_buf[i_inst->_work.str.n] = i_c;
			i_inst->_work.str.n++;
		}
		return;
	case PARAM_ST_BSTR:
		if (i_c == '"'){
			if (i_inst->_work.str.st != 0){
				goto ERROR;
			}
			//文字数を更新
			*((NyLPC_TUInt8*)(i_inst->_result->method.param_buf + i_inst->_result->method.param_index[i_inst->_pcounter - 1])) = i_inst->_work.str.n - i_inst->_result->method.param_index[i_inst->_pcounter - 1] - 1;
			i_inst->_pst = PARAM_ST_NEXT;
			return;
		}
		else{
			//16進数構成文字のみ
			if (!isxdigit((int)i_c)){
				goto ERROR;
			}
			switch(i_inst->_work.str.st){
			case 0:
				i_inst->_work.str.vt = NyLPC_ctox(i_c);
				i_inst->_work.str.st++;
				break;
			case 1:
				i_inst->_work.str.vt = (i_inst->_work.str.vt << 4) | NyLPC_ctox(i_c);
				if (i_inst->_work.str.n >= NyLPC_TJsonRpcParserResult_PARAM_BUF){
					goto ERROR;
				}
				i_inst->_result->method.param_buf[i_inst->_work.str.n] = i_inst->_work.str.vt;
				i_inst->_work.str.n++;
				i_inst->_work.str.st = 0;
				break;
			}
		}
		return;
	case PARAM_ST_INT:
		if (isUnSignedCharSet(i_c)){
			i_inst->_work.int32.v = i_inst->_work.int32.v * 10 + (i_c - '0');
			return;
		}
		switch (i_c){
		case ' ':
			i_inst->_pst = PARAM_ST_NEXT; break;
		case ',':
			i_inst->_pst = PARAM_ST_VAL; break;
		case ']':
			i_inst->_st = NyLPC_TcJsonRpcParser_ST_NEXT; break;
		default:
			goto ERROR;
		}
		//4バイト境界に揃える
		i_inst->_work.str.n += 4;
		if (i_inst->_work.str.n > NyLPC_TJsonRpcParserResult_PARAM_BUF){
			goto ERROR;
		}
		*((NyLPC_TInt32*)&(i_inst->_result->method.param_buf[i_inst->_work.str.n - 4])) = i_inst->_work.int32.v*i_inst->_work.int32.s;
		return;
	case PARAM_ST_UINT:
		if (isUnSignedCharSet(i_c)){
			i_inst->_work.uint32 = i_inst->_work.uint32 * 10 + (i_c - '0');
			return;
		}
		switch (i_c){
		case ' ':
			i_inst->_pst = PARAM_ST_NEXT; break;
		case ',':
			i_inst->_pst = PARAM_ST_VAL; break;
		case ']':
			i_inst->_st = NyLPC_TcJsonRpcParser_ST_NEXT; break;
		default:
			goto ERROR;
		}
		i_inst->_work.str.n += 4;
		if (i_inst->_work.str.n > NyLPC_TJsonRpcParserResult_PARAM_BUF){
			goto ERROR;
		}
		*((NyLPC_TUInt32*)&(i_inst->_result->method.param_buf[i_inst->_work.str.n - 4])) = i_inst->_work.uint32;
		return;
	case PARAM_ST_BYTE:
		if (isUnSignedCharSet(i_c)){
			i_inst->_work.uint32 = i_inst->_work.uint32 * 10 + (i_c - '0');
			return;
		}
		switch (i_c){
		case ' ':
			i_inst->_pst = PARAM_ST_NEXT; break;
		case ',':
			i_inst->_pst = PARAM_ST_VAL; break;
		case ']':
			i_inst->_st = NyLPC_TcJsonRpcParser_ST_NEXT; break;
		default:
			goto ERROR;
		}
		i_inst->_work.str.n ++;
		if (i_inst->_work.str.n > NyLPC_TJsonRpcParserResult_PARAM_BUF){
			goto ERROR;
		}
		*((NyLPC_TUInt8*)&(i_inst->_result->method.param_buf[i_inst->_work.str.n - 1])) = (NyLPC_TUInt8)(i_inst->_work.uint32&0xff);
		return;
	case PARAM_ST_NEXT:
		switch (i_c){
		case ' ':break;
		case ',':
			i_inst->_pst = PARAM_ST_VAL; break;
		case ']':
			i_inst->_st = NyLPC_TcJsonRpcParser_ST_NEXT; break;
		default:
			goto ERROR;
		}
		return;
	}
ERROR:
	i_inst->_st = NyLPC_TcJsonRpcParser_ST_ERROR;
	return;
}


/**
* NyLPC_cJsonRpcParser_putCharのサブ関数
*/
static NyLPC_TUInt8 valTerminator2St(NyLPC_TChar i_c)
{
	switch (i_c){
	case ' ':
		return NyLPC_TcJsonRpcParser_ST_NEXT;
	case '}':
		return NyLPC_TcJsonRpcParser_ST_END;
	case ',':
		return NyLPC_TcJsonRpcParser_ST_NAME_Q;
	default:
		//ないはず
		return NyLPC_TcJsonRpcParser_ST_ERROR;
	}
}

/** 文字列をパーサに入力してパーサの状態を遷移させます。

*/
void NyLPC_cJsonRpcParser_putChar(NyLPC_TcJsonRpcParser_t* i_inst,char i_c)
{
	switch(i_inst->_st){
	case NyLPC_TcJsonRpcParser_ST_PARAMS:
		putchar_params(i_inst,i_c);
		return;
	case NyLPC_TcJsonRpcParser_ST_START:
		if(i_c=='{'){
			i_inst->_st=NyLPC_TcJsonRpcParser_ST_NAME_Q;
			return;
		}
		if(strchr(" ",i_c)!=NULL){
			//無視
			return;
		}
		//エラー
		goto ERROR;
	case NyLPC_TcJsonRpcParser_ST_NAME_Q:
		if(i_c=='"'){
			i_inst->_st=NyLPC_TcJsonRpcParser_ST_NAME_STR;
			i_inst->_work.str.n=0;
			return;
		}
		if(strchr(" ",i_c)!=NULL){
			//無視
			return;
		}
		//エラー
		goto ERROR;
	case NyLPC_TcJsonRpcParser_ST_NAME_STR:
		if(i_c=='"'){
			//完了
			i_inst->_work.str.buf[i_inst->_work.str.n]='\0';
			//メソッドIDを記録
			i_inst->_name_id=NyLPC_TTextIdTbl_getMatchId(i_inst->_work.str.buf,method_name_tbl);
			i_inst->_st=NyLPC_TcJsonRpcParser_ST_NV_SEP;
			switch(i_inst->_name_id){
			case NAME_ID_METHOD:
			case NAME_ID_RESULT:
				if(i_inst->_result->type!=NyLPC_TJsonRpcParserResult_TYPE_UNKNOWN){
					goto ERROR;
				}
				break;
			case NAME_ID_PARAMS:
				if (i_inst->_pcounter != 0){
					goto ERROR;
				}
				break;
			default:
				break;
			}
			return;
		}
		i_inst->_work.str.n++;
		if(i_inst->_work.str.n>=NyLPC_TcJsonRpcParser_WORK_MAX){
			//文字列長すぎでござる。
			goto ERROR;
		}
		i_inst->_work.str.buf[i_inst->_work.str.n-1]=i_c;
		return;
	case NyLPC_TcJsonRpcParser_ST_NV_SEP:
		if(i_c==':'){
			if (i_inst->_name_id == NAME_ID_PARAMS){
				i_inst->_pst = PARAM_ST_START;
				i_inst->_st = NyLPC_TcJsonRpcParser_ST_PARAMS;
			}
			else{
				i_inst->_st = NyLPC_TcJsonRpcParser_ST_VAL;
				i_inst->_work.str.n = 0;
			}
			return;
		}
		if(strchr(" ",i_c)!=NULL){
			//無視
			return;
		}
		//エラー
		goto ERROR;
	case NyLPC_TcJsonRpcParser_ST_VAL:
		if(i_c=='"'){
			i_inst->_st=NyLPC_TcJsonRpcParser_ST_VAL_STR;
			i_inst->_work.str.n=0;
			return;
		}
		if(strchr(" ",i_c)!=NULL){
			//無視
			return;
		}
		if (isUnSignedCharSet(i_c)){
			i_inst->_work.uint32 = (i_c - '0');
			i_inst->_st=NyLPC_TcJsonRpcParser_ST_VAL_UINT;
			return;
		}
		//エラー
		goto ERROR;
	case NyLPC_TcJsonRpcParser_ST_VAL_UINT:
		if (isUnSignedCharSet(i_c)){
			i_inst->_work.uint32 = i_inst->_work.uint32 * 10 + (i_c - '0');
			return;
		}
		if(strchr(" ,}",i_c)!=NULL){
			//確定
			switch(i_inst->_name_id){
			case NAME_ID_ID:
				i_inst->_result->method.id=i_inst->_work.uint32;
				break;
			case NAME_ID_UNKNOWN:
				//知らないIDは無視
				break;
			default:
				//数値を受け入れないパラメータ
				goto ERROR;
			}
			i_inst->_st=valTerminator2St(i_c);
			return;
		}
		goto ERROR;
	case NyLPC_TcJsonRpcParser_ST_VAL_STR:
		if(i_c=='"'){
			i_inst->_work.str.buf[i_inst->_work.str.n]='\0';
			//確定
			switch(i_inst->_name_id){
			case NAME_ID_VERSION:
				if(strcmp(i_inst->_work.str.buf,"2.0")!=0){
					goto ERROR;
				}
				break;
			case NAME_ID_METHOD:
				i_inst->_result->method._type=NyLPC_TJsonRpcParserResult_TYPE_METHOD;
				i_inst->_result->method.class_def=findFunction(i_inst->_class_def,i_inst->_work.str.buf,&(i_inst->_result->method.func_number));
				if(i_inst->_result->method.class_def==NULL){
					goto ERROR;
				}
				break;
			case NAME_ID_UNKNOWN:
				//知らないIDは無視
				break;
			default:
				//文字列を受け入れないパラメータ
				goto ERROR;
			}
			i_inst->_st=NyLPC_TcJsonRpcParser_ST_NEXT;
			return;
		}
		i_inst->_work.str.n++;
		if(i_inst->_work.str.n>=NyLPC_TcJsonRpcParser_WORK_MAX){
			//文字列長すぎでござる。
			goto ERROR;
		}
		i_inst->_work.str.buf[i_inst->_work.str.n-1]=i_c;
		return;
	case NyLPC_TcJsonRpcParser_ST_NEXT:
		if(i_c==','){
			i_inst->_st=NyLPC_TcJsonRpcParser_ST_NAME_Q;
		}else if(i_c=='}'){
			i_inst->_st=NyLPC_TcJsonRpcParser_ST_END;
		}else if(strchr(" ",i_c)!=NULL){
			//nothing to do
		}else{
			goto ERROR;
		}
		return;
	default:
		goto ERROR;
	}
ERROR:
	i_inst->_st = NyLPC_TcJsonRpcParser_ST_ERROR;
	return;
}

#define COMMENT_DEBUG
#ifndef COMMENT_DEBUG

const struct NyLPC_TJsonRpcMethodDef test_method[]=
{
	{ "func" , ""},
	{ "func1", "d" },
	{ "func2", "dd" },
	{ "func3", "u" },
	{ "func4", "uu" },
	{ "func5", "uss" },
	{ "func5", "uss" },
	{ "func6", "s" },
	{ "func7", "sd" },
	{ "func8", "su" },
	{ "func9", "ss" },
	{ "f10", "suu" },
	{ "f11", "bbbb" },
	{ "f12", "usu" },
	{ "f13", "uBu" },
	{ NULL, "" }
};

const struct NyLPC_TJsonRpcClassDef test_def=
{
	"ns","class",
	test_method
};

/**
 * テスト用のアレイ
 */
const struct NyLPC_TJsonRpcClassDef* test_def_array[]=
{
	&test_def,
	NULL
};

void NyLPC_cJsonRpcParser_putText(NyLPC_TcJsonRpcParser_t* i_inst, const NyLPC_TChar* i_text,NyLPC_TUInt16 i_size)
{
	NyLPC_TUInt16 i;
	NyLPC_TUInt32 u32;
	NyLPC_TInt32 i32;
	NyLPC_TChar* c;
	NyLPC_TUInt8 u8,l8;


	for (i = 0; i < i_size; i++){
		NyLPC_cJsonRpcParser_putChar(i_inst,i_text[i]);
		if (i_inst->_st == NyLPC_TcJsonRpcParser_ST_ERROR){
			break;
		}
		else if (i_inst->_st == NyLPC_TcJsonRpcParser_ST_END){
			break;
		}
	}
	if (i_inst->_st == NyLPC_TcJsonRpcParser_ST_ERROR){
		printf("ERROR!\n");
	}
	else if (i_inst->_st == NyLPC_TcJsonRpcParser_ST_END){
		printf("OK!\n");
		for (int i = 0; i_inst->_result->method.param_index[i] != 0xff && i<i_inst->_pcounter; i++){
			switch (i_inst->_result->method.class_def->functions[i_inst->_result->method.func_number].param_patt[i]){
			case 'u':NyLPC_TJsonRpcParserResult_getUInt32(i_inst->_result,i, &u32); printf("%u,", u32); break;
			case 'd':NyLPC_TJsonRpcParserResult_getInt32(i_inst->_result, i, &i32); printf("%d,", i32); break;
			case 's':NyLPC_TJsonRpcParserResult_getStr(i_inst->_result, i, &c, &l8); printf("%s:%d,", c, l8); break;
			case 'B':NyLPC_TJsonRpcParserResult_getByteArray(i_inst->_result, i, &c, &l8); printf("--:%d,", l8); break;
			case 'b':NyLPC_TJsonRpcParserResult_getByte(i_inst->_result, i, &u8); printf("%u,", u8); break;
			}
		}
		printf("\n");
	}
	else{
		printf("CONTINUE...\n");
	}
}

void main(void)
{
	NyLPC_TcJsonRpcParser_t inst;
	union NyLPC_TJsonRpcParserResult ret;

//JSONCORE
//#define JSONCORE 1
#ifdef JSONCORE
	const char* t[] = {
		"{\"0123\"\"2.0\"}",//NO
		"{\"01234567890123456789\":\"2.0\"}",//NO
		"{\"0123\":\"01234567890123456789\"}",//NO
		"{\"version\":\"2.0\"}",//YES
		"{\"version\":\"2.1\"}",//NO
		" {  \"version\"  :  \"2.0\"  }",//YES
		"{\"version\":\"2.0\",}",//NO
		"{\"version\":\"2.0\" \"test\":\"t\"}",//NO
		"{\"version\":\"2.0\",\"method\":}",//NO
		"{\"version\":\"2.0\",\"method\":\"ns:class:func\"}",//YES
		"{\"version\":\"2.0\",\"method\":\"ns2:class:func\"}",//NO
		"{\"version\":\"2.0\",\"method\":\"ns:class2:func\"}",//NO
		"{\"version\":\"2.0\",\"method\":\"ns:class:func2\"}",//NO
		"{\"version\":\"2.0\",\"method\":\"ns:class:func\",\"method\":\"\"}",//NO
		"{\"version\":\"2.0\",\"method\":\"ns:class:func\",\"id\":0}",//YES
		"{\"version\":\"2.0\",\"method\":\"ns:class:func\",\"id\":\"123\"}",//YES
		"{\"version\":\"2.0\",\"method\":123}",//NO
		"{\"version\":\"2.0\",\"test\":123}",//YES
		"{\"version\":\"2.0\",\"test\":123, \"test\":123 }",//YES
		"{\"version\":\"2.0\",\"test\":123 ,\"test\":123 }",//YES
		NULL
	};
#else
	const char* t[] = {
/*		"{\"version\":\"2.0\",\"method\":\"ns:class:func\",\"params\":[]}",//YES
		"{\"version\":\"2.0\",\"method\":\"ns:class:func\",\"params\": [] }",//YES
		"{\"version\":\"2.0\",\"method\":\"ns:class:func1\",\"params\": [-123] }",//YES
		"{\"version\":\"2.0\",\"method\":\"ns:class:func1\",\"params\": [\"123\"] }",//NG
		"{\"version\":\"2.0\",\"method\":\"ns:class:func2\",\"params\": [ 1 , 456 ] }",//YES
		"{\"version\":\"2.0\",\"method\":\"ns:class:func2\",\"params\": [ 123 , 456 a] }",//NG
		"{\"version\":\"2.0\",\"method\":\"ns:class:func2\",\"params\": [ 123a] }",//NG
		"{\"version\":\"2.0\",\"method\":\"ns:class:func2\",\"params\": [ 123,456] }",//OK
		"{\"version\":\"2.0\",\"method\":\"ns:class:func2\",\"params\": [123,456] }",//YES
		"{\"version\":\"2.0\",\"method\":\"ns:class:func3\",\"params\": [123] }",//YES
		"{\"version\":\"2.0\",\"method\":\"ns:class:func3\",\"params\": [\"123\"] }",//NG
		"{\"version\":\"2.0\",\"method\":\"ns:class:func4\",\"params\": [ 123 , 456 ] }",//YES
		"{\"version\":\"2.0\",\"method\":\"ns:class:func4\",\"params\": [ 123 , 456 a] }",//NG
		"{\"version\":\"2.0\",\"method\":\"ns:class:func4\",\"params\": [ 123a] }",//NG
		"{\"version\":\"2.0\",\"method\":\"ns:class:func4\",\"params\": [ 123,456] }",//OK
		"{\"version\":\"2.0\",\"method\":\"ns:class:func5\",\"params\": [123, \"abc\" , \"abc\"] }",//OK
		"{\"version\":\"2.0\",\"method\":\"ns:class:func6\",\"params\": [\"01234567890123456789012\"] }",//OK
		"{\"version\":\"2.0\",\"method\":\"ns:class:func6\",\"params\": [1] }",//NG
		"{\"version\":\"2.0\",\"method\":\"ns:class:func8\",\"params\": [\"012345678901234567\",1,1] }",//OK
		"{\"version\":\"2.0\",\"method\":\"ns:class:func8\",\"params\": [\"012345678901234567\",1] }",//OK
		"{\"version\":\"2.0\",\"method\":\"ns:class:func7\",\"params\": [\"012345678901234567\",1] }",//OK
		"{\"version\":\"2.0\",\"method\":\"ns:class:f10\",\"params\": [\"01234567890123\",22,33] }",//OK
		"{\"version\":\"2.0\",\"method\":\"ns:class:f11\",\"params\": [ 0 , 1 ,2,3] }",//OK
		"{\"version\":\"2.0\",\"method\":\"ns:class:f11\",\"params\": [ 15 , 1 ,2,3a] }",//NG
*/
		"{\"version\":\"2.0\",\"method\":\"ns:class:f13\",\"params\": [2,\"0011223344ff\",1] }",//NG
//		"{\"version\":\"2.0\",\"method\":\"ns:class:f12\",\"params\": [2,\"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789\",1] }",//NG
		NULL
	};
#endif
	int i;

	for (i = 0;t[i]!=NULL; i++){
		NyLPC_cJsonRpcParser_initialize(&inst, test_def_array);
		NyLPC_cJsonRpcParser_initParser(&inst, &ret);
		NyLPC_cJsonRpcParser_putText(&inst, t[i], strlen(t[i]));
		continue;
	}
	printf("end\n");
	return;

}
#endif

