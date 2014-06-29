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
#ifndef NYLPC_CSTR_H_
#define NYLPC_CSTR_H_

#include "NyLPC_stdlib.h"
/**********************************************************************
 *
 * NyLPC_TcStr class
 *
 **********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * 長さ付き文字列を定義します。
 */
typedef struct NyLPC_TcStr NyLPC_TcStr_t;

struct NyLPC_TcStr
{
    NyLPC_TInt16 s;//バッファサイズ
    NyLPC_TInt16 l;//現在の文字列長
    void* buf;
};

/**
バッファをNyLPC_cStr_tに初期化して、キャストして返します。
*/
void NyLPC_cStr_initialize(NyLPC_TcStr_t* i_inst,void* i_buf,int sizeof_buf);

#define NyLPC_cStr_finalize(i)

/**
 * 文字をバッファへ追加します。
 * @return
 * TRUE 追加に成功;FALSE 追加に失敗
 */
NyLPC_TBool NyLPC_cStr_put(NyLPC_TcStr_t* i_inst,NyLPC_TChar i_c);

/**
 * 残容量を返します。
 * @return
 * バイト単位のバッファの残り容量
 */
#define NyLPC_cStr_capacity(i_inst) ((i_inst)->s-(i_inst)->l)

/**
現在の長さを返します。
*/
#define NyLPC_cStr_len(i_inst) ((i_inst)->l)
/**
 * 文字列ポインタを返します。
　*/
#define NyLPC_cStr_str(i_inst) ((NyLPC_TChar*)((i_inst)->buf))
/**
文字列長さをリセットします。
*/
#define NyLPC_cStr_clear(i_inst) (i_inst)->l=0
/**
 * NULL terminated文字列が同一か返します。
 */
#define NyLPC_cStr_isEqual(i_inst,i_str) (strcmp(NyLPC_cStr_str(i_inst),(i_str))==0)
/**
 * NULL terminated文字列が同一か返します。
 * 大文字小文字を区別しません。
 */
#define NyLPC_cStr_isEqualIgnoreCase(i_inst,i_str) (NyLPC_stricmp(NyLPC_cStr_str(i_inst),(i_str))==0)

/**
 * 文字列を大文字にします。
 *
 *
*/
void NyLPC_cStr_toUpper(NyLPC_TcStr_t* i_inst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CSTR_H_ */
