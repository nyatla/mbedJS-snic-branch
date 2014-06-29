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
#include <stdlib.h>


const struct NyLPC_TcPtrStream_TInterface NyLPC_TcPtrStream_Interface={
    NyLPC_cPtrStream_pread_func,
    NyLPC_cPtrStream_write_func,
    NyLPC_cPtrStream_pseek_func,
    NyLPC_cPtrStream_close_func
};

/*private*/
NyLPC_TInt32 NyLPC_cPtrStream_pread_func(NyLPC_TcPtrStream_t* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_wait_msec)
{
    (void)i_inst;
    (void)o_buf_ptr;
    (void)i_wait_msec;
    return -1;
}

NyLPC_TBool NyLPC_cPtrStream_write_func(NyLPC_TcPtrStream_t* i_inst,const void* i_data,NyLPC_TInt16 i_length,NyLPC_TUInt32 i_wait_msec)
{
    (void)i_inst;
    (void)i_data;
    (void)i_length;
    (void)i_wait_msec;
    return NyLPC_TBool_FALSE;
}


void NyLPC_cPtrStream_pseek_func(NyLPC_TcPtrStream_t* i_inst,NyLPC_TUInt16 i_seek)
{
    (void)i_inst;
    (void)i_seek;
}

void NyLPC_cPtrStream_close_func(NyLPC_TcPtrStream_t* i_inst)
{
    (void)i_inst;
}


/**
 * See Header file.
 */
NyLPC_TBool NyLPC_cPtrStream_writeln(NyLPC_TcPtrStream_t* i_inst,const void* i_data,NyLPC_TInt16 i_length,NyLPC_TUInt32 i_wait_msec)
{
    if(NyLPC_cPtrStream_write(i_inst,i_data,i_length,i_wait_msec)){
        if(NyLPC_cPtrStream_write(i_inst,"\r\n",2,i_wait_msec)){
            return NyLPC_TBool_TRUE;
        }
    }
    return NyLPC_TBool_FALSE;
}
/**
 * See Header file.
 */
NyLPC_TBool NyLPC_cPtrStream_writeInt(NyLPC_TcPtrStream_t* i_inst,NyLPC_TInt32 i_val,NyLPC_TUInt32 i_wait_msec,NyLPC_TUInt32 i_base)
{
    NyLPC_TChar v[12];
    NyLPC_itoa(i_val,v,i_base);
    if(NyLPC_cPtrStream_write(i_inst,v,strlen(v),i_wait_msec)){
        return NyLPC_TBool_TRUE;
    }
    return NyLPC_TBool_FALSE;
}
