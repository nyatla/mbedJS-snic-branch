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
#include "NyLPC_cFifoBuffer.h"
#include <string.h>

/**
 * See Header file.
 */
void NyLPC_cFifoBuffer_initialize(NyLPC_TcFifoBuffer_t* i_inst,void* i_buf,NyLPC_TUInt16 i_buf_size)
{
    i_inst->len=0;
    i_inst->size=i_buf_size;
    i_inst->buf=i_buf;
}


/**
 * See Header file.
 */
void NyLPC_cFifoBuffer_push(NyLPC_TcFifoBuffer_t* i_inst,const void* i_data,NyLPC_TUInt16 i_data_len)
{
    NyLPC_TUInt8* wp;
    NyLPC_ArgAssert(NyLPC_cFifoBuffer_getSpace(i_inst)>=i_data_len);

    wp=((NyLPC_TUInt8*)(i_inst->buf))+i_inst->len;
    memcpy(wp,i_data,i_data_len);
    i_inst->len+=i_data_len;
}
void* NyLPC_cFifoBuffer_prePush(NyLPC_TcFifoBuffer_t* i_inst,NyLPC_TUInt16 i_data_len)
{
    NyLPC_TUInt8* wp;
    NyLPC_ArgAssert(NyLPC_cFifoBuffer_getSpace(i_inst)>=i_data_len);
    wp=((NyLPC_TUInt8*)(i_inst->buf))+i_inst->len;
    i_inst->len+=i_data_len;
    return wp;
}

/**
 * See Header file.
 */
void NyLPC_cFifoBuffer_pop(NyLPC_TcFifoBuffer_t* i_inst,NyLPC_TUInt16 i_len)
{
    NyLPC_TUInt8* wp;
    NyLPC_TUInt8* rp;
    NyLPC_ArgAssert(NyLPC_cFifoBuffer_getLength(i_inst)>=i_len);
    wp=(NyLPC_TUInt8*)i_inst->buf;
    rp=wp+i_len;
    i_inst->len-=i_len;
    memmove(wp,rp,i_inst->len);
}

/**
 * See Header file.
 */
void* NyLPC_cFifoBuffer_getPtr(const NyLPC_TcFifoBuffer_t* i_inst)
{
    return (void*)(i_inst->buf);
}
/**
 * See Header file.
 */
NyLPC_TUInt16 NyLPC_cFifoBuffer_getLength(const NyLPC_TcFifoBuffer_t* i_inst)
{
    return i_inst->len;
}
/**
 * See Header file.
 */
NyLPC_TUInt16 NyLPC_cFifoBuffer_getSpace(const NyLPC_TcFifoBuffer_t* i_inst)
{
    return i_inst->size-i_inst->len;
}
