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
/**
 * @file
 * このファイルは、NyLPC_cFifoBufferクラスを定義します。
 */
#ifndef NyLPC_TcFifoBuffer_H
#define NyLPC_TcFifoBuffer_H

#include "NyLPC_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * クラス型を定義します。
 * NyLPC_cFifoBufferクラスは、固定長のメモリを、最大bit長のFIFOメモリーとして扱うためのラップクラスです。
 */
typedef struct NyLPC_TcFifoBuffer NyLPC_TcFifoBuffer_t;

/**
 * NyLPC_TcFifoBufferクラスの構造体です。
 */
struct NyLPC_TcFifoBuffer
{
    /** 配列の最大サイズ*/
    NyLPC_TUInt16 size;
    /** 配列の現在の長さ*/
    NyLPC_TUInt16 len;
    /** バッファ領域*/
    void* buf;
};

/**
 * コンストラクタです。
 * i_instを、i_bufをラップするインスタンスとして初期化します。
 * i_bufは、finalizeを呼び出すまで、クラスが参照します。
 * インスタンスを削除するまでの間、維持してください。
 * @param i_inst
 * 初期化するメモリブロックのアドレス。
 * @param i_buf
 * ラップするメモリブロックのアドレス
 * @param i_buf_size
 * i_bufのサイズ。
 */
void NyLPC_cFifoBuffer_initialize(NyLPC_TcFifoBuffer_t* i_inst,void* i_buf,NyLPC_TUInt16 i_buf_size);

/**
 * デストラクタです。
 * インスタンスの確保しているリソースを開放します。
 * @param i_inst
 * 開放するインスタンスのポインタ
 */
#define NyLPC_cFifoBuffer_finalize(i_inst)

/**
 * この関数は、FIFOバッファの有効データ長を0にしてリセットします。
 * @param i_inst
 * 操作するインスタンスのポインタ
 */
#define NyLPC_cFifoBuffer_clear(i_inst) (i_inst)->len=0;

/**
 * この関数は、バッファの後方にデータをコピーして追記します。
 * 十分なサイズがない場合、ASSERTします。
 * 書込み可能な最大サイズは、getSpace関数で得ることが出来ます。
 * @param i_inst
 * 操作するインスタンスのポインタ
 * @param i_data
 * 追記するデータ。
 * @param i_data_len
 * 追記するデータのサイズ
 *
 */
void NyLPC_cFifoBuffer_push(NyLPC_TcFifoBuffer_t* i_inst,const void* i_data,NyLPC_TUInt16 i_data_len);

/**
 * この関数は、バッファの先頭からデータを削除します。
 * 十分なデータがない場合、ASSERTします。
 * この関数は、戻り値を返しません。getPtrで得たポインタからデータを読み込んだ後に、読み込んだデータをバッファから削除するために使います。
 * @param i_inst
 * 操作するインスタンスのポインタ
 * @param i_data
 * 削除するデータのサイズ。getLengthの戻り値以下である必要があります。
 */
void NyLPC_cFifoBuffer_pop(NyLPC_TcFifoBuffer_t* i_inst,NyLPC_TUInt16 i_len);

/**
 * バッファの先頭ポインタを得ます。
 * @param i_inst
 * 操作するインスタンスのポインタ
 * @return
 * バッファの先頭ポインタを返します。値は、次回にpush/popをするまでの間有効です。
 */
void* NyLPC_cFifoBuffer_getPtr(const NyLPC_TcFifoBuffer_t* i_inst);

/**
 * 格納しているデータの長さを返します。
 * getPtrで得たポインタから読み出せるデータのサイズに相当します。
 * @param i_inst
 * 操作するインスタンスのポインタ
 * @return
 * 読み出せるデータの長さです。
 */
NyLPC_TUInt16 NyLPC_cFifoBuffer_getLength(const NyLPC_TcFifoBuffer_t* i_inst);

/**
 * バッファの残量を計算して返します。
 * この値は、push関数で追記できるデータサイズと同じです。
 * @param i_inst
 * 操作するインスタンスのポインタ
 * @return
 * バッファの空き領域のサイズです。
 */
NyLPC_TUInt16 NyLPC_cFifoBuffer_getSpace(const NyLPC_TcFifoBuffer_t* i_inst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
