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
#ifndef NyLPC_cPtrStream_protected_h
#define NyLPC_cPtrStream_protected_h
#include "NyLPC_cPtrStream.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



NyLPC_TInt32 NyLPC_cPtrStream_pread_func(NyLPC_TcPtrStream_t* i_inst,const void** o_buf_ptr,NyLPC_TUInt32 i_wait_msec);
NyLPC_TBool NyLPC_cPtrStream_write_func(NyLPC_TcPtrStream_t* i_inst,const void* i_data,NyLPC_TInt16 i_length,NyLPC_TUInt32 i_wait_msec);
void NyLPC_cPtrStream_pseek_func(NyLPC_TcPtrStream_t* i_inst,NyLPC_TUInt16 i_seek);
void NyLPC_cPtrStream_close_func(NyLPC_TcPtrStream_t* i_inst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
