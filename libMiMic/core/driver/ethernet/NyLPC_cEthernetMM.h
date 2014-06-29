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

#ifndef NyLPC_cEthernetMM_protected_h
#define NyLPC_cEthernetMM_protected_h

#include "NyLPC_stdlib.h"
#include "NyLPC_IEthernetDevice.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * NyLPC_cEthernetMM_allocのヒント値。
 * コントロールパケット用のサイズ要求をするときに使用します。
 */
#define NyLPC_TcEthernetMM_HINT_CTRL_PACKET 0
/**
 * @file
 * このファイルは、イーサネットメモリマネージャクラスを定義します。
 */

int NyLPC_cEthernetMM_dbg_getNumofUsedTx(void);

/**
 * メモリブロックを初期化してメモリマネージャを構築します。
 * メモリサイズはsizeof(struct TTxMemoryBlock)以上である必要があります。
 */
void NyLPC_cEthernetMM_initialize(void* i_memblock_addr);
#define NyLPC_cEthernetMM_finalize(i)
/**
 * メモリを割り当てます。
 * @param i_hint
 * 割り当てるメモリサイズのヒント。
 * 数値の場合、128バイト以上のもっともhintに近いメモリを割り当てます。
 * 以下の定義値の場合、特別な領域を優先して返します。たぶん。
 * <ul>
 * <li>NyLPC_TcEthernetMM_HINT_CTRL_PACKET - 64
 * </ul>
 * @return
 * 割り当て不能な場合はNULLが帰ります。
 * @bug
 * 戻り値、メモリブロックヘッダ不要では？
 */
struct NyLPC_TTxBufferHeader* NyLPC_cEthernetMM_alloc(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size);
/**
 * NyLPC_EthernetMM_allocで得たメモリを解放します。
 */
void NyLPC_cEthernetMM_release(struct NyLPC_TTxBufferHeader* i_buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
