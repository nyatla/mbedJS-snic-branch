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
#include "NyLPC_cModUPnPDevice.h"
#include "../NyLPC_cHttpdConnection_protected.h"
#include "NyLPC_net.h"
#include "NyLPC_stdlib.h"
#include "NyLPC_http.h"
#include <ctype.h>



#define SIZE_OF_STRBUF 16
struct TUPnPDeviceHeader
{
    struct NyLPC_THttpBasicHeader super;
    //解析用
    NyLPC_TUInt8 _content_id;
    NyLPC_TUInt8 _astate;
    NyLPC_TInt16 _prefix_len;
    NyLPC_TcStr_t _tstr;
    NyLPC_TChar _tstr_buf[SIZE_OF_STRBUF];
    /** 文字列のパーサ*/
    union{
        NyLPC_TInt16 service_idx;
    }content;
};

#define ST_PARSE_PATH 1
#define ST_PARSE_QUERY_NAME 2

/**
 * コンテンツID定義(コンテンツ名に対応)
 */
#define CONTENT_ID_UNKNOWN      1
#define CONTENT_ID_DEVICE_XML   2
#define CONTENT_ID_CONTROL      3
#define CONTENT_ID_EVENT        4

#define QNAME_ID_UNKNOWN 1
#define QNAME_IDX 2

#define CONTENT_STR_DEVICE_XML "d.xml"
#define CONTENT_STR_CONTROL_PATH "control"
#define CONTENT_STR_EVENT_PATH  "event"
#define CONTENT_STR_XML_MIME_TYPE  "text/xml"





static void writeDeviceNode(const struct NyLPC_TUPnPDevDescDevice* i_dev,NyLPC_TcHttpdConnection_t* i_connection,NyLPC_TUInt16* sidx)
{
    //Required
    NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
        "<device>"
        "<deviceType>%s</deviceType>"
        "<friendlyName>%s</friendlyName>"
        "<manufacturer>%s</manufacturer>",
        i_dev->device_type,
        i_dev->frendly_name,
        i_dev->manufacturer);
    NyLPC_TInt16 i;
    //Optional
    if(i_dev->manufacturer_url!=NULL){
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<manufacturerURL>%s</manufacturerURL>",
            i_dev->manufacturer_url);
    }
    //Recommended
    if(i_dev->model_descriprion!=NULL){
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<modelDescription>%s</modelDescription>",
            i_dev->model_descriprion);
    }else{
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<modelDescription/>"); //Recommended
    }
    //Required
    NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
        "<modelName>%s</modelName>",
        i_dev->model_name);
    //Recommended
    if(i_dev->model_number!=NULL){
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<modelNumber>%s</modelNumber>",
            i_dev->model_number);
    }else{
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<modelNumber/>");
    }
    //Optional
    if(i_dev->model_url!=NULL){
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<modelURL>%s</modelURL>",
            i_dev->model_url);
    }
    //Recommended
    if(i_dev->serial_number!=NULL){
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<serialNumber>%s</serialNumber>",
            i_dev->serial_number);
    }else{
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<serialNumber/>");
    }
    //Required
    NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
        "<UDN>%s</UDN>",
        i_dev->udn);
    //Oprional
    if(i_dev->upc!=NULL){
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<UPC>%s</UPC>",
            i_dev->upc);
    }
    if(i_dev->number_of_icon>0){
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<iconList>");
        for(i=0;i<i_dev->number_of_icon;i++){
            NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
                "<icon>"
                "<mimetype>%s</mimetype>"
                "<width>%d</width>"
                "<height>%d</height>"
                "<depth>%d</depth>"
                "<url>%s</url>"
                "</icon>",
                i_dev->icons[i].mimetype,
                i_dev->icons[i].width,
                i_dev->icons[i].height,
                i_dev->icons[i].depth,
                i_dev->icons[i].url);
        }
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "</iconList>");
    }else{
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<iconList/>");
    }
    //Optional
    if(i_dev->number_of_service>0){
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<serviceList>");
        for(i=0;i<i_dev->number_of_service;i++){
            NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
                "<service>"
                "<serviceType>%s</serviceType>"
                "<serviceId>%s</serviceId>"
                "<SCPDURL>%s</SCPDURL>"
                "<controlURL>./control/%d</controlURL>"
                "<eventSubURL>./event/%d</eventSubURL>"
                "</service>",
                i_dev->services[i].scpd_url,
                i_dev->services[i].service_type,
                i_dev->services[i].service_id,
                (*sidx)+i,
                (*sidx)+i);
        }
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "</serviceList>");
    }
    if(i_dev->number_of_devices>0){
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<deviceList>");
        for(i=0;i<i_dev->number_of_devices;i++){
            (*sidx)=(*sidx)+0x10;
            writeDeviceNode(i_dev->devices[i],i_connection,sidx);
        }
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "</deviceList>");
    }
    if(i_dev->presentation_url!=NULL){
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "<presentationURL>%s</presentationURL></device>",
            i_dev->presentation_url);
    }
    else{
        NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
            "</device>");
    }
}
NyLPC_TBool writeDeviceDescription(const struct NyLPC_TUPnPDevDescDevice* i_dev,NyLPC_TcHttpdConnection_t* i_connection)
{
    NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,
        "<?xml version=\"1.0\"?>"
        "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
        "<specVersion><major>1</major><minor>0</minor></specVersion>");
    writeDeviceNode(i_dev,i_connection,0);
    return NyLPC_cHttpdConnection_sendResponseBodyF(i_connection,"</root>");
}





/**
 * control,eventのServiceIndex部分をパースする。
 * [:str:]/[:HEX8:][:HEX8:]
 */
static NyLPC_TInt16 parseSidx(const NyLPC_TChar* i_str)
{
    //先頭は/であること
    if(*i_str!='/'){
        return -1;
    }
    //2桁の16進数であること
    if(!isxdigit(*(i_str+1)) || !isxdigit(*(i_str+2))){
        return -1;
    }
    //サービスID化
    return NyLPC_ctox(*(i_str+1))<<8 | NyLPC_ctox(*(i_str+2));
}


static NyLPC_TBool urlHandler(NyLPC_TcHttpBasicHeaderParser_t* i_inst,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{

    struct TUPnPDeviceHeader* out=(struct TUPnPDeviceHeader*)o_out;
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
        if(strcmp(NyLPC_cStr_str(&(out->_tstr)),CONTENT_STR_DEVICE_XML)==0){
            out->_content_id=CONTENT_ID_DEVICE_XML;
        }else if(strncmp(CONTENT_STR_CONTROL_PATH,NyLPC_cStr_str(&(out->_tstr)),7)==0){
            out->_content_id=CONTENT_ID_CONTROL;
            parseSidx(NyLPC_cStr_str(&(out->_tstr))+7);
        }else if(strncmp(CONTENT_STR_EVENT_PATH,NyLPC_cStr_str(&(out->_tstr)),5)==0){
            out->_content_id=CONTENT_ID_EVENT;
            parseSidx(NyLPC_cStr_str(&(out->_tstr))+5);
        }else{
            NyLPC_OnErrorGoto(ERROR);
        }
        NyLPC_cStr_clear(&(out->_tstr));
        out->_astate=ST_PARSE_QUERY_NAME;//クエリ名解析へ
        return NyLPC_TBool_TRUE;
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
void NyLPC_cModUPnPDevice_initialize(NyLPC_TcModUPnPDevice_t* i_inst,const NyLPC_TcUPnP_t* i_ref_upnp)
{
    NyLPC_cModRomFiles_initialize(&i_inst->super,i_ref_upnp->_ref_root_path,NULL,0);
    i_inst->_ref_upnp=i_ref_upnp;
}
void NyLPC_cModUPnPDevice_finalize(NyLPC_TcModUPnPDevice_t* i_inst)
{
    NyLPC_cModRomFiles_finalize(&i_inst->super);
}
/**
 * モジュールがコネクションをハンドリングできるかを返します。
 */
NyLPC_TBool NyLPC_cModUPnPDevice_canHandle(NyLPC_TcModUPnPDevice_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection)
{
    return NyLPC_cModRomFiles_canHandle(&i_inst->super,i_connection);
}

/**
 * モジュールを実行します。
 */
NyLPC_TBool NyLPC_cModUPnPDevice_execute(NyLPC_TcModUPnPDevice_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection)
{
    NyLPC_TUInt8 method_type;
    struct TUPnPDeviceHeader header;
    NyLPC_TcHttpBasicHeaderParser_t parser;

    //リクエストParse済へ遷移(この関数の後はModが責任を持ってリクエストを返却)
    NyLPC_cHttpdConnection_setReqStatusParsed(i_connection);
    NyLPC_cStr_initialize(&header._tstr,header._tstr_buf,SIZE_OF_STRBUF);

    //URL解析の準備
    header._prefix_len=-((NyLPC_TInt16)strlen(i_inst->super._ref_root_path)+2);
    header._astate=ST_PARSE_PATH;

    NyLPC_cHttpBasicHeaderParser_initialize(&parser,&handler);
    NyLPC_cHttpBasicHeaderParser_parseInit(&parser,&(header.super));
    //プリフェッチしたデータを流す
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
    //GETかHEADに制限(Descriptionの場合だけ)
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
    case CONTENT_ID_DEVICE_XML:
        NyLPC_cHttpdConnection_sendResponseHeader(i_connection,200,CONTENT_STR_XML_MIME_TYPE,NULL);
        writeDeviceDescription(i_inst->_ref_upnp->ref_root_device,i_connection);
        //DeviceXML
        break;
    case CONTENT_ID_CONTROL:
        //SoapHandler 未実装
    case CONTENT_ID_EVENT:
        //EventHandler 未実装
    default:
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
        NyLPC_OnErrorGoto(Error2);
    }
    NyLPC_cHttpBasicHeaderParser_finalize(&parser);
    NyLPC_cStr_finalize(&header._tstr);
    return NyLPC_TBool_TRUE;
Error2:
    NyLPC_cHttpBasicHeaderParser_finalize(&parser);
    NyLPC_cStr_finalize(&header._tstr);
    return NyLPC_TBool_FALSE;
}

