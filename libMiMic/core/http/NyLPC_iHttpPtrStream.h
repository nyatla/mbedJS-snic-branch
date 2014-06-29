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
#ifndef NyLPC_TiHttpPtrStream_H_
#define NyLPC_TiHttpPtrStream_H_

/**********************************************************************
 *
 * NyLPC_TiHttpPtrStream class
 *
 **********************************************************************/
#include "NyLPC_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Httpストリームのインタフェイス。
 */
typedef struct NyLPC_TiHttpPtrStream NyLPC_TiHttpPtrStream_t;

/**
 * ストリームのエンコーディングモード
 */
typedef NyLPC_TUInt8 NyLPC_TiHttpPtrStream_ET;
#define NyLPC_TiHttpPtrStream_ET_NONE 0x00
#define NyLPC_TiHttpPtrStream_ET_CHUNKED 0x01

/**
 * HTTP通信タイムアウトのデフォルト値
 */
#define NyLPC_TiHttpPtrStream_DEFAULT_HTTP_TIMEOUT (8*1000)


typedef NyLPC_TInt32 (*NyLPC_TiHttpPtrStream_pread)(void* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_timeout);
typedef NyLPC_TBool (*NyLPC_TiHttpPtrStream_write)(void* i_inst,const void* i_data,NyLPC_TInt32 i_length);
typedef void (*NyLPC_TiHttpPtrStream_rseek)(void* i_inst,NyLPC_TUInt16 i_seek);
typedef NyLPC_TBool (*NyLPC_TiHttpPtrStream_flush)(void* i_inst);
typedef void (*NyLPC_TiHttpPtrStream_setReadEncoding)(void* i_inst,NyLPC_TiHttpPtrStream_ET i_et);
typedef void (*NyLPC_TiHttpPtrStream_setWriteEncoding)(void* i_inst,NyLPC_TiHttpPtrStream_ET i_et);

/**
 * _interface_httpptrstreamで宣言してください。
 */
struct NyLPC_TiHttpPtrStream_TInterface
{
    NyLPC_TiHttpPtrStream_pread pread;
    NyLPC_TiHttpPtrStream_write write;
    NyLPC_TiHttpPtrStream_rseek readSeek;
    NyLPC_TiHttpPtrStream_flush flush;
    NyLPC_TiHttpPtrStream_setReadEncoding setreadencoding;
    NyLPC_TiHttpPtrStream_setWriteEncoding setwriteencoding;
};
struct NyLPC_TiHttpPtrStream
{
    /**
     * 継承クラスで実装すべきハンドラ
     */
    const struct NyLPC_TiHttpPtrStream_TInterface* absfunc;
};

/**
 * ストリームからデータを読み出して、そのポインタを返します。
 * @return
 * ストリームから読み込んだデータサイズを返します。
 * 0の場合はタイムアウトです。
 * 0未満の場合はエラーです。
 */
#define NyLPC_iHttpPtrStream_pread(i_inst,o_buf_ptr,i_timeout) (i_inst)->absfunc->pread((i_inst),(o_buf_ptr),i_timeout)
/**
 * ストリームへデータを書き込みます。
 * @param i_length
 * i_dataのデータ長を指定します。-1の場合、strlenを実行します。
 * @return
 * 規定時間内にストリームへの書き込みが完了すると、TRUEを返します。
 */
#define NyLPC_iHttpPtrStream_write(i_inst,i_data,i_length) (i_inst)->absfunc->write((i_inst),i_data,i_length)
#define NyLPC_iHttpPtrStream_rseek(i_inst,i_seek) (i_inst)->absfunc->readSeek((i_inst),(i_seek))
/**
 * バッファに残っているデータを送信し、空にします。
 */
#define NyLPC_iHttpPtrStream_flush(i_inst) (i_inst)->absfunc->flush(i_inst)

/**
 * 読み出しエンコーディングの設定。
 * この関数は削除するかもしれない。使用しないこと。
 * @bug 関数が非対称。cHttpBasicBodyParserがチャンク読み出し処理を肩代わりしている?
 */
#define NyLPC_iHttpPtrStream_setReadEncoding(i_inst,i_et) (i_inst)->absfunc->setreadencoding((i_inst),i_et)
/**
 * 書込みエンコーディングの設定。
 */
#define NyLPC_iHttpPtrStream_setWriteEncoding(i_inst,i_et) (i_inst)->absfunc->setwriteencoding((i_inst),i_et)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NyLPC_TiHttpPtrStream_H_ */
