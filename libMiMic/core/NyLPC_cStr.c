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
#include "NyLPC_cStr.h"
#include <ctype.h>

void NyLPC_cStr_initialize(NyLPC_TcStr_t* i_inst,void* i_buf,int sizeof_buf)
{
    i_inst->s=(NyLPC_TInt16)(sizeof_buf-1);
    i_inst->l=0;
    i_inst->buf=i_buf;
    *((NyLPC_TChar*)(i_inst->buf))=0;
}

NyLPC_TBool NyLPC_cStr_put(NyLPC_TcStr_t* i_inst,NyLPC_TChar i_c)
{
    NyLPC_TChar* p=(NyLPC_TChar*)(i_inst->buf)+i_inst->l;
    if(i_inst->s-i_inst->l>0){
        *p=i_c;
        i_inst->l++;
        *(++p)=0;
        return NyLPC_TBool_TRUE;
    }
    return NyLPC_TBool_FALSE;
}

void NyLPC_cStr_toUpper(NyLPC_TcStr_t* i_inst)
{
    NyLPC_TChar* p;
    for(p=NyLPC_cStr_str(i_inst);*p!=0;p++){
        *p=(NyLPC_TChar)toupper((int)*p);
    }
    return;
}
