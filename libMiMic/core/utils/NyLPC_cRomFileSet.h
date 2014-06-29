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
#ifndef NYLPC_CROMFILESET_H_
#define NYLPC_CROMFILESET_H_
#include <stdlib.h>
#include "NyLPC_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * オンメモリファイルの構造体。
 */
struct NyLPC_TRomFileData{
    const char* name;
    NyLPC_TUInt32 size;
    const char* content_type;
    const char* data;
};
/**
 * このクラスは、NyLPC_TRomFileData構造体の配列を管理します。
 */
typedef struct NyLPC_TcRomFileSet NyLPC_TcRomFileSet_t;

struct NyLPC_TcRomFileSet
{
    const struct NyLPC_TRomFileData** _ref_fs;
    NyLPC_TUInt32 _num_of_fs;
};

/**
 * このクラスは、オンメモリデータをファイルとして提供します。
 * RomFileのセットを初期化します。
 * @param i_inst
 * 初期化するインスタンスのポインタを指定します。
 * @param i_ref_fs
 * ROMFS
 */
void NyLPC_cRomFileSet_initialize(NyLPC_TcRomFileSet_t* i_inst,const struct NyLPC_TRomFileData* i_ref_fs[],NyLPC_TUInt32 i_num_of_file);
#define NyLPC_cRomFileSet_finalize(i)

/**
 * 名前に一致するROMファイルデータセットを取得します。
 */
const struct NyLPC_TRomFileData* NyLPC_cRomFileSet_getFilaData(NyLPC_TcRomFileSet_t* i_inst,const NyLPC_TChar* i_name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CROMFILE_H_ */
