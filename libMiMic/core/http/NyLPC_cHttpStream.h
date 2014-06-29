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
#ifndef NyLPC_TcHttpStream_h
#define NyLPC_TcHttpStream_h

#include "NyLPC_config.h"
#include "NyLPC_iHttpPtrStream.h"



#if NyLPC_CONFIG_cHttpStream_DEBUG == 1
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef int NyLPC_TcTcpSocket_t;
void NyLPC_cTcpSocket_initialized(void* inst,const char* rb,int l);
void* NyLPC_cTcpSocket_allocSendBuf(void* inst,NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_len,NyLPC_TUInt32 i_to);
NyLPC_TBool NyLPC_cTcpSocket_psend(void* inst,void* i_buf,NyLPC_TUInt16 i_len,NyLPC_TUInt32 i_to);
NyLPC_TInt32 NyLPC_cTcpSocket_precv(void* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_wait_msec);
void NyLPC_cTcpSocket_pseek(void* i_inst,NyLPC_TUInt16 i_seek);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else
#include "../uip/NyLPC_cTcpSocket.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * HttpStreamを供給するためのクラスです。このクラスは、NyLPC_TiHttpPtrStream_TInterfaceインタフェイスを実装します。
 */
typedef struct NyLPC_TcHttpStream NyLPC_TcHttpStream_t;



struct NyLPC_TcHttpStream
{
    NyLPC_TiHttpPtrStream_t super;
    NyLPC_TcTcpSocket_t* _ref_sock;
    NyLPC_TUInt8* txb;//送信バッファ
    NyLPC_TUInt16 txb_size;//送信バッファサイズ
    NyLPC_TUInt16 tx_len;  //送信サイズ
    NyLPC_TiHttpPtrStream_ET re_type;
    NyLPC_TiHttpPtrStream_ET we_type;
};



/**
 * 接続済のソケットをストリームに抽象化します。
 * このインスタンスは、NyLPC_TiHttpPtrStream_TInterfaceインタフェイスを提供します。
 * @
 */
NyLPC_TBool NyLPC_cHttpStream_initialize(NyLPC_TcHttpStream_t* i_inst,NyLPC_TcTcpSocket_t* i_ref_sock);

void NyLPC_cHttpStream_finalize(NyLPC_TcHttpStream_t* i_inst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
