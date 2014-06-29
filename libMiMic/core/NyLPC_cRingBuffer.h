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
#ifndef NyLPC_TcRingBuffer_h
#define NyLPC_TcRingBuffer_h



#include "NyLPC_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************
 *
 * NyLPC_TcRingBuffer class
 *
 **********************************************************************/
typedef struct NyLPC_TcRingBuffer NyLPC_TcRingBuffer_t;


struct NyLPC_TcRingBuffer
{
    void* buf;          //バッファ
    NyLPC_TUInt16 bl;   //バッファ長さ
    NyLPC_TUInt16 wo;   //バッファ書込み位置オフセット
    NyLPC_TUInt16 ro;   //バッファ読み込み位置オフセット
};


#if NyLPC_ARCH==NyLPC_ARCH_FREERTOS
#elif NyLPC_ARCH==NyLPC_ARCH_WIN32
/**
 * リングバッファをダンプします。
 */
void NyLPC_cRingBuffer_dump(NyLPC_TcRingBuffer_t* i_inst);
#else
#endif


/**
 * i_bufをラップするリングバッファを作成します。
 */
void NyLPC_cRingBuffer_initialize(NyLPC_TcRingBuffer_t* i_inst,void* i_buf,NyLPC_TUInt16 sizeof_buf);

/**
 * この関数は、読出し可能なサイズを計算して返します。
 * @return
 * 読み出し可能なバイト数を返します。
 * 0以外の場合、もう一度読み出せる可能性があります。
 */
NyLPC_TInt16 NyLPC_cRingBuffer_getReadableSize(NyLPC_TcRingBuffer_t* i_inst);

/**
 * この関数は、書き込み可能なサイズを計算して返します。
 * @return
 * 書き込み可能なバイト数です。
 * この数値は、バッファ全体に対する空き領域と同じです。
 */
NyLPC_TInt16 NyLPC_cRingBuffer_getWritableSize(const NyLPC_TcRingBuffer_t* i_inst);

/**
 * リングバッファにデータを書き込みます。
 * @return
 * 書きこめたサイズ。
 */
int NyLPC_cRingBuffer_write(NyLPC_TcRingBuffer_t* i_inst,NyLPC_TUInt8* i_data,const int i_len);

/**
 * この関数は、リングバッファを初期化します。
 */
void NyLPC_cRingBuffer_reset(NyLPC_TcRingBuffer_t* i_inst);

/**
 * リングバッファの読出しポイントと読出し可能サイズを返します。
 */
NyLPC_TUInt8* NyLPC_cRingBuffer_pread(NyLPC_TcRingBuffer_t* i_inst,NyLPC_TUInt16* len);

/**
 * この関数は、リングバッファの読み取り点を前方にシークします。
 * @i_seek
 * シークするバイト数。NyLPC_cRingBuffer_getReadableSize,又はNyLPC_cRingBuffer_preadのlen以下のサイズである必要があります。
 */
void NyLPC_cRingBuffer_preadSeek(NyLPC_TcRingBuffer_t* i_inst,NyLPC_TUInt16 i_seek);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
