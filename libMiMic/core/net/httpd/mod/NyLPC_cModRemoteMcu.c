/*********************************************************************************
 * PROJECT: MiMic
 * --------------------------------------------------------------------------------
 *
 * This file is part of MiMic
 * Copyright (C)2011 Ryo Iizuka
 *
 * MiMic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by　the Free Software Foundation, either version 3 of the　License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information please contact.
 *  http://nyatla.jp/
 *  <airmail(at)ebony.plala.or.jp> or <nyatla(at)nyatla.jp>
 *
 *********************************************************************************/
#include "NyLPC_cModRemoteMcu.h"
#include "NyLPC_stdlib.h"
#include "NyLPC_http.h"
#include "NyLPC_mimicVm.h"
#include "../NyLPC_cHttpdConnection_protected.h"
#include "../NyLPC_cHttpdUtils.h"
#include "NyLPC_net.h"

#define MVM_VERSION "ModRemoteMcu/1.0;Json/1.0"
#define SIZE_OF_IBUF 256
struct TModMiMicRemoteMcuHeader
{
    struct NyLPC_THttpBasicHeader super;
    NyLPC_TUInt8 _content_id;
    //解析用
    NyLPC_TUInt8 _qery_name_id;
    NyLPC_TUInt8 _astate;
    NyLPC_TInt16 _prefix_len;
    NyLPC_TcStr_t _tstr;
    NyLPC_TChar _tstr_buf[16];
    struct NyLPC_TUInt32ArrayPtr _binarray;
    /** 文字列のパーサ*/
    NyLPC_TcMiMicDbCompiler_t _binparser;
    NyLPC_TcMiMicTxtCompiler_t _txtcmp;
    union{
        struct{
            NyLPC_TUInt8 v;//バージョン
            NyLPC_TUInt8 o;//outputスタイル
            /**
             * il_bufはbcとdbの2パートのデータを格納します。
             * 先頭からbc_lenの長さのBCパートと、db_partからdb_lenの長さのデータです。
             */
            struct{
                /** MiMicVMインストラクションの蓄積用。前半にTXT,後半にDBを格納する。 */
                NyLPC_TUInt32 bc_buf[SIZE_OF_IBUF];
                /** MiMicVM入力ストリーム(MimicDB)の開始位置(bufの一部を指す) */
                const NyLPC_TUInt32* db_part;
                /** MiMicTXTのワード長(1ワード32bit)*/
                NyLPC_TUInt16 txt_len;
                /** MiMicDBのワード長(1ワード32bit)*/
                NyLPC_TUInt16 db_len;
            }vm_instruction;
        }mvm;
        struct{
            /**
             * 不明な名前の場合は、ここに名前をコピー
             */
            NyLPC_TChar path[32];
        }unknown;
    }content;
};


static void mvm(NyLPC_TcHttpdConnection_t* i_connection,const struct TModMiMicRemoteMcuHeader* i_rqh);
static void status(NyLPC_TcHttpdConnection_t* i_connection);




#define ST_PARSE_PATH 1
#define ST_PARSE_QUERY_NAME 2
#define ST_PARSE_QUERY_VALUE 3      //Query読み出し中
#define ST_PARSE_QUERY_VALUE_V 4
#define ST_PARSE_QUERY_VALUE_O 5
#define ST_PARSE_QUERY_VALUE_BC 6
#define ST_PARSE_QUERY_VALUE_DB 7
/**
 * コンテンツID定義(コンテンツ名に対応)
 */
#define CONTENT_ID_MVM   2
#define CONTENT_ID_STATUS  3
#define CONTENT_ID_UNKNOWN 0


#define QNAME_ID_V  1
#define QNAME_ID_O  2
#define QNAME_ID_BC 3
#define QNAME_ID_UNKNOWN 0

/**
 * TRemoteMcuRequest.content.mvm.oの値
 */
#define QVAL_O_UNKNOWN 0    //default
#define QVAL_O_XML     1
#define QVAL_O_JSON    2

#define QVAL_V_UNKNOWN  0
#define QVAL_V_1        1


static const struct NyLPC_TTextIdTbl url_tbl[]=
{
    {"mvm.api",CONTENT_ID_MVM},
    {"status.api",CONTENT_ID_STATUS},
    {NULL,CONTENT_ID_UNKNOWN}
};

static const struct NyLPC_TTextIdTbl qname_id_table[]=
{
    {"o",QNAME_ID_O},
    {"bc",QNAME_ID_BC},
    {"v",QNAME_ID_V},
    {NULL,QNAME_ID_UNKNOWN}
};



static NyLPC_TBool urlHandler(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
    NyLPC_TUInt16 ol;
    struct TModMiMicRemoteMcuHeader* out=(struct TModMiMicRemoteMcuHeader*)o_out;
    //読み飛ばし
    if(out->_prefix_len<0){
        out->_prefix_len++;
        return NyLPC_TBool_TRUE;//読み飛ばし
    }
    if(out->_astate==ST_PARSE_PATH){
        if(i_c!='\0' && i_c!='?'){
            if(!NyLPC_cStr_put(&(out->_tstr),i_c)){
                //ERROR
                NyLPC_OnErrorGoto(ERROR);
            }
        }else{
            out->_content_id=NyLPC_TTextIdTbl_getMatchId(NyLPC_cStr_str(&(out->_tstr)),url_tbl);
            switch(out->_content_id)
            {
            case CONTENT_ID_MVM:
                out->content.mvm.vm_instruction.txt_len=0;
                out->content.mvm.vm_instruction.db_len=0;
                out->content.mvm.vm_instruction.db_part=NULL;
                NyLPC_TUInt32ArrayPtr_setBuf(&out->_binarray,out->content.mvm.vm_instruction.bc_buf,SIZE_OF_IBUF);
                out->content.mvm.o=QVAL_O_JSON;
                out->content.mvm.v=QVAL_V_UNKNOWN;
                break;
            default:
                break;
            }
            NyLPC_cStr_clear(&(out->_tstr));
            out->_astate=ST_PARSE_QUERY_NAME;//クエリ名解析へ
        }
        return NyLPC_TBool_TRUE;
    }
    switch(out->_content_id)
    {
    case CONTENT_ID_MVM:
        switch(out->_astate){
        case ST_PARSE_QUERY_NAME:
            if(i_c!='\0' && i_c!='&' && i_c!='='){
                if(!NyLPC_cStr_put(&(out->_tstr),i_c)){
                    NyLPC_OnErrorGoto(ERROR);
                }
            }else{
                //Query確定。
                out->_qery_name_id=NyLPC_TTextIdTbl_getMatchId(NyLPC_cStr_str(&(out->_tstr)),qname_id_table);
                NyLPC_cStr_clear(&(out->_tstr));
                //クエリ値がある場合
                switch(out->_qery_name_id){
                case QNAME_ID_O:
                    out->_astate=ST_PARSE_QUERY_VALUE_O;//MIMICBCのDBパラメータパーサを借用。
                    break;
                case QNAME_ID_V:
                    out->_astate=ST_PARSE_QUERY_VALUE_V;
                    break;
                case QNAME_ID_BC:
                    out->_astate=ST_PARSE_QUERY_VALUE_BC;
                    break;
                default:
                    out->_astate=ST_PARSE_QUERY_VALUE;
                    break;
                }
            }
            return NyLPC_TBool_TRUE;
        case ST_PARSE_QUERY_VALUE:
            //未知のクエリは無視
            if(i_c!='\0' && i_c!='&'){
            }else{
                //クエリ値解析完了
                out->_astate=ST_PARSE_QUERY_NAME;
            }
            return NyLPC_TBool_TRUE;
        case ST_PARSE_QUERY_VALUE_O:
            if(i_c!='\0' && i_c!='&'){
                if(!NyLPC_cStr_put(&(out->_tstr),i_c)){
                    NyLPC_OnErrorGoto(ERROR);
                }
            }else{
                if(NyLPC_cStr_isEqual(&(out->_tstr),"j")){
                    out->content.mvm.o=QVAL_O_JSON;
                }else if(NyLPC_cStr_isEqual(&(out->_tstr),"x")){
                    out->content.mvm.o=QVAL_O_XML;
                }
                out->_astate=ST_PARSE_QUERY_NAME;
                NyLPC_cStr_clear(&(out->_tstr));
            }
            return NyLPC_TBool_TRUE;
        case ST_PARSE_QUERY_VALUE_V:
            if(i_c!='\0' && i_c!='&'){
                if(!NyLPC_cStr_put(&(out->_tstr),i_c)){
                    NyLPC_OnErrorGoto(ERROR);
                }
            }else{
                if(NyLPC_cStr_isEqual(&(out->_tstr),"1")){
                    out->content.mvm.v=QVAL_V_1;
                }
                out->_astate=ST_PARSE_QUERY_NAME;
                NyLPC_cStr_clear(&(out->_tstr));
            }
            return NyLPC_TBool_TRUE;
        case ST_PARSE_QUERY_VALUE_BC:
            if(i_c!='\0' && i_c!='&'){
                //コンパイル
                switch(NyLPC_cMiMicTxtCompiler_compileFragment2(&(out->_txtcmp),i_c,&(out->_binarray),&ol))
                {
                case NyLPC_TcMiMicTxtCompiler_RET_OK:
                    //命令確定。
                    break;
                case NyLPC_TcMiMicTxtCompiler_RET_OK_END:
                    //命令終端検出->モード切替
                    out->content.mvm.vm_instruction.txt_len=SIZE_OF_IBUF-out->_binarray.len+ol;
                    out->content.mvm.vm_instruction.db_part=out->content.mvm.vm_instruction.bc_buf+out->content.mvm.vm_instruction.txt_len;
                    out->_astate=ST_PARSE_QUERY_VALUE_DB;
                    break;
                case NyLPC_TcMiMicTxtCompiler_RET_CONTINUE:
                    //何もしない
                    break;
                case NyLPC_TcMiMicTxtCompiler_RET_NG:
                default:
                    //ERROR
                    NyLPC_OnErrorGoto(ERROR);
                }
            }
            return NyLPC_TBool_TRUE;
//          //フラグメント終端が検出できない終了はエラー
//          NyLPC_OnErrorGoto(ERROR);
        case ST_PARSE_QUERY_VALUE_DB:
            if(i_c!='\0' && i_c!='&'){
                switch(NyLPC_cMiMicDbCompiler_compileFragment2(&(out->_binparser),i_c,out->_binarray.ptr))
                {
                case NyLPC_TcMiMicDbCompiler_RET_CONTINUE:
                    break;
                case NyLPC_TcMiMicDbCompiler_RET_OK:
                    //
                    if(!NyLPC_TUInt32ArrayPtr_seek(&(out->_binarray),1)){
                        //ERROR
                        NyLPC_OnErrorGoto(ERROR);
                    }
                    break;
                case NyLPC_TcMiMicDbCompiler_RET_ERROR:
                default:
                    //ERROR
                    NyLPC_OnErrorGoto(ERROR);
                }
            }else{
                //区切りのいいところで終わってる？
                if(NyLPC_cMiMicDbCompiler_hasFragment(&(out->_binparser))){
                    //ERROR
                    NyLPC_OnErrorGoto(ERROR);
                }
                out->content.mvm.vm_instruction.db_len=((NyLPC_TUInt8*)(out->_binarray.ptr)-(NyLPC_TUInt8*)(out->content.mvm.vm_instruction.db_part))/sizeof(NyLPC_TUInt32);

                //終端しているなら、次のクエリへ
                out->_astate=ST_PARSE_QUERY_NAME;
            }
            return NyLPC_TBool_TRUE;
        default:
            break;
        }
        NyLPC_OnErrorGoto(ERROR);
    default:
        NyLPC_OnErrorGoto(ERROR);
    }
    return NyLPC_TBool_TRUE;
ERROR:
    return NyLPC_TBool_FALSE;
}
/**
 * デフォルトハンドラ
 */
static const struct NyLPC_TcHttpBasicHeaderParser_Handler handler=
{
    NULL,
    urlHandler
};


/**
 * コンストラクタ。
 */
void NyLPC_cModRemoteMcu_initialize(NyLPC_TcModRemoteMcu_t* i_inst,const NyLPC_TChar* i_ref_root_path)
{
    NyLPC_cModRomFiles_initialize(&i_inst->super,i_ref_root_path,NULL,0);
}
void NyLPC_cModRemoteMcu_finalize(NyLPC_TcModRemoteMcu_t* i_inst)
{
    NyLPC_cModRomFiles_finalize(&i_inst->super);
}
/**
 * モジュールがコネクションをハンドリングできるかを返します。
 */
NyLPC_TBool NyLPC_cModRemoteMcu_canHandle(NyLPC_TcModRemoteMcu_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection)
{
    return NyLPC_cModRomFiles_canHandle(&i_inst->super,i_connection);
}

static struct TModMiMicRemoteMcuHeader single_header;

/**
 * モジュールを実行します。
 */
NyLPC_TBool NyLPC_cModRemoteMcu_execute(NyLPC_TcModRemoteMcu_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection)
{
    NyLPC_TcHttpBasicHeaderParser_t parser;
    NyLPC_TUInt8 method_type;
    //リクエストParse済へ遷移(この関数の後はModが責任を持ってリクエストを返却)
    NyLPC_cHttpdConnection_setReqStatusParsed(i_connection);


    //VM起動の為の排他ロック
    NyLPC_cHttpdConnection_lock(i_connection);


    //URL解析の準備
    single_header._prefix_len=-((NyLPC_TInt16)strlen(i_inst->super._ref_root_path)+2);
    single_header._astate=ST_PARSE_PATH;
    NyLPC_cStr_initialize(&single_header._tstr,single_header._tstr_buf,16);
    NyLPC_cMiMicDbCompiler_initialize(&single_header._binparser);
    NyLPC_cMiMicTxtCompiler_initialize(&single_header._txtcmp);

    NyLPC_cHttpBasicHeaderParser_initialize(&parser,&handler);


    //プリフェッチしたデータを流す
    NyLPC_cHttpBasicHeaderParser_parseInit(&parser,&(single_header.super));
    NyLPC_cHttpdConnection_pushPrefetchInfo(i_connection,&parser,&single_header.super);
    //後続をストリームから取り込む
    if(!NyLPC_cHttpBasicHeaderParser_parseStream(&parser,NyLPC_cHttpdConnection_refStream(i_connection),&(single_header.super))){
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
        NyLPC_OnErrorGoto(Error1);
    }
    if(!NyLPC_cHttpBasicHeaderParser_parseFinish(&parser,&(single_header.super))){
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
        NyLPC_OnErrorGoto(Error1);
    }
    //GETかHEADに制限
    method_type=NyLPC_cHttpdConnection_getMethod(i_connection);
    if(method_type!=NyLPC_THttpMethodType_GET && method_type!=NyLPC_THttpMethodType_HEAD)
    {
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,405);
        NyLPC_OnErrorGoto(Error1);
    }
    //Request::ConnectionがClose設定,又はHTTP1.1では無い場合,CLOSE
    if(single_header.super.connection==NyLPC_THttpMessgeHeader_Connection_CLOSE || single_header.super.startline.req.version!=NyLPC_THttpVersion_11)
    {
        NyLPC_cHttpdConnection_setConnectionMode(i_connection,NyLPC_TcHttpdConnection_CONNECTION_MODE_CLOSE);
    }
    //CGIの実行
    switch(single_header._content_id)
    {
    case CONTENT_ID_MVM:
        mvm(i_connection,&single_header);
        break;
    case CONTENT_ID_STATUS:
        status(i_connection);
        break;
    default:
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,400);
        NyLPC_OnErrorGoto(Error1);
    }
    NyLPC_cStr_finalize(&single_header._tstr);
    NyLPC_cMiMicDbCompiler_finalize(&single_header._binparser);
    NyLPC_cMiMicTxtCompiler_finalize(&single_header._txtcmp);
    NyLPC_cHttpBasicHeaderParser_finalize(&parser);
//占有解除
    NyLPC_cHttpdConnection_unlock(i_connection);
    return NyLPC_TBool_TRUE;
Error1:
    NyLPC_cStr_finalize(&single_header._tstr);
    NyLPC_cMiMicDbCompiler_finalize(&single_header._binparser);
    NyLPC_cMiMicTxtCompiler_finalize(&single_header._txtcmp);
    NyLPC_cHttpBasicHeaderParser_finalize(&parser);
    //VM排他ロックの解除
    NyLPC_cHttpdConnection_unlock(i_connection);
    return NyLPC_TBool_FALSE;
}


/**
 * イベントハンドラを継承
 */
struct TVmEventHandler
{
    struct NyLPC_TcMiMicVM_TEvent super;
    const struct TModMiMicRemoteMcuHeader* req;
    NyLPC_TcHttpdConnection_t* connection;
    NyLPC_TUInt16 db_pos;
    /** ストリームへ出力したデータの数*/
    NyLPC_TUInt16 st_len;
    /** Bodyを送信するかのフラグ*/
    NyLPC_TBool is_send_body;
};

/**
 * ストリームハンドラ(put)
 */
static NyLPC_TBool mvmputs_json(struct NyLPC_TcMiMicVM_TEvent* i_eh,NyLPC_TUInt32 i_val)
{
    struct TVmEventHandler* eh=(struct TVmEventHandler*)i_eh;
    if(eh->is_send_body){
        if(eh->st_len>0){
            eh->st_len++;
            return NyLPC_cHttpdConnection_sendResponseBodyF(eh->connection,",%u",i_val);
        }else{
            eh->st_len++;
            return NyLPC_cHttpdConnection_sendResponseBodyF(eh->connection,"%u",i_val);
        }
    }
    return NyLPC_TBool_TRUE;
}

/**
 * ストリームハンドラ(get)
 */
static NyLPC_TBool mvmgets(struct NyLPC_TcMiMicVM_TEvent* i_eh,NyLPC_TUInt32* o_val)
{
    struct TVmEventHandler* eh=(struct TVmEventHandler*)i_eh;
    //読み出し済みDBサイズの確認
    if(eh->req->content.mvm.vm_instruction.db_len<=eh->db_pos){
        //読めない
        return NyLPC_TBool_FALSE;
    }
    *o_val=eh->req->content.mvm.vm_instruction.db_part[eh->db_pos];
    eh->db_pos++;
    return NyLPC_TBool_TRUE;
}
/**
 * ネイティブCALLハンドラ
 */
static NyLPC_TUInt32 nativeCall(struct NyLPC_TcMiMicVM_TEvent* i_evh,NyLPC_TUInt32 i_id,NyLPC_TcMiMicVM_t* i_vm)
{
    (void)i_evh;
//  NyLPC_TNativeFunction f=getNativeFunctionById(i_id);
//  if(f==NULL){
//      return NyLPC_cMiMicVM_RESULT_RUNTIME_NG_UNKNOWN_CALL;
//  }
//  return f(i_vm)?NyLPC_cMiMicVM_RESULT_OK:NyLPC_cMiMicVM_RESULT_RUNTIME_NG_CALL;
    return NyLPC_cMiMicVM_RESULT_RUNTIME_NG_CALL;
}


static void mvmsleep(struct NyLPC_TcMiMicVM_TEvent* i_eh,NyLPC_TUInt32 i_sleep_in_msec)
{
    (void)i_eh;
    NyLPC_cThread_sleep(i_sleep_in_msec);
}

/**
 * RemoteMCUのステータスを返す。基本的にjson
 * {
 *  application:"[VERSION]"
 * }
 */
static void status(NyLPC_TcHttpdConnection_t* i_connection)
{
    if(!NyLPC_cHttpdUtils_sendJsonHeader(i_connection)){
        return;
    }
    //JSONを書く。
    if(NyLPC_cHttpdConnection_getMethod(i_connection)==NyLPC_THttpMethodType_GET){
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,"{\"application\":\""MVM_VERSION"\"}");
    }
    return;
}
/**
 * MimicVMの起動と,ResponseJSONの起動
 * @return
 * 持続性接続を継続するかの真偽値
 */
static void mvm(NyLPC_TcHttpdConnection_t* i_connection,const struct TModMiMicRemoteMcuHeader* i_rqh)
{
    struct TVmEventHandler he;
    NyLPC_TcMiMicVM_t vm;
    NyLPC_TUInt32 vmret;
    if(i_rqh->content.mvm.v!=QVAL_V_1 || i_rqh->content.mvm.o!=QVAL_O_JSON)
    {
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,400);
        return;
    }

    //Bodyを書く
    //ハンドラインスタンスの設定
    if(!NyLPC_cHttpdUtils_sendJsonHeader(i_connection)){
        NyLPC_OnErrorGoto(Error1);
    }

    he.super.get_stream=mvmgets;
    he.super.put_stream=mvmputs_json;
    he.super.native_call=nativeCall;
    he.super.sleep=mvmsleep;
    he.db_pos=0;
    he.st_len=0;
    he.connection=i_connection;
    he.req=i_rqh;
    he.is_send_body=(NyLPC_cHttpdConnection_getMethod(i_connection)==NyLPC_THttpMethodType_GET);

    //起動VMの初期化
    NyLPC_cMiMicVM_initialize(&vm,(struct NyLPC_TcMiMicVM_TEvent*)&(he.super));

    //JSONを書く。
    if(he.is_send_body){
        if(!NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,"{\"version\":\""MVM_VERSION"\",\"stream\":[")){
            NyLPC_OnErrorGoto(Error1);
        }
    }
    //VMの実行
    vmret=NyLPC_cMiMicVM_run(&(vm),i_rqh->content.mvm.vm_instruction.bc_buf,i_rqh->content.mvm.vm_instruction.txt_len);
    //only GET method
    if(he.is_send_body){
        if(!NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,"],\"result\":%u}",vmret)){
            NyLPC_OnErrorGoto(Error1);
        }
    }
    NyLPC_cMiMicVM_finalize(&vm);
    return;
Error1:
    NyLPC_cMiMicVM_finalize(&vm);
    return;
}

