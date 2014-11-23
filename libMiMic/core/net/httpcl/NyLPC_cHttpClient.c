/*
 * NyLPC_cHttpClient.c
 *
 *  Created on: 2013/08/24
 *      Author: nyatla
 */
#include "NyLPC_cHttpClient.h"
typedef NyLPC_TUInt8 NyLPC_TcHttpClient_ST;
#define NyLPC_TcHttpClient_ST_CLOSED 0x00	//ソケット切断
#define NyLPC_TcHttpClient_ST_IDLE   0x01	//メソッド選択待ち

#define NyLPC_TcHttpClient_ST_SEND_REQ_BODY	0x21 //POSTリクエスト送信中
#define NyLPC_TcHttpClient_ST_RECV_RES_HEAD	0x23
#define NyLPC_TcHttpClient_ST_RECV_RES_BODY	0x24



NyLPC_TBool NyLPC_cHttpClient_initialize(NyLPC_TcHttpClient_t* i_inst,void* i_rx_buf,NyLPC_TUInt16 i_rx_size)
{
	i_inst->_sock=NyLPC_cNetIf_createTcpSocketEx(NyLPC_TSocketType_TCP_NORMAL);
	if(i_inst->_sock==NULL){
		return NyLPC_TBool_FALSE;
	}
	i_inst->_state=NyLPC_TcHttpClient_ST_CLOSED;
	return NyLPC_TBool_TRUE;
}
void NyLPC_cHttpClient_finalize(NyLPC_TcHttpClient_t* i_inst)
{
	NyLPC_cHttpClient_close(i_inst);
	NyLPC_iTcpSocket_finalize(i_inst->_sock);
}

void NyLPC_cHttpClient_close(NyLPC_TcHttpClient_t* i_inst)
{
	//ステータスをclosedへ遷移
	switch(i_inst->_state){
	case NyLPC_TcHttpClient_ST_RECV_RES_BODY:
		NyLPC_cHttpBodyParser_finalize(&i_inst->pw.body_parser);
		break;
	case NyLPC_TcHttpClient_ST_SEND_REQ_BODY:
		NyLPC_cHttpBodyWriter_finalize(&(i_inst->pw.body_writer));
		break;
	case NyLPC_TcHttpClient_ST_RECV_RES_HEAD:
		//開放するものとくにない
		break;
	case NyLPC_TcHttpClient_ST_IDLE:
		break;
	case NyLPC_TcHttpClient_ST_CLOSED:
		return;
	}
	NyLPC_iTcpSocket_close(i_inst->_sock,1000);
	i_inst->_state=NyLPC_TcHttpClient_ST_CLOSED;
}

/**
 * サーバに接続する。
 * 関数はステータスをIDLEへ遷移する。
 * インスタンスのステータスは何でも構わない。
 * @return
 * TRUE - ステータスはIDLEへ遷移する。
 * FALSE - ステータスはCLOSEDへ遷移する。
 */
NyLPC_TBool NyLPC_cHttpClient_connect(NyLPC_TcHttpClient_t* i_inst,const struct NyLPC_TIPv4Addr* i_addr,NyLPC_TUInt16 i_port)
{
	//ステータスをclosedへ遷移
	NyLPC_cHttpClient_close(i_inst);
	//接続
	if(!NyLPC_iTcpSocket_connect(i_inst->_sock,i_addr,i_port,3000)){
		return NyLPC_TBool_FALSE;
	}
	//streamの生成
	if(!NyLPC_cHttpStream_initialize(&i_inst->_stream,i_inst->_sock)){
		NyLPC_OnErrorGoto(ERROR);
	}
	//ステータス遷移
	i_inst->_state=NyLPC_TcHttpClient_ST_IDLE;
	return NyLPC_TBool_TRUE;
ERROR:
	return NyLPC_TBool_FALSE;
}



/**
 * POSTリクエストを送信する。
 * @return
 * 引き続き処理が可能かを返す。
 */
NyLPC_TBool NyLPC_cHttpClient_sendMethod(
	NyLPC_TcHttpClient_t* i_inst,
	NyLPC_THttpMethodType i_method,
	const NyLPC_TChar* i_path,
	NyLPC_TUInt32 i_content_length,
	const NyLPC_TChar* i_mime_type,
	const NyLPC_TChar* i_additional_header)
{
	//ステータスチェック
	if(i_inst->_state!=NyLPC_TcHttpClient_ST_IDLE){
		NyLPC_OnErrorGoto(Error_0);
	}
	//POSTリクエストの送信
	if(!NyLPC_cHttpHeaderWriter_initialize(&i_inst->pw.head_writer,&i_inst->_stream.super,NULL)){
		NyLPC_OnErrorGoto(Error_0);
	}
	//ヘッダを送信
	NyLPC_cHttpHeaderWriter_setConnectionClose(&i_inst->pw.head_writer,NyLPC_TBool_TRUE);//Connection closeを強制
	if(i_content_length==NyLPC_cHttpHeaderWriter_CONTENT_LENGTH_UNLIMITED){
		NyLPC_cHttpHeaderWriter_setChunked(&i_inst->pw.head_writer);
	}else{
		NyLPC_cHttpHeaderWriter_setContentLength(&i_inst->pw.head_writer,i_content_length);
	}
	if(!NyLPC_cHttpHeaderWriter_writeRequestHeader(
		&i_inst->pw.head_writer,
		i_method,
		NyLPC_iTcpSocket_getPeerAddr((i_inst->_sock)),
		NyLPC_iTcpSocket_getPeerPort((i_inst->_sock)),i_path)){
		NyLPC_OnErrorGoto(Error_1);
	}
	//MimeType
	if(i_mime_type!=NULL){
		if(!NyLPC_cHttpHeaderWriter_writeMessage(&i_inst->pw.head_writer,"Content-type",i_mime_type)){
			NyLPC_OnErrorGoto(Error_1);
		}
	}
	if(i_additional_header!=NULL){
		if(!NyLPC_cHttpHeaderWriter_writeRawMessage(&i_inst->pw.head_writer,i_additional_header)){
			NyLPC_OnErrorGoto(Error_1);
		}
	}
	NyLPC_cHttpHeaderWriter_close(&i_inst->pw.head_writer);
	NyLPC_cHttpHeaderWriter_finalize(&i_inst->pw.head_writer);

	//BodyWriter生成
	NyLPC_cHttpBodyWriter_initialize(&(i_inst->pw.body_writer),&(i_inst->_stream));
	//bodyのchunkedもセット
	if(i_content_length==0xffffffff){
		NyLPC_cHttpBodyWriter_setChunked(&(i_inst->pw.body_writer));
	}else{
		NyLPC_cHttpBodyWriter_setContentLength(&(i_inst->pw.body_writer),i_content_length);
	}
	i_inst->_state=NyLPC_TcHttpClient_ST_SEND_REQ_BODY;
	return NyLPC_TBool_TRUE;
Error_0:
	return NyLPC_TBool_FALSE;
Error_1:
	NyLPC_cHttpHeaderWriter_finalize(&i_inst->pw.head_writer);
	return NyLPC_TBool_FALSE;
}

/**
 * POSTリクエストのデータを送信する。
 * @return
 * 0:EOF
 */
NyLPC_TBool NyLPC_cHttpClient_write(NyLPC_TcHttpClient_t* i_inst,const void* i_buf,NyLPC_TUInt32 i_buf_size)
{
	if(i_inst->_state!=NyLPC_TcHttpClient_ST_SEND_REQ_BODY){
		return NyLPC_TBool_FALSE;
	}
	if(!NyLPC_cHttpBodyWriter_write(&i_inst->pw.body_writer,i_buf,i_buf_size)){
		//ERROR
		NyLPC_cHttpClient_close(i_inst);
		NyLPC_OnErrorGoto(Error);
	}
	return NyLPC_TBool_TRUE;
Error:
	return NyLPC_TBool_FALSE;
}

NyLPC_TBool NyLPC_cHttpClient_writeFormat(NyLPC_TcHttpClient_t* i_inst,const NyLPC_TChar* i_fmt,...)
{
	NyLPC_TBool ret;
	va_list a;
	if(i_inst->_state!=NyLPC_TcHttpClient_ST_SEND_REQ_BODY){
		return NyLPC_TBool_FALSE;
	}
	va_start(a,i_fmt);
	ret=NyLPC_cHttpBodyWriter_formatV(&i_inst->pw.body_writer,i_fmt,a);
	va_end(a);
	if(!ret){
		NyLPC_cHttpClient_close(i_inst);
	}
	return ret;
}
NyLPC_TBool NyLPC_cHttpClient_writeFormatV(NyLPC_TcHttpClient_t* i_inst,const NyLPC_TChar* i_fmt,va_list i_args)
{
	NyLPC_TBool ret;
	if(i_inst->_state!=NyLPC_TcHttpClient_ST_SEND_REQ_BODY){
		return NyLPC_TBool_FALSE;
	}
	ret=NyLPC_cHttpBodyWriter_formatV(&i_inst->pw.body_writer,i_fmt,i_args);
	if(!ret){
		NyLPC_cHttpClient_close(i_inst);
	}
	return ret;
}


/**
 * ステータスコードを返す。
 * @return
 * ステータスコード
 */
NyLPC_TUInt16 NyLPC_cHttpClient_getStatus(NyLPC_TcHttpClient_t* i_inst)
{
	struct NyLPC_THttpBasicHeader header;
	if(i_inst->_state!=NyLPC_TcHttpClient_ST_SEND_REQ_BODY){
		return 0;
	}
	//
	if(!NyLPC_cHttpBodyWriter_close(&i_inst->pw.body_writer)){
		NyLPC_OnErrorGoto(Error_1);
	}
	i_inst->_state=NyLPC_TcHttpClient_ST_RECV_RES_HEAD;
	//100を無視してHTTPヘッダをパース
	//@todo POSTの時だけに制限したら？
	do{
		NyLPC_cHttpBasicHeaderParser_initialize(&i_inst->pw.head_parser,NULL);
		NyLPC_cHttpBasicHeaderParser_parseInit(&i_inst->pw.head_parser,&header);
		if(!NyLPC_cHttpBasicHeaderParser_parseStream(&i_inst->pw.head_parser,&i_inst->_stream.super,&header)){
			NyLPC_OnErrorGoto(Error_2);
		}
		if(!NyLPC_cHttpBasicHeaderParser_parseFinish(&i_inst->pw.head_parser,&header)){
			NyLPC_OnErrorGoto(Error_2);
		}
		NyLPC_cHttpBasicHeaderParser_finalize(&i_inst->pw.head_parser);
		//レスポンスヘッダか確認
		if(header.type!=NyLPC_THttpHeaderType_RESPONSE){
			NyLPC_OnErrorGoto(Error_1);
		}
	}while(header.startline.res.status==100);
	//BodyParserを起動
	NyLPC_cHttpBodyParser_initialize(&i_inst->pw.body_parser);
	NyLPC_cHttpBodyParser_parseInit(&i_inst->pw.body_parser,&header);
	i_inst->_state=NyLPC_TcHttpClient_ST_RECV_RES_BODY;
	return header.startline.res.status;
Error_1:
	NyLPC_cHttpClient_close(i_inst);
	return 0;
Error_2:
	NyLPC_cHttpBasicHeaderParser_finalize(&i_inst->pw.head_parser);
	NyLPC_cHttpClient_close(i_inst);
	return 0;
}


/**
 * GET/POSTリクエストで受信したデータを読み出す。
 * @param o_read_len
 * 戻り値TRUEの場合のみ有効。
 * 終端の場合は0
 * @return
 * TRUE:正常読み出し。o_read_lenの値で終端判定
 * FALSE:失敗。コネクションはクローズされる。
 */
NyLPC_TBool NyLPC_cHttpClient_read(NyLPC_TcHttpClient_t* i_inst,void* i_buf,NyLPC_TUInt32 i_buf_size,NyLPC_TInt16* o_read_len)
{
	if(i_inst->_state!=NyLPC_TcHttpClient_ST_RECV_RES_BODY){
		return NyLPC_TBool_FALSE;
	}
	if(!NyLPC_cHttpBodyParser_parseStream(&i_inst->pw.body_parser,&i_inst->_stream.super,i_buf,i_buf_size,o_read_len)){
		NyLPC_cHttpClient_close(i_inst);
		return NyLPC_TBool_FALSE;
	}
	return NyLPC_TBool_TRUE;
}

