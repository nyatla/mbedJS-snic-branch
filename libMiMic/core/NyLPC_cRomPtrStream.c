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
#include "NyLPC_cPtrStream_protected.h"

/**
 * ROMをラップするブロッキングストリーム型です。
 * このクラスは、NyLPC_TcBlockingStreamにキャストすることができます。
 特性
 このクラスは、read onlyです。write動作は永久に失敗します。
 また、タイムアウト値は意味を持たず、最終データを読みだした後はカウント0でタイムアウトと同じ挙動を示します。
 */
typedef struct NyLPC_TcRomPtrStream NyLPC_TcRomPtrStream_t;

/**********************************************************************
 *
 * NyLPC_TcRomBlockingStream class
 *
 **********************************************************************/

struct NyLPC_TcRomPtrStream
{
    NyLPC_TcPtrStream_t _super;
    NyLPC_TChar* _rom;          //ROMアドレス
    NyLPC_TInt32 _rom_size;     //ROMサイズ
    NyLPC_TInt32 _ed;           //読出し済みサイズ
    NyLPC_TInt16 _packet_size;  //一度当たりの読出しサイズ制限
};

NyLPC_TBool NyLPC_TcRomPtrStream_initialize(NyLPC_TcRomPtrStream_t* i_inst,void* i_rom_addr,NyLPC_TInt32 i_length,NyLPC_TInt16 i_packetsize);

#define NyLPC_TcRomPtrStream_finalize(i_inst)

static NyLPC_TInt32 m_pread(NyLPC_TcPtrStream_t* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_wait_msec);
static void m_close(NyLPC_TcPtrStream_t* i_inst);
static void m_preadSeek_func(NyLPC_TcPtrStream_t* i_inst,NyLPC_TUInt16 i_seek);

//関数テーブル
static struct NyLPC_TcPtrStream_TInterface NyLPC_TcRomPtrStream_Interface=
{
    m_pread,
    NyLPC_cPtrStream_write_func,
    m_preadSeek_func,
    m_close
};



NyLPC_TBool NyLPC_TcRomPtrStream_initialize(NyLPC_TcRomPtrStream_t* i_inst,void* i_rom_addr,NyLPC_TInt32 i_length,NyLPC_TInt16 i_packetsize)
{
    NyLPC_ArgAssert(i_inst!=NULL);
    NyLPC_ArgAssert(i_rom_addr!=NULL);
    NyLPC_ArgAssert(i_length>0);
    NyLPC_ArgAssert(i_packetsize>0);
    i_inst->_super._interface=&NyLPC_TcRomPtrStream_Interface;
    i_inst->_rom=(NyLPC_TChar*)i_rom_addr;
    i_inst->_rom_size=i_length;
    i_inst->_packet_size=i_packetsize;
    return NyLPC_TBool_TRUE;
}

static void m_preadSeek_func(NyLPC_TcPtrStream_t* i_inst,NyLPC_TUInt16 i_seek)
{
    (void)i_inst;
    NyLPC_TcRomPtrStream_t* inst=(NyLPC_TcRomPtrStream_t*)i_inst;
    inst->_rom+=i_seek;
}


/*private*/
static NyLPC_TBool m_pread(NyLPC_TcPtrStream_t* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_wait_msec)
{
    NyLPC_TInt32 size,psize;
    NyLPC_TcRomPtrStream_t* inst=(NyLPC_TcRomPtrStream_t*)i_inst;
    //引数
    NyLPC_ArgAssert(i_inst!=NULL);
    NyLPC_ArgAssert(o_buf_ptr!=NULL);
    //クローズしてない？
    NyLPC_Assert(inst->_rom!=NULL);

    size=inst->_rom_size-inst->_ed;     //残り長さ計算
    psize=(size>inst->_packet_size)?inst->_packet_size:size;    //パケットサイズ計算
    *o_buf_ptr=(inst->_rom+inst->_ed);  //現在位置を返す
    inst->_ed+=psize;//読出し位置更新
    return psize;
}


static void m_close(NyLPC_TcPtrStream_t* i_inst)
{
    NyLPC_TcRomPtrStream_t* inst=(NyLPC_TcRomPtrStream_t*)i_inst;
    inst->_rom=NULL;
}



