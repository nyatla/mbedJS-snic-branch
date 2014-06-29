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
#include "NyLPC_cModMiMicSetting.h"
#include "NyLPC_stdlib.h"
#include "../NyLPC_cHttpdUtils.h"
#include "NyLPC_http.h"
#include "NyLPC_mimicVm.h"
#include "NyLPC_flash.h"
#include "../NyLPC_cHttpdConnection_protected.h"
#include "../../NyLPC_cNet.h"
//#include <ctype.h>

#define MOD_VERSION "ModMiMicSetting/1.4"
#define SIZE_OF_SETUP_PARAM 8
struct TModMiMicSettingRequest
{
    struct NyLPC_THttpBasicHeader super;
    NyLPC_TUInt8 _content_id;
    //解析用
    NyLPC_TUInt8 _qery_name_id;
    NyLPC_TUInt8 _astate;
    NyLPC_TInt16 _prefix_len;
    NyLPC_TcStr_t _tstr;
    NyLPC_TChar _tstr_buf[16];
    /** 文字列のパーサ*/
    NyLPC_TcMiMicDbCompiler_t _binparser;
    union{
        /**
         * スタックサイズ削減のための構造体。tmpは受信処理に使用。
         * memimgはFlashへ書き込むときに使用。
         */
        union{
            /**
             * 受信用構造体。host_name,param_buf[6]までのデータは、memimgのfast_boot以降のデータ構造と位置をあわせてください。
             * param_buf[7]以降については値変換必須
             * tmpにデータを作成後にmemimgへ整形して書きこむかんじ。
             */
            struct{
                NyLPC_TUInt16 param_len;
                NyLPC_TUInt16 host_len;
                NyLPC_TChar host_name[NyLPC_TcNetConfig_HOSTNAME_LEN];
                /**pパラメータ。最大長さは16。
                 * 詳細はNyLPC_TcModMiMicSetting_tを参照
                 */
                NyLPC_TUInt32 param_buf[SIZE_OF_SETUP_PARAM];
                NyLPC_TUInt32 cval;//コマンド値
            }tmp;
            /**
             * 書き込み用構造体
             */
            struct NyLPC_TMiMicConfigulation memimg;
        }setup;
        struct{
            /**
             * 不明な名前の場合は、ここに名前をコピー
             */
            NyLPC_TChar path[32];
        }unknown;
    }content;
};

#define ST_PARSE_PATH 1
#define ST_PARSE_QUERY_NAME 2
#define ST_PARSE_QUERY_VALUE 3      //Query読み出し中
#define ST_PARSE_QUERY_VALUE_P 4
#define ST_PARSE_QUERY_VALUE_C 5
#define ST_PARSE_QUERY_VALUE_HOST 6
/**
 * コンテンツID定義(コンテンツ名に対応)
 */
#define CONTENT_ID_UNKNOWN 1
#define CONTENT_ID_SETUP   2
#define CONTENT_ID_INDEX   3
#define CONTENT_ID_STATUS  4
#define CONTENT_ID_CSS     5
#define CONTENT_ID_LOGO    6

#define QNAME_ID_P  4
#define QNAME_ID_C  5
#define QNAME_ID_HOST   6
#define QNAME_ID_UNKNOWN 0

#define QVAL_C_GET 1
#define QVAL_C_UPDATE 2
#define QVAL_C_UNKNOWN 0


static const struct NyLPC_TTextIdTbl url_tbl[]=
{
    {"setup.api",CONTENT_ID_SETUP},
    {NULL,CONTENT_ID_UNKNOWN}
};

static const struct NyLPC_TTextIdTbl qname_id_table[]=
{
    {"p",QNAME_ID_P},
    {"c",QNAME_ID_C},
    {"host",QNAME_ID_HOST},
    {NULL,QNAME_ID_UNKNOWN}
};




static NyLPC_TBool urlHandler(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{

    struct TModMiMicSettingRequest* out=(struct TModMiMicSettingRequest*)o_out;
    //読み飛ばし
    if(out->_prefix_len<0){
        out->_prefix_len++;
        return NyLPC_TBool_TRUE;//読み飛ばし
    }
    if(out->_astate==ST_PARSE_PATH){
        if(i_c!='\0' && i_c!='?'){
            if(!NyLPC_cStr_put(&(out->_tstr),i_c)){
                NyLPC_OnErrorGoto(ERROR);
            }
            return NyLPC_TBool_TRUE;
        }
        out->_content_id=NyLPC_TTextIdTbl_getMatchId(NyLPC_cStr_str(&(out->_tstr)),url_tbl);
        switch(out->_content_id)
        {
        case CONTENT_ID_SETUP:
            out->content.setup.tmp.param_len=0;//クエリが無い場合の初期値
            out->content.setup.tmp.host_len =0;//クエリが無い場合の初期値
            out->content.setup.tmp.cval=QVAL_C_UNKNOWN;
            break;
        default:
            break;
        }
        NyLPC_cStr_clear(&(out->_tstr));
        out->_astate=ST_PARSE_QUERY_NAME;//クエリ名解析へ
        return NyLPC_TBool_TRUE;
    }
    switch(out->_content_id)
    {
    case CONTENT_ID_SETUP:
        switch(out->_astate){
        case ST_PARSE_QUERY_NAME:
            if(i_c!='\0' && i_c!='&' && i_c!='='){
                if(!NyLPC_cStr_put(&(out->_tstr),i_c)){
                    NyLPC_OnErrorGoto(ERROR);
                }
                return NyLPC_TBool_TRUE;
            }
            //Query確定。
            out->_qery_name_id=NyLPC_TTextIdTbl_getMatchId(NyLPC_cStr_str(&(out->_tstr)),qname_id_table);
            NyLPC_cStr_clear(&(out->_tstr));
            //クエリ値がある場合
            switch(out->_qery_name_id){
            case QNAME_ID_P:
                out->_astate=ST_PARSE_QUERY_VALUE_P;//MIMICBCのDBパラメータパーサを借用。
                out->content.setup.tmp.param_len=0;
                break;
            case QNAME_ID_C:
                out->_astate=ST_PARSE_QUERY_VALUE_C;
                break;
            case QNAME_ID_HOST:
                out->_astate=ST_PARSE_QUERY_VALUE_HOST;//_host_nameに蓄積
                out->content.setup.tmp.host_len=0;
                break;
            default:
                out->_astate=ST_PARSE_QUERY_VALUE;
                break;
            }
            return NyLPC_TBool_TRUE;
        case ST_PARSE_QUERY_VALUE:
            //未知のクエリは無視
            if(i_c!='\0' && i_c!='&'){
                return NyLPC_TBool_TRUE;
            }
            //クエリ値解析完了
            out->_astate=ST_PARSE_QUERY_NAME;
            return NyLPC_TBool_TRUE;
        case ST_PARSE_QUERY_VALUE_HOST:
            //未知のクエリは無視
            if(i_c!='\0' && i_c!='&'){
                //許可する文字列は、[:AlNum:]||'_'
                if(!isalnum(i_c) && i_c!='_'){
                    NyLPC_OnErrorGoto(ERROR);
                }
                out->content.setup.tmp.host_name[out->content.setup.tmp.host_len++]=i_c;
                if(out->content.setup.tmp.host_len>=NyLPC_TcNetConfig_HOSTNAME_LEN){
                    //長すぎ
                    NyLPC_OnErrorGoto(ERROR);
                }
                return NyLPC_TBool_TRUE;
            }
            //クエリ値解析完了
            out->content.setup.tmp.host_name[out->content.setup.tmp.host_len]='\0';
            out->_astate=ST_PARSE_QUERY_NAME;
            return NyLPC_TBool_TRUE;
        case ST_PARSE_QUERY_VALUE_C:
            if(i_c!='\0' && i_c!='&'){
                if(!NyLPC_cStr_put(&(out->_tstr),i_c)){
                    NyLPC_OnErrorGoto(ERROR);
                }
                return NyLPC_TBool_TRUE;
            }
            if(NyLPC_cStr_isEqual(&out->_tstr,"get")){
                out->content.setup.tmp.cval=QVAL_C_GET;
            }else if(NyLPC_cStr_isEqual(&out->_tstr,"update")){
                out->content.setup.tmp.cval=QVAL_C_UPDATE;
            }else{
                NyLPC_OnErrorGoto(ERROR);
            }
            out->_astate=ST_PARSE_QUERY_NAME;
            NyLPC_cStr_clear(&(out->_tstr));
            return NyLPC_TBool_TRUE;
        case ST_PARSE_QUERY_VALUE_P:
            if(i_c!='\0' && i_c!='&'){
                if(out->content.setup.tmp.param_len>=SIZE_OF_SETUP_PARAM)
                {
                    NyLPC_OnErrorGoto(ERROR);
                }
                switch(NyLPC_cMiMicDbCompiler_compileFragment2(&(out->_binparser),i_c,out->content.setup.tmp.param_buf+out->content.setup.tmp.param_len))
                {
                case NyLPC_TcMiMicDbCompiler_RET_CONTINUE:
                    break;
                case NyLPC_TcMiMicDbCompiler_RET_OK:
                    out->content.setup.tmp.param_len++;
                    break;
                case NyLPC_TcMiMicDbCompiler_RET_ERROR:
                default:
                    //ERROR
                    NyLPC_OnErrorGoto(ERROR);
                }
                return NyLPC_TBool_TRUE;
            }
            //区切りのいいところで終わってる？
            if(NyLPC_cMiMicDbCompiler_hasFragment(&(out->_binparser))){
                //ERROR
                NyLPC_OnErrorGoto(ERROR);
            }
            //終端しているなら、次のクエリへ
            out->_astate=ST_PARSE_QUERY_NAME;
            NyLPC_cStr_clear(&(out->_tstr));
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
void NyLPC_cModMiMicSetting_initialize(NyLPC_TcModMiMicSetting_t* i_inst,const NyLPC_TChar* i_ref_root_path)
{
    NyLPC_cModRomFiles_initialize(&i_inst->super,i_ref_root_path,NULL,0);
}
void NyLPC_cModMiMicSetting_finalize(NyLPC_TcModMiMicSetting_t* i_inst)
{
    NyLPC_cModRomFiles_finalize(&i_inst->super);
}
/**
 * モジュールがコネクションをハンドリングできるかを返します。
 */
NyLPC_TBool NyLPC_cModMiMicSetting_canHandle(NyLPC_TcModMiMicSetting_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection)
{
    return NyLPC_cModRomFiles_canHandle(&i_inst->super,i_connection);
}



static void setup_proc(NyLPC_TcHttpdConnection_t* i_connection,struct TModMiMicSettingRequest* i_req);

/**
 * モジュールを実行します。
 */
NyLPC_TBool NyLPC_cModMiMicSetting_execute(NyLPC_TcModMiMicSetting_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection)
{
    NyLPC_TUInt8 method_type;
    struct TModMiMicSettingRequest header;
    NyLPC_TcHttpBasicHeaderParser_t parser;

    //リクエストParse済へ遷移(この関数の後はModが責任を持ってリクエストを返却)
    NyLPC_cHttpdConnection_setReqStatusParsed(i_connection);

    //URL解析の準備
    header._prefix_len=-((NyLPC_TInt16)strlen(i_inst->super._ref_root_path)+2);
    header._astate=ST_PARSE_PATH;
    NyLPC_cStr_initialize(&header._tstr,header._tstr_buf,16);
    NyLPC_cMiMicDbCompiler_initialize(&header._binparser);

    NyLPC_cHttpBasicHeaderParser_initialize(&parser,&handler);
    //プリフェッチしたデータを流す
    NyLPC_cHttpBasicHeaderParser_parseInit(&parser,&(header.super));
    NyLPC_cHttpdConnection_pushPrefetchInfo(i_connection,&parser,&header.super);
    //後続をストリームから取り込む
    if(!NyLPC_cHttpBasicHeaderParser_parseStream(&parser,NyLPC_cHttpdConnection_refStream(i_connection),&(header.super))){
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
        NyLPC_OnErrorGoto(Error2);
    }
    if(!NyLPC_cHttpBasicHeaderParser_parseFinish(&parser,&(header.super))){
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
        NyLPC_OnErrorGoto(Error2);
    }
    //GETかHEADに制限
    method_type=NyLPC_cHttpdConnection_getMethod(i_connection);
    if(method_type!=NyLPC_THttpMethodType_GET && method_type!=NyLPC_THttpMethodType_HEAD)
    {
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,405);
        NyLPC_OnErrorGoto(Error2);
    }
    //Request::ConnectionがClose設定,又はHTTP1.1では無い場合,CLOSE
    if(header.super.connection==NyLPC_THttpMessgeHeader_Connection_CLOSE || header.super.startline.req.version!=NyLPC_THttpVersion_11)
    {
        NyLPC_cHttpdConnection_setConnectionMode(i_connection,NyLPC_TcHttpdConnection_CONNECTION_MODE_CLOSE);
    }
    //CGIの実行
    switch(header._content_id)
    {
    case CONTENT_ID_SETUP:
        setup_proc(i_connection,&header);
        break;
    case CONTENT_ID_UNKNOWN:
    default:
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
        NyLPC_OnErrorGoto(Error2);
    }
    NyLPC_cHttpBasicHeaderParser_finalize(&parser);
    NyLPC_cMiMicDbCompiler_finalize(&header._binparser);
    NyLPC_cStr_finalize(&(header._tstr));
    return NyLPC_TBool_TRUE;
Error2:
    NyLPC_cHttpBasicHeaderParser_finalize(&parser);
    NyLPC_cMiMicDbCompiler_finalize(&header._binparser);
    NyLPC_cStr_finalize(&(header._tstr));

    return NyLPC_TBool_FALSE;
}


static void setup_proc(NyLPC_TcHttpdConnection_t* i_connection,struct TModMiMicSettingRequest* i_req)
{
    NyLPC_TBool ret;
    const struct NyLPC_TMiMicConfigulation* config;
    const NyLPC_TcNetConfig_t* currebt_cfg;
    NyLPC_Assert(
        (NyLPC_cHttpdConnection_getMethod(i_connection)==NyLPC_THttpMethodType_GET)||
        (NyLPC_cHttpdConnection_getMethod(i_connection)==NyLPC_THttpMethodType_HEAD));

    switch(i_req->content.setup.tmp.cval){
    case QVAL_C_GET:
        if(!NyLPC_cHttpdUtils_sendJsonHeader(i_connection)){
            NyLPC_OnErrorGoto(Error);
        }
        if(NyLPC_cHttpdConnection_getMethod(i_connection)==NyLPC_THttpMethodType_GET){
            config=NyLPC_cMiMicConfiglation_loadFromFlash();
            //Flashの内容から
            if(!NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
                "{"
                "\"application\":\""MOD_VERSION";%s;%s(%s)\","
                "\"landev\":\"%s\",",
                NyLPC_cMiMicEnv_getStrProperty(NyLPC_cMiMicEnv_VERSION),
                NyLPC_cMiMicEnv_getStrProperty(NyLPC_cMiMicEnv_SHORT_NAME),
                NyLPC_cMiMicEnv_getStrProperty(NyLPC_cMiMicEnv_MCU_NAME),
                NyLPC_cMiMicEnv_getStrProperty(NyLPC_cMiMicEnv_ETHERNET_PHY)
                ))
            {
                NyLPC_OnErrorGoto(Error);
            }
            if(!NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
                "\"cfg\":{"
                "\"mac00010203\":%u,"
                "\"mac0405xxxx\":%u,"
                "\"host\":\"%s\","
                "\"ipv4\":{"
                    "\"flags\":%u,"
                    "\"ip\":%u,"
                    "\"mask\":%u,"
                    "\"droute\":%u,"
                "},"
                "\"services\":{"
                    "\"flags\":%u,"
                    "\"http_port\":%u"
                "}},",
                config->mac_00_01_02_03,
                config->mac_04_05_xx_xx,
                config->hostname,
                config->ipv4_flags,
                config->ipv4_addr_net,
                config->ipv4_mask_net,
                config->ipv4_drut_net,
                config->srv_flags,
                config->http_port
                )){
                NyLPC_OnErrorGoto(Error);
            }
            //write current status
            currebt_cfg=(const NyLPC_TcNetConfig_t*)NyLPC_cUipService_refCurrentConfig();
            if(!NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
                "\"cur\":{"
                "\"mac00010203\":%u,"
                "\"mac0405xxxx\":%u,"
                "\"host\":\"%s\","
                "\"ipv4\":{"
                    "\"flags\":%u,"
                    "\"ip\":%u,"
                    "\"mask\":%u,"
                    "\"droute\":%u,"
                "},"
                "\"services\":{"
                    "\"flags\":%u,"
                    "\"http_port\":%u"
                "}}}",
                (currebt_cfg->super.eth_mac.addr[0]<<24)|(currebt_cfg->super.eth_mac.addr[1]<<16)|(currebt_cfg->super.eth_mac.addr[2]<<8)|currebt_cfg->super.eth_mac.addr[3],
                (currebt_cfg->super.eth_mac.addr[4]<<24)|(currebt_cfg->super.eth_mac.addr[5]<<16),
                currebt_cfg->hostname,
                currebt_cfg->tcp_mode,
                NyLPC_ntohl(currebt_cfg->super.ip_addr.v),
                NyLPC_ntohl(currebt_cfg->super.netmask.v),
                NyLPC_ntohl(currebt_cfg->super.dr_addr.v),
                currebt_cfg->services.flags,
                currebt_cfg->services.http_port
                )){
                NyLPC_OnErrorGoto(Error);
            }
        }
        break;
    case QVAL_C_UPDATE:
        //check parameter length
        if(i_req->content.setup.tmp.param_len!=SIZE_OF_SETUP_PARAM || i_req->content.setup.tmp.host_len<1)
        {
            NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
        }else{
            //パラメータ→ROMイメージ変換
            i_req->content.setup.memimg.fast_boot=0xffffffff;
//          ここの部分は受信時にデータ位置を合わせてあるのでコピー不要。
//          cfg_image.mac_00_01_02_03=(i_req->content.setup.param_buf[0]);
//          cfg_image.mac_04_05_xx_xx=(i_req->content.setup.param_buf[1]&0xffff0000);
//          cfg_image.ipv4_flags     =i_req->content.setup.param_buf[2];
//          cfg_image.ipv4_addr_net  =i_req->content.setup.param_buf[3];
//          cfg_image.ipv4_mask_net  =i_req->content.setup.param_buf[4];
//          cfg_image.ipv4_drut_net  =i_req->content.setup.param_buf[5];
//          cfg_image.srv_flags =i_req->content.setup.param_buf[6];
//          strcpy(cfg_image.hostname,i_req->content.setup.host_name);
            i_req->content.setup.memimg.http_port =(NyLPC_TUInt16)(i_req->content.setup.tmp.param_buf[7]>>16);
            i_req->content.setup.memimg.padding=0xffff;
            //一応確認。
            if(i_req->content.setup.memimg.http_port==0){
                NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
            }else{
                //FreeRTOSの停止
                NyLPC_cIsr_enterCritical();
                //Flashへの書き込み
                ret=NyLPC_cMiMicConfiglation_updateConfigulation(&i_req->content.setup.memimg);
                //FreeRTOSの復帰
                NyLPC_cIsr_exitCritical();
                if(!ret){
                    NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
                }else{
                    if(!NyLPC_cHttpdUtils_sendJsonHeader(i_connection)){
                        NyLPC_OnErrorGoto(Error);
                    }
                    if(NyLPC_cHttpdConnection_getMethod(i_connection)==NyLPC_THttpMethodType_GET){
                        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
                            "{\"application\":\""MOD_VERSION"\",\"result\":%u}",
                            ret?0x00000000:0x80000000);
                    }
                }
            }
        }
        //JSONを書く。
        break;
    default:
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,400);
        NyLPC_OnErrorGoto(Error);
        break;
    }
    return;
Error:
    return;
}
