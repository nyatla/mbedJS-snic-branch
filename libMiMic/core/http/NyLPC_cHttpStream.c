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
#include "NyLPC_cHttpStream.h"



#if NyLPC_CONFIG_cHttpStream_DEBUG == 1
#include <stdio.h>
char _wbuf[1024];
const char* _rbuf;
int _rbuf_len;
void NyLPC_iTcpSocket_initialized(void* inst,const char* rb,int l)
{
    _rbuf=rb;
    _rbuf_len=l;

}
void* NyLPC_iIpTcpSocket_allocSendBuf(void* inst,NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_len,NyLPC_TUInt32 i_to)
{
    *o_len=30;
    return _wbuf;
}
NyLPC_TBool NyLPC_iTcpSocket_psend(void* inst,void* i_buf,NyLPC_TUInt16 i_len,NyLPC_TUInt32 i_to)
{
    printf("%.*s",i_len,i_buf);
    return NyLPC_TBool_TRUE;
}
NyLPC_TInt32 NyLPC_iTcpSocket_precv(void* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_wait_msec)
{
    int l=(_rbuf_len>100)?100:_rbuf_len;
    *o_buf_ptr=_rbuf;
    return l;
}
void NyLPC_iTcpSocket_pseek(void* i_inst,NyLPC_TUInt16 i_seek)
{
    _rbuf+=i_seek;
    _rbuf_len-=i_seek;
}

void* NyLPC_iTcpSocket_releaseSendBuf(NyLPC_TiTcpSocket_t* i_inst,void* i_buf_ptr)
{
    return NULL;
}
#endif


static NyLPC_TInt32 pread_func(void* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_timeout);
static NyLPC_TBool write_func(void* i_inst,const void* i_data,NyLPC_TInt32 i_length);
static void pseek_func(void* i_inst,NyLPC_TUInt16 i_seek);
static NyLPC_TBool flush_func(void* i_inst);
static void setReadEncoding_func(void* i_inst,NyLPC_TiHttpPtrStream_ET i_et);
static void setWriteEncoding_func(void* i_inst,NyLPC_TiHttpPtrStream_ET i_et);


/**
 * HTTP送信バッファのヒント値
 */
#define HTTP_TX_BUF_HINT 1024

//関数テーブル
const static struct NyLPC_TiHttpPtrStream_TInterface _interface=
{
        pread_func,
        write_func,
        pseek_func,
        flush_func,
        setReadEncoding_func,
        setWriteEncoding_func
};

/**
 * i_bufに5バイトのchunkedヘッダを書きます。
 */
static void put_chunked_header(NyLPC_TUInt16 i_val,NyLPC_TUInt8* o_buf)
{
    const static char* D="0123456789ABCDEF";
    *(o_buf+0)=D[((i_val&0x0f00)>>8)];
    *(o_buf+1)=D[((i_val&0x00f0)>>4)];
    *(o_buf+2)=D[ (i_val&0x000f)];
    *(o_buf+3)='\r';
    *(o_buf+4)='\n';
}
/**
 * 接続済のソケットをストリームに抽象化します。
 */
NyLPC_TBool NyLPC_cHttpStream_initialize(NyLPC_TcHttpStream_t* i_inst,NyLPC_TiTcpSocket_t* i_ref_sock)
{
    i_inst->super.absfunc=&_interface;
    i_inst->_ref_sock=i_ref_sock;
    i_inst->we_type=NyLPC_TiHttpPtrStream_ET_NONE;
    i_inst->re_type=NyLPC_TiHttpPtrStream_ET_NONE;
    i_inst->txb=NULL;
    return NyLPC_TBool_TRUE;
}

void NyLPC_cHttpStream_finalize(NyLPC_TcHttpStream_t* i_inst)
{
    if(i_inst->txb!=NULL){
        NyLPC_iTcpSocket_releaseSendBuf(i_inst->_ref_sock,i_inst->txb);
    }
}

//
//  インタフェイス
//

static NyLPC_TInt32 pread_func(void* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_timeout)
{
    NyLPC_TcHttpStream_t* inst=(NyLPC_TcHttpStream_t*)i_inst;
    return NyLPC_iTcpSocket_precv(inst->_ref_sock,o_buf_ptr,i_timeout);
}

static NyLPC_TBool write_func(void* i_inst,const void* i_data,NyLPC_TInt32 i_length)
{
    NyLPC_TcHttpStream_t* inst=(NyLPC_TcHttpStream_t*)i_inst;
    NyLPC_TUInt16 s,free_size;
    NyLPC_TUInt32 l;
    const char* src=(const char*)i_data;
    l=((i_length<0)?strlen(src):i_length);
    while(l>0){
        //送信バッファがNULLなら、割り当て。
        if(inst->txb==NULL){
            inst->txb=(NyLPC_TUInt8*)NyLPC_iTcpSocket_allocSendBuf(inst->_ref_sock,HTTP_TX_BUF_HINT,&s,NyLPC_TiHttpPtrStream_DEFAULT_HTTP_TIMEOUT);
            if(inst->txb==NULL){
                return NyLPC_TBool_FALSE;
            }
            //chunked encodingなら、先頭5バイト+末尾2バイトを予約する. 000
            if(inst->we_type==NyLPC_TiHttpPtrStream_ET_CHUNKED){
                inst->tx_len=5;
                inst->txb_size=s-7;
            }else{
                inst->tx_len=0;
                inst->txb_size=s;
            }
        }
        //書き込み可能サイズの計算
        free_size=inst->txb_size-inst->tx_len;
        if((NyLPC_TInt32)free_size>l){
            //書き込み可能サイズがi_length未満なら、バッファに貯めるだけ。
            memcpy(inst->txb+inst->tx_len,src,l);
            inst->tx_len+=(NyLPC_TUInt16)l;
            break;
        }
        //バッファフルになるなら、送信する。
        memcpy(inst->txb+inst->tx_len,src,free_size);
        inst->tx_len+=free_size;
        //書き込み
        if(!flush_func(i_inst)){
            //書込みエラー・・・
            return NyLPC_TBool_FALSE;
        }
        //読み出し位置の調整
        l-=free_size;
        src+=free_size;
    };
    return NyLPC_TBool_TRUE;
}


static void pseek_func(void* i_inst,NyLPC_TUInt16 i_seek)
{
    NyLPC_TcHttpStream_t* inst=(NyLPC_TcHttpStream_t*)i_inst;

    NyLPC_iTcpSocket_pseek(inst->_ref_sock,i_seek);
}

/**
 * キャッシュに保持してるデータを出力する。
 */
static NyLPC_TBool flush_func(void* i_inst)
{
    NyLPC_TcHttpStream_t* inst=(NyLPC_TcHttpStream_t*)i_inst;
    if(inst->txb==NULL){
        return NyLPC_TBool_TRUE;
    }
    //chunkedの場合は、header/footerをセットする。
    if(inst->we_type==NyLPC_TiHttpPtrStream_ET_CHUNKED){
        //5バイト分のヘッダを記述。
        put_chunked_header(inst->tx_len-5,inst->txb);
        *(inst->txb+inst->tx_len)=0x0d;
        *(inst->txb+inst->tx_len+1)=0x0a;
        inst->tx_len+=2;
    }
    //送信する。
    if(!NyLPC_iTcpSocket_psend(inst->_ref_sock,inst->txb,inst->tx_len,NyLPC_TiHttpPtrStream_DEFAULT_HTTP_TIMEOUT)){
        //失敗。
        NyLPC_iTcpSocket_releaseSendBuf(inst->_ref_sock,inst->txb);
        inst->txb=NULL;
        return NyLPC_TBool_FALSE;
    }
    //キャッシュを開放
    inst->txb=NULL;
    return NyLPC_TBool_TRUE;
}
static void setReadEncoding_func(void* i_inst,NyLPC_TiHttpPtrStream_ET i_et)
{
    //未実装。(この関数は不要？)
    NyLPC_Abort();
    return;
}
static void setWriteEncoding_func(void* i_inst,NyLPC_TiHttpPtrStream_ET i_et)
{
    NyLPC_TcHttpStream_t* inst=(NyLPC_TcHttpStream_t*)i_inst;
    if(inst->we_type==i_et)
    {
        return;
    }
    //バッファがあるならフラッシュしてしまう。
    if(inst->txb!=NULL){
        if(!flush_func(i_inst)){
            return;
        }
    }
    //モードの切り替え。
    inst->we_type=i_et;
}
