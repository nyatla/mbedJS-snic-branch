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
#ifndef NYLPC_TCPTRSTREAM_H_
#define NYLPC_TCPTRSTREAM_H_

/**********************************************************************
 *
 * NyLPC_TcPtrStream class
 *
 **********************************************************************/
#include "NyLPC_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * ブロッキングストリームのベース型です。
 */
typedef struct NyLPC_TcPtrStream NyLPC_TcPtrStream_t;


typedef NyLPC_TInt32 (*NyLPC_TcPtrStream_pread)(NyLPC_TcPtrStream_t* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_wait_msec);
typedef NyLPC_TBool (*NyLPC_TcPtrStream_write)(NyLPC_TcPtrStream_t* i_inst,const void* i_data,NyLPC_TInt16 i_length,NyLPC_TUInt32 i_wait_msec);
typedef void (*NyLPC_TcPtrStream_rseek)(NyLPC_TcPtrStream_t* i_inst,NyLPC_TUInt16 i_seek);
typedef void (*NyLPC_TcPtrStream_close)(NyLPC_TcPtrStream_t* i_inst);

struct NyLPC_TcPtrStream_TInterface
{
    NyLPC_TcPtrStream_pread pread;
    NyLPC_TcPtrStream_write write;
    NyLPC_TcPtrStream_rseek readSeek;
    NyLPC_TcPtrStream_close close;
};


struct NyLPC_TcPtrStream
{
    const struct NyLPC_TcPtrStream_TInterface* _interface;
};

/**
 * ストリームからデータを読み出して、そのポインタを返します。
 * @return
 * ストリームから読み込んだデータサイズを返します。
 * 0の場合はタイムアウトです。
 * 0未満の場合はエラーです。
 */
#define NyLPC_cPtrStream_read(i_inst,o_buf_ptr,i_wait_msec) (i_inst)->_interface->read((i_inst),o_buf_ptr,i_wait_msec)
/**
 * ストリームへデータを書き込みます。
 * @param i_length
 * i_dataのデータ長を指定します。-1の場合、strlenを実行します。
 * @return
 * 規定時間内にストリームへの書き込みが完了すると、TRUEを返します。
 */
#define NyLPC_cPtrStream_write(i_inst,i_data,i_length,i_wait_msec) (i_inst)->_interface->write((i_inst),i_data,i_length,i_wait_msec)

/**
 * 改行付でストリームへ書き込みます。
 */
NyLPC_TBool NyLPC_cPtrStream_writeln(NyLPC_TcPtrStream_t* i_inst,const void* i_data,NyLPC_TInt16 i_length,NyLPC_TUInt32 i_wait_msec);
/**
 * 数値をストリームへ書き込みます。
 * @param i_base
 * 10以上を指定してください。
 */
NyLPC_TBool NyLPC_cPtrStream_writeInt(NyLPC_TcPtrStream_t* i_inst,NyLPC_TInt32 i_val,NyLPC_TUInt32 i_wait_msec,NyLPC_TUInt32 i_base);

/**
 * 書き込みメモリを要求します。
 * @return
 * 書き込みメモリの先頭ポインタを返します。この値は次回のpwriteに入力してください。
 */
#define NyLPC_cPtrStream_wreserv(i_inst,i_hint,o_len)


#define NyLPC_cPtrStream_rseek(i_inst,i_seek) (i_inst)->_interface->preadSeek((i_inst),i_seek)
/**
ストリームに対する操作を閉じます。
readの場合は占有している共有メモリの解放、ブロックしているタスクの解放をします。
writeの場合はバッファに残っているデータの書き込みを保証します。
*/
#define NyLPC_cPtrStream_close(i_inst) i_inst->_interface->close(i_inst)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_TCPTRSTREAM_H_ */
