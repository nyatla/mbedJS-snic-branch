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
 *	http://nyatla.jp/
 *	<airmail(at)ebony.plala.or.jp> or <nyatla(at)nyatla.jp>
 *
 *********************************************************************************/
#include "NyLPC_cModWebSocket_protected.h"
#include "NyLPC_utils.h"

#define HTTP_TIMEOUT NyLPC_TiHttpPtrStream_DEFAULT_HTTP_TIMEOUT

#define NyLPC_TcModWebSocket_FRAME_TYPE_BIN 0x01
#define NyLPC_TcModWebSocket_FRAME_TYPE_TXT 0x02



#define STRBUF_MAX 32
struct TModWebSocketHeader
{
	struct NyLPC_THttpBasicHeader super;
	NyLPC_TcStr_t _tstr;
	NyLPC_TChar _tstr_buf[STRBUF_MAX];
	NyLPC_TChar key[24+4];
	NyLPC_TInt16 version;
	NyLPC_TUInt8 sub_protocol_id;
	NyLPC_TUInt8 message_id;
	const NyLPC_TChar* _ref_sub_protocol;
};



#define MESSAGE_ID_UNKNOWN					0x00
#define MESSAGE_ID_UPGRADE					0x01
#define MESSAGE_ID_SEC_WEBSOCKET_KEY		0x02
#define MESSAGE_ID_ORIGIN					0x03
#define MESSAGE_ID_SEC_WEBSOCKET_PROTOCL	0x04
#define MESSAGE_ID_SEC_WEBSOCKET_VERSION	0x05

static const struct NyLPC_TTextIdTbl msg_tbl[]=
{
	{"Upgrade",MESSAGE_ID_UPGRADE},
	{"Sec-WebSocket-Key",MESSAGE_ID_SEC_WEBSOCKET_KEY},
	{"Origin",MESSAGE_ID_ORIGIN},
	{"Sec-WebSocket-Protocol",MESSAGE_ID_SEC_WEBSOCKET_PROTOCL},
	{"Sec-WebSocket-Version",MESSAGE_ID_SEC_WEBSOCKET_VERSION},
	{NULL,MESSAGE_ID_UNKNOWN}
};

static NyLPC_TBool messageHandler(NyLPC_TcHttpBasicHeaderParser_t* i_inst,const NyLPC_TChar* i_name,NyLPC_TChar i_c,struct NyLPC_THttpBasicHeader* o_out)
{
	struct TModWebSocketHeader* out=(struct TModWebSocketHeader*)o_out;
	if(i_name!=NULL){
		out->message_id=NyLPC_TTextIdTbl_getMatchIdIgnoreCase(i_name,msg_tbl);
		NyLPC_cStr_clear(&(out->_tstr));
	}else{
		switch(out->message_id)
		{
		case MESSAGE_ID_UPGRADE:
			if(i_c!='\0'){
				if(!NyLPC_cStr_put(&(out->_tstr),i_c)){
					NyLPC_OnErrorGoto(ERROR);
				}
			}else{
				//websocketかチェック
				if(!NyLPC_cStr_isEqualIgnoreCase(&out->_tstr,"websocket")){
					return NyLPC_TBool_FALSE;//不一致
				}
			}
			break;
		case MESSAGE_ID_SEC_WEBSOCKET_KEY:
			if(i_c!='\0'){
				if(!NyLPC_cStr_put(&(out->_tstr),i_c)){
					NyLPC_OnErrorGoto(ERROR);
				}
			}else{
				//HASH値をコピー
				strcpy(out->key,NyLPC_cStr_str(&out->_tstr));
			}
			break;
		case MESSAGE_ID_SEC_WEBSOCKET_PROTOCL:
			if(i_c!='\0' && i_c!=','){
				if(!NyLPC_cStr_put(&(out->_tstr),i_c)){
					NyLPC_OnErrorGoto(ERROR);
				}
			}else{
				//トークン終端
				if(out->_ref_sub_protocol!=NULL){
					//サブプロトコルが指定されている場合はチェック
					if(NyLPC_stricmp(NyLPC_cStr_str(&out->_tstr),out->_ref_sub_protocol)==0){
						out->sub_protocol_id=1;//SubProtocol一致
					}
				}
				//','の時はリセット
				if(i_c!=','){
					NyLPC_cStr_clear(&(out->_tstr));
				}
			}
			break;
		case MESSAGE_ID_SEC_WEBSOCKET_VERSION:
			if(i_c!='\0'){
				if(!NyLPC_cStr_put(&(out->_tstr),i_c)){
					NyLPC_OnErrorGoto(ERROR);
				}
			}else{
				//VERSION
				out->version=atoi(NyLPC_cStr_str(&out->_tstr));
				if(out->version<0){
					NyLPC_OnErrorGoto(ERROR);
				}
			}
		}
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
	messageHandler,
	NULL
};



/**
 * コンストラクタ。
 */
void NyLPC_cModWebSocket_initialize(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_ref_root_path)
{
	NyLPC_cModRomFiles_initialize(&i_inst->super,i_ref_root_path,NULL,0);
	i_inst->_frame_type=NyLPC_TcModWebSocket_FRAME_TYPE_TXT;
	i_inst->_payload_st=NyLPC_TcModWebSocket_ST_CLOSED;
}
void NyLPC_cModWebSocket_finalize(NyLPC_TcModWebSocket_t* i_inst)
{
	NyLPC_cModRomFiles_finalize(&i_inst->super);
}
/**
 * モジュールがコネクションをハンドリングできるかを返します。
 */
NyLPC_TBool NyLPC_cModWebSocket_canHandle(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection)
{
	return NyLPC_cModRomFiles_canHandle(&i_inst->super,i_connection);
}

static union{
	struct TModWebSocketHeader header;
}work;



/**
 * モジュールを実行します。
 */
NyLPC_TBool NyLPC_cModWebSocket_execute(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TcHttpdConnection_t* i_connection)
{
	union{
		NyLPC_TcHttpBasicHeaderParser_t parser;
		SHA1_CTX sh1;
	}sh;

	//リクエストParse済へ遷移(この関数の後はModが責任を持ってリクエストを返却)
	NyLPC_cHttpdConnection_setReqStatusParsed(i_connection);



	//排他ロック
	NyLPC_cHttpdConnection_lock(i_connection);
	{//parser

		//初期化
		work.header.version=0;
		work.header.sub_protocol_id=0;
		NyLPC_cStr_initialize(&work.header._tstr,work.header._tstr_buf,STRBUF_MAX);

		NyLPC_cHttpBasicHeaderParser_initialize(&sh.parser,&handler);

		//プリフェッチしたデータを流す
		NyLPC_cHttpBasicHeaderParser_parseInit(&sh.parser,&(work.header.super));
		NyLPC_cHttpdConnection_pushPrefetchInfo(i_connection,&sh.parser,&work.header.super);
		//後続をストリームから取り込む
		if(!NyLPC_cHttpBasicHeaderParser_parseStream(&sh.parser,NyLPC_cHttpdConnection_refStream(i_connection),&(work.header.super))){
			NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
			NyLPC_OnErrorGoto(Error1);
		}
		if(!NyLPC_cHttpBasicHeaderParser_parseFinish(&sh.parser,&(work.header.super))){
			NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
			NyLPC_OnErrorGoto(Error1);
		}
		//HeaderParserはここで破棄(URLEncode,cSTRも)
		NyLPC_cHttpBasicHeaderParser_finalize(&sh.parser);

		NyLPC_cStr_finalize(&single_header._tstr);


		//HTTP/1.1であること。Connection:Upgradeはチェックしない。
		if(work.header.super.startline.req.version!=NyLPC_THttpVersion_11)
		{
			NyLPC_cHttpdUtils_sendErrorResponse(i_connection,400);
			NyLPC_OnErrorGoto(Error2);
		}
		if(NyLPC_cHttpdConnection_getMethod(i_connection)!=NyLPC_THttpMethodType_GET){
			NyLPC_cHttpdUtils_sendErrorResponse(i_connection,400);
			NyLPC_OnErrorGoto(Error2);
		}
		//WebSocket version 13であること
		if(work.header.version!=13){
			NyLPC_cHttpdUtils_sendErrorResponse(i_connection,400);
			NyLPC_OnErrorGoto(Error2);
		}

		//レスポンスの生成(生データを直接ストリームへ書きこむ)
		if(!NyLPC_iHttpPtrStream_write(NyLPC_cHttpdConnection_refStream(i_connection),
			"HTTP/1.1 101 Switching Protocols\r\n"	//32+2
			"Upgrade: websocket\r\n" 				//18+2
			"Connection: Upgrade\r\n"				//19+2
			"Sec-WebSocket-Accept: "				//22
			,32+2+18+2+19+2+22)){
			NyLPC_OnErrorGoto(Error3);
		}
		//SH1キーの生成
		SHA1Init(&sh.sh1);
		SHA1Update(&sh.sh1,(const unsigned char*)work.header.key,strlen(work.header.key));
		SHA1Update(&sh.sh1,(const unsigned char*)"258EAFA5-E914-47DA-95CA-C5AB0DC85B11",36);
		//ワークメモリ32バイトはstrの使いまわし
		SHA1Final((unsigned char*)(work.header._tstr_buf),&sh.sh1);
		//BASE64化(single_header.keyへ出力)
		NyLPC_cBase64_encode(work.header._tstr_buf,20,work.header.key);
		if(!NyLPC_iHttpPtrStream_write(NyLPC_cHttpdConnection_refStream(i_connection),work.header.key,28)){
			NyLPC_OnErrorGoto(Error3);
		}
		//SubProtocolの認証が有る場合
		if(work.header.sub_protocol_id!=0){
			if(!NyLPC_iHttpPtrStream_write(NyLPC_cHttpdConnection_refStream(i_connection)
				,"\r\nSec-WebSocket-Protocol: "	//24
				,24)){
				NyLPC_OnErrorGoto(Error3);
			}
			if(!NyLPC_iHttpPtrStream_write(NyLPC_cHttpdConnection_refStream(i_connection)
				,work.header._ref_sub_protocol
				,strlen(work.header._ref_sub_protocol)))
			{
				NyLPC_OnErrorGoto(Error3);
			}
		}
		//Sec-WebSocket-Protocol
		if(!NyLPC_iHttpPtrStream_write(NyLPC_cHttpdConnection_refStream(i_connection),"\r\n\r\n",4)){
			NyLPC_OnErrorGoto(Error3);
		}
		//connection phase
		i_inst->_payload_st=NyLPC_TcModWebSocket_ST_START_PAYLOAD;
	}
//占有解除
	NyLPC_cHttpdConnection_unlock(i_connection);
	//参照コネクションの設定
	i_inst->_ref_connection=i_connection;
	NyLPC_iHttpPtrStream_flush(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection));
	return NyLPC_TBool_TRUE;
Error3:
Error2:
	//VM排他ロックの解除
	NyLPC_cHttpdConnection_unlock(i_connection);
	return NyLPC_TBool_FALSE;
Error1:
	NyLPC_cHttpBasicHeaderParser_finalize(&parser);
	NyLPC_cStr_finalize(&single_header._tstr);
	//VM排他ロックの解除
	NyLPC_cHttpdConnection_unlock(i_connection);
	return NyLPC_TBool_FALSE;
}



static void writeClosePacket(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TUInt16 i_code)
{
	char w[4];
	w[0]=0x88;
	w[1]=0x02;
	*((NyLPC_TUInt16*)(&w[2]))=NyLPC_htons(i_code);	//REASON
	//CloseFrame送信
	NyLPC_iHttpPtrStream_write(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),w,4);
	NyLPC_iHttpPtrStream_flush(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection));
}


NyLPC_TBool NyLPC_cModWebSocket_canRead(const NyLPC_TcModWebSocket_t* i_inst)
{
	const NyLPC_TUInt8* rx;
	return NyLPC_iHttpPtrStream_pread(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),(const void**)&rx,0)>0;
}

#define FLAGS_MASK_BIT 7

/**
 * Websocketの状態を更新します。
 * @return
 */
void NyLPC_cModWebSocket_update(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TUInt32 i_time_out)
{
	const NyLPC_TUInt8* rx;
	NyLPC_TInt32 rs,rt;
	NyLPC_TUInt16 header_size;
	NyLPC_TUInt8 w8[2];
	if(i_inst->_payload_st==NyLPC_TcModWebSocket_ST_CLOSED){
		return;
	}
START:
	rs=NyLPC_iHttpPtrStream_pread(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),(const void**)&rx,i_time_out);

	//Error?
	if(rs<0){
		NyLPC_OnErrorGoto(Error);
	}
	//Timeout?
	if(rs==0){
		goto Timeout;
	}
	switch(i_inst->_payload_st){
	case NyLPC_TcModWebSocket_ST_READ_PAYLOAD:
		//ペイロード読み出し中破何もしない
		return;
	case NyLPC_TcModWebSocket_ST_START_PAYLOAD:
		//ペイロード
		//2バイト溜まるまで待つ
		if(rs<2){
			//Timeout?
			goto Timeout;
		}
		//ペイロードサイズの分析
		if((0x7f&rx[1])<=125){
			header_size=2+(((rx[1]&0x80)==0x80)?4:0);
			i_inst->payload_size=(0x7f&rx[1]);
		}else if((0x7f&rx[1])==126){
			if(rs<4){
				//Timeout?
				goto Timeout;
			}
			header_size=2+2+(((rx[1]&0x80)==0x80)?4:0);
			i_inst->payload_size=(rx[2]<<8)|rx[3];
		}else{
			//CLOSEの送信
			writeClosePacket(i_inst,1009);
			NyLPC_OnErrorGoto(Error);
		}
		//十分なヘッダが集まったかチェック
		if(rs<header_size){
			goto Timeout;
		}
		i_inst->_frame_flags_bits=0;
		//FINがセットされていること.断片化禁止!
		if((rx[0]&0x80)!=0x80){
			NyLPC_OnErrorGoto(Error);
		}
		//必要ならMaskをコピー
		if((rx[1]&0x80)==0x80){
			memcpy(i_inst->_frame_mask,(rx+header_size-4),4);
			NyLPC_TUInt8_setBit(i_inst->_frame_flags_bits,FLAGS_MASK_BIT);
		}
		//ペイロードポインターのリセット
		i_inst->payload_ptr=0;

		//パケットサイズの確定(基本ヘッダ+マスク)
		switch(rx[0]&0x0f){
		case 0x00:
			//継続パケットは扱わない
			NyLPC_OnErrorGoto(Error);
		case 0x01:
			if(i_inst->_frame_type!=NyLPC_TcModWebSocket_FRAME_TYPE_TXT){
				NyLPC_OnErrorGoto(Error);
			}
			break;
		case 0x02:
			if(i_inst->_frame_type==NyLPC_TcModWebSocket_FRAME_TYPE_BIN){
				NyLPC_OnErrorGoto(Error);
			}
			break;
		case 0x08://close(非断片)
			//CloseFrame送信
			writeClosePacket(i_inst,1009);
			//Errorとして処理
			NyLPC_OnErrorGoto(Error);
		case 0x09://ping(非断片)
			//PONGを送信
			w8[0]=0x0a;
			NyLPC_iHttpPtrStream_write(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),w8,1);
			NyLPC_iHttpPtrStream_write(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),rx+1,header_size-1);
			NyLPC_iHttpPtrStream_flush(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection));
			NyLPC_iHttpPtrStream_rseek(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),header_size);
			while(i_inst->payload_size!=i_inst->payload_ptr){
				rs=NyLPC_iHttpPtrStream_pread(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),(const void**)&rx,HTTP_TIMEOUT);
				if(rs<=0){
					if(rs<0){
						//Error
						NyLPC_OnErrorGoto(Error);
					}
					//Timeout
					goto Timeout;
				}
				//読出し可能なサイズを決定
				rt=i_inst->payload_size-i_inst->payload_ptr;
				if(rs>rt){
					rs=rt;
				}
				//パケットを破棄
				NyLPC_iHttpPtrStream_write(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),rx,rs);
				NyLPC_iHttpPtrStream_rseek(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),rs);
				i_inst->payload_ptr+=rs;
			}
			//Timeout(パケットスタートに戻る？)
			goto START;
		case 0x0a://pong(非断片)
			//パケットの読み捨て
			NyLPC_iHttpPtrStream_rseek(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),header_size);
			while(i_inst->payload_size!=i_inst->payload_ptr){
				rs=NyLPC_iHttpPtrStream_pread(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),(const void**)&rx,HTTP_TIMEOUT);
				if(rs<=0){
					if(rs<0){
						//Error
						NyLPC_OnErrorGoto(Error);
					}
					//Timeout
					goto Timeout;
				}
				//読出し可能なサイズを決定
				rt=i_inst->payload_size-i_inst->payload_ptr;
				if(rs>rt){
					rs=rt;
				}
				//パケットを破棄
				NyLPC_iHttpPtrStream_rseek(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),rs);
				i_inst->payload_ptr+=rs;
			}
			//Timeout(パケットスタートに戻る？)
			goto START;
		default:
			//知らないコードはエラー
			NyLPC_OnErrorGoto(Error);
		}
		//読み出し位置のシーク(Header部)
		NyLPC_iHttpPtrStream_rseek(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),header_size);
		//ペイロード読み出しへ
		i_inst->_payload_st=NyLPC_TcModWebSocket_ST_READ_PAYLOAD;
		//継続してペイロード受信処理
		return;
	}
	//処理されなければエラー
Error:
	NyLPC_cHttpdConnection_closeSocket(i_inst->_ref_connection);
	i_inst->_payload_st=NyLPC_TcModWebSocket_ST_CLOSED;
	return;
Timeout:
	return;
}


/**
 * 受信データをコールバック関数に通知するNyLPC_cModWebSocket_readです。
 * @return
 * n>0:データ受信
 * 0  :タイムアウト。コネクションの状態は変化しない。
 * -1 :エラー コネクションはNyLPC_TcModWebSocket_ST_CLOSEDへ遷移する。
 */
NyLPC_TInt16 NyLPC_cModWebSocket_readCB(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TcModWebSocket_onRreadCB i_cb,void* i_cb_param)
{
	const NyLPC_TUInt8* rx;
	NyLPC_TInt32 rs,rd,i;
	//ストリームの状態を更新する。
	NyLPC_cModWebSocket_update(i_inst,HTTP_TIMEOUT);

	switch(i_inst->_payload_st)
	{
	case NyLPC_TcModWebSocket_ST_READ_PAYLOAD:
		break;//処理継続
	case NyLPC_TcModWebSocket_ST_START_PAYLOAD:
		//タイムアウト扱い
		return 0;
	default:
		return -1;
	}
	//読み出し可能なデータをパース
	rs=NyLPC_iHttpPtrStream_pread(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),(const void**)&rx,HTTP_TIMEOUT);
	if(rs<=0){
		if(rs<0){
			//Error
			NyLPC_OnErrorGoto(Error);
		}
		//Timeout
		goto Timeout;
	}
	//読出し可能な残りサイズを計算して上書き
	rd=i_inst->payload_size-i_inst->payload_ptr;
	if(rs>rd){
		rs=rd;
	}
	//読みだしたバイト数をリセット
	rd=0;
	//アンマスク
	if(NyLPC_TUInt8_isBitOn(i_inst->_frame_flags_bits,FLAGS_MASK_BIT)){
		//マスク有の時
		for(i=0;i<rs;i++){
			rd++;
			switch(i_cb(i_cb_param,rx[i]^i_inst->_frame_mask[(i_inst->payload_ptr+i)%4])){
			case 1:
				continue;
			case 0:
				break;
			default:
				NyLPC_OnErrorGoto(Error);
			}
		}
	}else{
		//マスクなしの時
		for(i=0;i<rs;i++){
			rd++;
			switch(i_cb(i_cb_param,rx[i])){
			case 1:
				continue;
			case 0:
				break;
			default:
				NyLPC_OnErrorGoto(Error);
			}
		}
	}
	//読取位置を移動
	NyLPC_iHttpPtrStream_rseek(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),rd);
	i_inst->payload_ptr+=rd;
	if(i_inst->payload_size==i_inst->payload_ptr){
		i_inst->_payload_st=NyLPC_TcModWebSocket_ST_START_PAYLOAD;
	}
	return rd;
	//処理されなければエラー
Error:
	NyLPC_cHttpdConnection_closeSocket(i_inst->_ref_connection);
	i_inst->_payload_st=NyLPC_TcModWebSocket_ST_CLOSED;
	return -1;
Timeout:
	return 0;
}

/**
 * @return
 * n>0:データ受信
 * 0  :タイムアウト。コネクションの状態は変化しない。
 * -1 :エラー コネクションはNyLPC_TcModWebSocket_ST_CLOSEDへ遷移する。
 */
NyLPC_TInt16 NyLPC_cModWebSocket_read(NyLPC_TcModWebSocket_t* i_inst,void* i_buf,NyLPC_TInt16 i_buf_len)
{
	const NyLPC_TUInt8* rx;
	NyLPC_TInt32 rs,i;
	//ストリームの状態を更新する。
	NyLPC_cModWebSocket_update(i_inst,HTTP_TIMEOUT);

	switch(i_inst->_payload_st)
	{
	case NyLPC_TcModWebSocket_ST_READ_PAYLOAD:
		break;//処理継続
	case NyLPC_TcModWebSocket_ST_START_PAYLOAD:
		//タイムアウト扱い
		return 0;
	default:
		return -1;
	}
	//読み出し可能なデータをパース
	rs=NyLPC_iHttpPtrStream_pread(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),(const void**)&rx,HTTP_TIMEOUT);
	if(rs<=0){
		if(rs<0){
			//Error
			NyLPC_OnErrorGoto(Error);
		}
		//Timeout
		goto Timeout;
	}
	//読み込みサイズを決定
	rs=(rs<i_buf_len)?rs:i_buf_len;
	//アンマスク
	if(NyLPC_TUInt8_isBitOn(i_inst->_frame_flags_bits,FLAGS_MASK_BIT)){
		for(i=0;i<rs;i++){
			*(((NyLPC_TUInt8*)i_buf)+i)=rx[i]^i_inst->_frame_mask[(i_inst->payload_ptr+i)%4];
		}
	}else{
		memcpy(i_buf,rx,rs);
	}
	//読取位置を移動
	NyLPC_iHttpPtrStream_rseek(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),rs);
	i_inst->payload_ptr+=rs;
	if(i_inst->payload_size==i_inst->payload_ptr){
		i_inst->_payload_st=NyLPC_TcModWebSocket_ST_START_PAYLOAD;
	}
	return rs;
	//処理されなければエラー
Error:
	NyLPC_cHttpdConnection_closeSocket(i_inst->_ref_connection);
	i_inst->_payload_st=NyLPC_TcModWebSocket_ST_CLOSED;
	return -1;
Timeout:
	return 0;
}




static NyLPC_TBool fmt_handler(void* i_inst,const void* i_buf,NyLPC_TUInt32 i_len)
{
	return NyLPC_iHttpPtrStream_write((NyLPC_TiHttpPtrStream_t*)i_inst,i_buf,i_len);
}


/**
 * Payloadヘッダを書く。
 */
NyLPC_TBool NyLPC_cModWebSocket_writePayloadHeader(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TInt16 i_len)
{
	NyLPC_TUInt16 s;
	NyLPC_TChar w[4];
	//CLOSED,CONNECTの時は使用不可
	switch(i_inst->_payload_st){
	case NyLPC_TcModWebSocket_ST_CLOSED:
		return NyLPC_TBool_FALSE;
	default:
		break;
	}
	//データサイズで切り分け
	switch(i_inst->_frame_type)
	{
	case NyLPC_TcModWebSocket_FRAME_TYPE_TXT:
		w[0]=0x80|0x01;
		break;
	case NyLPC_TcModWebSocket_FRAME_TYPE_BIN:
		w[0]=0x80|0x02;
		break;
	default:
		NyLPC_OnErrorGoto(Error);
	}
	if(i_len<126){
		w[1]=(NyLPC_TUInt8)i_len;
		s=2;
	}else{
		w[1]=126;
		s=3;
		*((NyLPC_TUInt16*)(&(w[2])))=NyLPC_htons(i_len);
	}
	if(!NyLPC_iHttpPtrStream_write(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),w,s)){
		//CLOSE
		NyLPC_OnErrorGoto(Error);
	}
	return NyLPC_TBool_TRUE;
Error:
	return NyLPC_TBool_FALSE;
}



NyLPC_TBool NyLPC_cModWebSocket_writeFormatV(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_fmt,va_list args)
{
	NyLPC_TInt16 l;
	va_list a;
	//ストリームの状態を更新する。
	NyLPC_cModWebSocket_update(i_inst,0);

	//書式文字列の長さを計算
	NyLPC_va_copy(a,args);
	l=NyLPC_cFormatWriter_length(i_fmt,a);
	va_end(a);
	if(!NyLPC_cModWebSocket_writePayloadHeader(i_inst,l)){
		//CLOSE
		NyLPC_OnErrorGoto(Error);
	}
	if(!NyLPC_cFormatWriter_print(fmt_handler,NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),i_fmt,args)){
		NyLPC_OnErrorGoto(Error);
	}
	NyLPC_iHttpPtrStream_flush(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection));
	return NyLPC_TBool_TRUE;
Error:
	NyLPC_cHttpdConnection_closeSocket(i_inst->_ref_connection);
	i_inst->_payload_st=NyLPC_TcModWebSocket_ST_CLOSED;
	return NyLPC_TBool_FALSE;
}

NyLPC_TBool NyLPC_cModWebSocket_writeFormat(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_fmt,...)
{
	NyLPC_TBool r;
	va_list a;
	va_start(a,i_fmt);
	r=NyLPC_cModWebSocket_writeFormatV(i_inst,i_fmt,a);
	va_end(a);
	return r;
}




NyLPC_TBool NyLPC_cModWebSocket_write(NyLPC_TcModWebSocket_t* i_inst,const void* i_buf,NyLPC_TInt16 i_len)
{
	//ストリームの状態を更新する。
	NyLPC_cModWebSocket_update(i_inst,0);
	if(!NyLPC_cModWebSocket_writePayloadHeader(i_inst,i_len)){
		//CLOSE
		NyLPC_OnErrorGoto(Error);
	}
	if(!NyLPC_iHttpPtrStream_write(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),i_buf,i_len)){
		//CLOSE
		NyLPC_OnErrorGoto(Error);
	}
	NyLPC_iHttpPtrStream_flush(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection));
	return NyLPC_TBool_TRUE;
Error:
	NyLPC_cHttpdConnection_closeSocket(i_inst->_ref_connection);
	i_inst->_payload_st=NyLPC_TcModWebSocket_ST_CLOSED;
	return NyLPC_TBool_FALSE;
}

void NyLPC_cModWebSocket_close(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TUInt16 i_code)
{
	//ストリームの状態を更新する。
	NyLPC_cModWebSocket_update(i_inst,0);

	if(i_inst->_payload_st==NyLPC_TcModWebSocket_ST_CLOSED){
		return;
	}
	//CLOSE送信
	writeClosePacket(i_inst,i_code);
	i_inst->_payload_st=NyLPC_TcModWebSocket_ST_CLOSED;
	//切断
	NyLPC_cHttpdConnection_closeSocket(i_inst->_ref_connection);
}


NyLPC_TInt16 NyLPC_cModWebSocket_testFormatV(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_fmt,va_list args)
{
	return NyLPC_cFormatWriter_length(i_fmt,args);
}
NyLPC_TInt16 NyLPC_cModWebSocket_testFormat(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_fmt,...)
{
	NyLPC_TInt16 r;
	va_list a;
	va_start(a,i_fmt);
	r=NyLPC_cFormatWriter_length(i_fmt,a);
	va_end(a);
	return r;
}


NyLPC_TBool NyLPC_cModWebSocket_startBulkWrite(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TInt16 i_len)
{
	//ストリームの状態を更新する。
	NyLPC_cModWebSocket_update(i_inst,0);
	//ペイロードヘッダの出力
	if(!NyLPC_cModWebSocket_writePayloadHeader(i_inst,i_len)){
		//CLOSE
		NyLPC_OnErrorGoto(Error);
	}
	return NyLPC_TBool_TRUE;
Error:
	NyLPC_cHttpdConnection_closeSocket(i_inst->_ref_connection);
	i_inst->_payload_st=NyLPC_TcModWebSocket_ST_CLOSED;
	return NyLPC_TBool_FALSE;
}
/**
 * バルク書き込みを終了します。
 * この関数をコールする前に、startBulkWrite関数のi_lenで指定した大きさのデータを入力し終えている必要があります。
 * 過不足があった場合、関数は失敗するか、WebSocketセッションが破壊されます。
 */
NyLPC_TBool NyLPC_cModWebSocket_endBulkWrite(NyLPC_TcModWebSocket_t* i_inst)
{
	if(i_inst->_payload_st==NyLPC_TcModWebSocket_ST_CLOSED){
		return NyLPC_TBool_FALSE;
	}
	//送信サイズ確認？
	NyLPC_iHttpPtrStream_flush(NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection));
	return NyLPC_TBool_TRUE;
}

NyLPC_TBool NyLPC_cModWebSocket_writeBulkFormatV(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_fmt,va_list args)
{
	if(i_inst->_payload_st==NyLPC_TcModWebSocket_ST_CLOSED){
		return NyLPC_TBool_FALSE;
	}
	if(!NyLPC_cFormatWriter_print(fmt_handler,NyLPC_cHttpdConnection_refStream(i_inst->_ref_connection),i_fmt,args)){
		NyLPC_OnErrorGoto(Error);
	}
	return NyLPC_TBool_TRUE;
Error:
	NyLPC_cHttpdConnection_closeSocket(i_inst->_ref_connection);
	i_inst->_payload_st=NyLPC_TcModWebSocket_ST_CLOSED;
	return NyLPC_TBool_FALSE;
}
NyLPC_TBool NyLPC_cModWebSocket_writeBulkFormat(NyLPC_TcModWebSocket_t* i_inst,const NyLPC_TChar* i_fmt,...)
{
	NyLPC_TBool ret;
	va_list a;
	va_start(a,i_fmt);
	ret=NyLPC_cModWebSocket_writeBulkFormatV(i_inst,i_fmt,a);
	va_end(a);
	return ret;
}
