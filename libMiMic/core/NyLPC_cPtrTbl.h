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
#ifndef NYLPC_PTRTABLE_H_
#define NYLPC_PTRTABLE_H_


#include "NyLPC_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************
 *
 * NyLPC_TcPtrTbl
 *
 **********************************************************************/
/**
 * ポインタを格納するテーブルです。
 * 固定長のメモリ領域をポインタテーブルとして管理します。
 * 要素はNULLを無効値とします。
 */
typedef struct NyLPC_TcPtrTbl NyLPC_TcPtrTbl_t;



struct NyLPC_TcPtrTbl
{
    /** 配列の最大サイズ*/
    NyLPC_TUInt16 size;
    /** 配列の現在の長さ*/
    NyLPC_TUInt16 len;
    /** 配列*/
    void** buf;
};


/**
 * コンストラクタです。
 * メモリを初期化します。
 * @param i_buf
 * インスタンスを格納するメモリを指定します。バッファサイズは、sizeof(void*)*i_sizeである必要があります。
 * @param i_size
 * テーブルのサイズを指定します。
 */
void NyLPC_cPtrTbl_initialize(NyLPC_TcPtrTbl_t* i_inst,void** i_buf,NyLPC_TUInt16 i_size);

void* NyLPC_cPtrTbl_get(NyLPC_TcPtrTbl_t* i_inst,NyLPC_TUInt16 i_index);

void NyLPC_cPtrTbl_set(NyLPC_TcPtrTbl_t* i_inst,NyLPC_TUInt16 i_index,void* i_val);

/**
 * リストへ要素を追加します。
 * @i_val
 * NULL以外を指定します。
 * @return
 * 追加した要素のインデクス番号を返します。
 * 失敗した場合、-1を返します。
 */
NyLPC_TInt16 NyLPC_cPtrTbl_add(NyLPC_TcPtrTbl_t* i_inst,void* i_val);

void NyLPC_cPtrTbl_remove(NyLPC_TcPtrTbl_t* i_inst,NyLPC_TUInt16 i_index);

/**
 * 現在の長さを返します。
 */
NyLPC_TInt16 NyLPC_cPtrTbl_getLength(NyLPC_TcPtrTbl_t* i_inst);

/**
 * 空き領域の有無を返します。
 */
NyLPC_TBool NyLPC_cPtrTbl_hasEmpty(NyLPC_TcPtrTbl_t* i_inst);

/**
 * ポインタに一致するインデクスを返します。
 */
NyLPC_TInt16 NyLPC_cPtrTbl_getIndex(NyLPC_TcPtrTbl_t* i_inst,void* i_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_PTRTABLE_H_ */
