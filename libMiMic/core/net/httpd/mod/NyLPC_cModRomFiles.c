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
#include "NyLPC_cModRomFiles_protected.h"
#include "NyLPC_stdlib.h"
#include "NyLPC_http.h"
#include "../NyLPC_cHttpdUtils.h"
#include "../NyLPC_cHttpdConnection_protected.h"


/**
 * コンストラクタ。
 * @param i_ref_path
 * 16文字以内のルートパスを指定します。文字列は外部参照です。インスタンスをfinalizeするまで維持してください。
 * 文字列は/で開始され/で終了する必要があります。
 * path:= /({.,14}/)?
 * @param i_data
 * ROMFile構造体の配列の先頭アドレス
 */
void NyLPC_cModRomFiles_initialize(NyLPC_TcModRomFiles_t* i_inst,const NyLPC_TChar* i_ref_root_path,const struct NyLPC_TRomFileData* i_data,int i_num_of_data)
{
    NyLPC_cModUrl_initialize(&(i_inst->super));
    i_inst->_data=i_data;
    i_inst->_num_of_data=i_num_of_data;
    i_inst->_ref_root_path=i_ref_root_path;
}
void NyLPC_cModRomFiles_finalize(NyLPC_TcModRomFiles_t* i_inst)
{
    NyLPC_cModUrl_finalize(&(i_inst->super));
}

/**
 * モジュールがコネクションをハンドリングできるかを返します。
 */
NyLPC_TBool NyLPC_cModRomFiles_canHandle(NyLPC_TcModRomFiles_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection)
{
    const NyLPC_TChar* in_url;
    const NyLPC_TChar* base_url=i_inst->_ref_root_path;
    //connectonの状態を確認
    if(!NyLPC_cHttpdConnection_getReqStatus(i_connection)==NyLPC_cHttpdConnection_ReqStatus_REQPARSE)
    {
        return NyLPC_TBool_FALSE;
    }
    in_url=NyLPC_cHttpdConnection_getUrlPrefix(i_connection);
    size_t base_url_len=strlen(base_url);
    //check '/'+base_url+'/'
    if(strlen(in_url)-2<base_url_len){
        return NyLPC_TBool_FALSE;
    }
    if(in_url[0]!='/' || strncmp(in_url+1,base_url,base_url_len)!=0 || in_url[base_url_len+1]!='/'){
        return NyLPC_TBool_FALSE;
    }
    return NyLPC_TBool_TRUE;
}



/**
 * モジュールを実行します。
 */
NyLPC_TBool NyLPC_cModRomFiles_execute(NyLPC_TcModRomFiles_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection)
{
    int i;
    char url[32];
    NyLPC_TUInt8 method_type;
    //URLサフィックスを取得(rootpath+'2'*2)
    if(!NyLPC_cModUrl_execute2(&(i_inst->super),i_connection,url,32,strlen(i_inst->_ref_root_path)+2,NyLPC_cModUrl_ParseMode_PATH_ONLY))
    {
        //Response処理はモジュール内で実行済
        NyLPC_OnErrorGoto(Error1);
    }
    //GETかHEADに制限
    method_type=NyLPC_cHttpdConnection_getMethod(i_connection);
    if(method_type!=NyLPC_THttpMethodType_GET && method_type!=NyLPC_THttpMethodType_HEAD)
    {
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,405);
        NyLPC_OnErrorGoto(Error1);
    }
    if(strlen(url)==0)
    {
        //PrefixがURLよりも短ければ403エラーで処理終了。
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,403);
        NyLPC_OnErrorGoto(Error1);
    }
    //URL比較
    for(i=i_inst->_num_of_data-1;i>=0;i--)
    {
        if(strcmp(url,i_inst->_data[i].name)!=0){
            continue;
        }
        //Request::ConnectionがClose設定,又はHTTP1.1では無い場合,CLOSE
        if(i_inst->super._header.connection==NyLPC_THttpMessgeHeader_Connection_CLOSE || i_inst->super._header.startline.req.version!=NyLPC_THttpVersion_11)
        {
            NyLPC_cHttpdConnection_setConnectionMode(i_connection,NyLPC_TcHttpdConnection_CONNECTION_MODE_CLOSE);
        }
        return NyLPC_cHttpdUtils_sendFixedContentBatch(i_connection,i_inst->_data[i].content_type,i_inst->_data[i].data,(i_inst->_data[i].size>0)?i_inst->_data[i].size:strlen(i_inst->_data[i].data));
    }
    //404Error
    NyLPC_cHttpdUtils_sendErrorResponse(i_connection,404);
Error1:
    return NyLPC_TBool_FALSE;
}
