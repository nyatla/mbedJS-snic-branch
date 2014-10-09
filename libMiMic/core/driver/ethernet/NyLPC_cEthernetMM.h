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
/**
 * @file
 * このファイルは、イーサネットメモリマネージャクラスを定義します。
 */
#ifndef NyLPC_cEthernetMM_protected_h
#define NyLPC_cEthernetMM_protected_h

#include "NyLPC_stdlib.h"
#include "NyLPC_IEthernetDevice.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef PACK_STRUCT_END
    #define PACK_STRUCT_END __attribute((packed))
#endif

/**
 * 構造体のアライメントサイズ(4 or 16)
 */
#ifndef NyLPC_TTxBufferHeader_ALIGNMENT
#	define NyLPC_TTxBufferHeader_ALIGNMENT 4
#endif

/**
 *バッファメモリのパディングサイズ
 */
#ifndef NyLPC_TcEthernetMM_BUF_PADDING
#	define NyLPC_TcEthernetMM_BUF_PADDING 0
#endif

/**
 * 送信バッフメモリのヘッダ。
 * この構造体は、TXバッファメモリブロックのヘッダーです。
 * TXバッファメモリブロックは、この構造体の後ろに、sizeに一致したメモリを連結したもので表現します。
 * <pre>
 * buffer=[struct NyLPC_TTxBufferHeader][n]
 * </pre>
 */
struct NyLPC_TTxBufferHeader
{
	//メモリブロックの参照カウンタ。
	NyLPC_TInt8  ref;
	//送信用にロックしたかを示すフラグ
	NyLPC_TUInt8 is_lock;
	//Nビット境界に合わせるためのパディング。
	NyLPC_TUInt8 padding[NyLPC_TTxBufferHeader_ALIGNMENT-2];
}PACK_STRUCT_END;

/**
 * バッファメモリアドレスからメモリヘッダアドレスを復元します。
 */
#define NyLPC_TTxBufferHeader_getBufferHeaderAddr(a)	((struct NyLPC_TTxBufferHeader*)(((NyLPC_TUInt8*)a)-sizeof(struct NyLPC_TTxBufferHeader)))


/**
 * NyLPC_cEthernetMM_allocのヒント値。
 * コントロールパケット用のサイズ要求をするときに使用します。
 */
#define NyLPC_TcEthernetMM_HINT_CTRL_PACKET 0


/**
 * メモリブロック構造体の定数値
 */
#define NyLPC_TcEthernetMM_NUM_OF_MAX_BUF  3
#define NyLPC_TcEthernetMM_NUM_OF_512_BUF  3
#define NyLPC_TcEthernetMM_NUM_OF_256_BUF  4
#define NyLPC_TcEthernetMM_NUM_OF_128_BUF 16
#define NyLPC_TcEthernetMM_NUM_OF_64_BUF   4

/**
 * FULLサイズのEthernetFrame送信メモリのサイズ。
 * ここで最大送信サイズを制限します。
 * 通常は1460+20+20+14=1514バイト
 * パディングと合計で128bit(16byte)アライメントにしておかないと不幸になる。
 */
#define NyLPC_TcEthernetMM_MAX_TX_ETHERNET_FRAME_SIZE	1514
/**
 * Alignment padding(128bit)
 */
#define NyLPC_TcEthernetMM_MAX_TX_ETHERNET_PADDING		22


/**
 * TXメモリブロックの定義配列
 */
struct NyLPC_TcEthernetMM_TxMemoryBlock
{
	struct{
		struct NyLPC_TTxBufferHeader h;
		NyLPC_TUInt8 b[NyLPC_TcEthernetMM_MAX_TX_ETHERNET_FRAME_SIZE];
		NyLPC_TUInt8 _padding[NyLPC_TcEthernetMM_MAX_TX_ETHERNET_PADDING+NyLPC_TcEthernetMM_BUF_PADDING];
	}buf_max[NyLPC_TcEthernetMM_NUM_OF_MAX_BUF];//(4+MAX_TX_ETHERNET_FRAME_SIZE(1514))*3=? default=4554
	struct{
		struct NyLPC_TTxBufferHeader h;
		NyLPC_TUInt8 b[512+NyLPC_TcEthernetMM_BUF_PADDING];
	}buf_512[NyLPC_TcEthernetMM_NUM_OF_512_BUF];//(4+512)*3=1548
	struct{
		struct NyLPC_TTxBufferHeader h;
		NyLPC_TUInt8 b[256+NyLPC_TcEthernetMM_BUF_PADDING];
	}buf_256[NyLPC_TcEthernetMM_NUM_OF_256_BUF];//(4+256)*4=1560
	struct{
		struct NyLPC_TTxBufferHeader h;
		NyLPC_TUInt8 b[128+NyLPC_TcEthernetMM_BUF_PADDING];
	}buf_128[NyLPC_TcEthernetMM_NUM_OF_128_BUF];//(4+128)*16=1584
	struct{
		struct NyLPC_TTxBufferHeader h;
		NyLPC_TUInt8 b[64+NyLPC_TcEthernetMM_BUF_PADDING];
	}buf_64[NyLPC_TcEthernetMM_NUM_OF_64_BUF];//(4+64)*4=272
}PACK_STRUCT_END;



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
 * パケットバッファの先頭アドレス
 */
void* NyLPC_cEthernetMM_alloc(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size);
/**
 * NyLPC_EthernetMM_allocで得たメモリを解放します。
 */
void NyLPC_cEthernetMM_release(void* i_buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
