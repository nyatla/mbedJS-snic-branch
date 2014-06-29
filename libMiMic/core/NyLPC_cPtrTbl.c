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
#include "NyLPC_cPtrTbl.h"

void NyLPC_cPtrTbl_initialize(NyLPC_TcPtrTbl_t* i_inst,void** i_buf,NyLPC_TUInt16 i_size)
{
    i_inst->size=i_size;
    i_inst->len=0;
    i_inst->buf=i_buf;
    memset(i_inst->buf,0,i_size*sizeof(void*));
}
void* NyLPC_cPtrTbl_get(NyLPC_TcPtrTbl_t* i_inst,NyLPC_TUInt16 i_index)
{
    return i_inst->buf[i_index];
}

void NyLPC_cPtrTbl_set(NyLPC_TcPtrTbl_t* i_inst,NyLPC_TUInt16 i_index,void* i_val)
{
    void** p=(i_inst->buf+i_index);
    if(*p==NULL){
        if(i_val!=NULL){
            //投入
            i_inst->len++;
        }
    }else{
        if(i_val==NULL){
            //消去
            i_inst->len--;
        }
    }
    //値を反映
    *p=i_val;
    return;
}

NyLPC_TInt16 NyLPC_cPtrTbl_add(NyLPC_TcPtrTbl_t* i_inst,void* i_val)
{
    int i;
    void** p=i_inst->buf;

    for(i=i_inst->size-1;i>=0;i--){
        if(*(p+i)==NULL){
            NyLPC_cPtrTbl_set(i_inst,i,i_val);
            return i;
        }
    }
    return -1;
}

/**
 * i_indexの要素に、NULLをセットして削除します。
 */
void NyLPC_cPtrTbl_remove(NyLPC_TcPtrTbl_t* i_inst,NyLPC_TUInt16 i_index)
{
    NyLPC_cPtrTbl_set(i_inst,i_index,NULL);
    return;
}

NyLPC_TInt16 NyLPC_cPtrTbl_getIndex(NyLPC_TcPtrTbl_t* i_inst,void* i_val)
{
    int i;
    void** p=i_inst->buf;

    for(i=i_inst->size-1;i>=0;i--){
        if(*(p+i)==i_val){
            return i;
        }
    }
    return -1;
}

/**
 * 現在の要素数を返します。
 * この数は、テーブルに存在する有効なポインタの数です。
 */
NyLPC_TInt16 NyLPC_cPtrTbl_getLength(NyLPC_TcPtrTbl_t* i_inst)
{
    return i_inst->len;
}

NyLPC_TBool NyLPC_cPtrTbl_hasEmpty(NyLPC_TcPtrTbl_t* i_inst)
{
    return i_inst->len<i_inst->size;
}
