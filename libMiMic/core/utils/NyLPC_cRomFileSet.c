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
#include "NyLPC_cRomFileSet.h"

/**
 * See Header file.
 */
void NyLPC_cRomFileSet_initialize(NyLPC_TcRomFileSet_t* i_inst,const struct NyLPC_TRomFileData* i_ref_fs[],NyLPC_TUInt32 i_num_of_file)
{
    i_inst->_ref_fs=i_ref_fs;
    i_inst->_num_of_fs=i_num_of_file;
    return;
}
/**
 * See Header file.
 */
const struct NyLPC_TRomFileData* NyLPC_cRomFileSet_getFilaData(NyLPC_TcRomFileSet_t* i_inst,const NyLPC_TChar* i_name)
{
    int i;
    for(i=0;i<i_inst->_num_of_fs;i++){
        if(strcmp(i_inst->_ref_fs[i]->name,i_name)==0){
            return i_inst->_ref_fs[i];
        }
    }
    return NULL;
}
