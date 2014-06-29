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
 *
 * Parts of this file were leveraged from uIP:
 *
 * Copyright (c) 2001-2003, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "NyLPC_uip.h"
#include "NyLPC_cIPv4Payload_protected.h"





/*********************************************************************************
 * public 関数
 *********************************************************************************/




void NyLPC_cIPv4Payload_initialize(NyLPC_TcIPv4Payload_t* i_inst)
{
    return;
}

/**
 * IPパケットを格納したバッファをセットして、ペイロードのポインタを返します。
 * 失敗時はFALSE
 */
NyLPC_TBool NyLPC_cIPv4Payload_attachRxBuf(NyLPC_TcIPv4Payload_t* i_inst,const void* i_buf,NyLPC_TUInt16 i_flagment_size)
{
    i_inst->header=(const struct NyLPC_TIPv4Header*)i_buf;
    i_inst->payload.rawbuf=(const NyLPC_TUInt8*)i_buf+(i_inst->header->vhl & 0x0f)*4;
    //IPパケットのバージョンチェック
    if((i_inst->header->vhl & 0xf0)!=0x40){
        NyLPC_OnErrorGoto(Error);
    }
    //IPフレームサイズの調整
    if(NyLPC_ntohs(i_inst->header->len16)>i_flagment_size){
        NyLPC_OnErrorGoto(Error);
    }
    //フラグメントは許可しない。
    if ((NyLPC_ntohs(i_inst->header->ipoffset) & 0x3fff) != 0){
        NyLPC_OnErrorGoto(Error);
    }
    //IPv4ヘッダのチェックサムを確認
    if(!NyLPC_TIPv4Header_isCorrectIpCheckSum(i_inst->header))
    {
        NyLPC_OnErrorGoto(Error);
    }
    return NyLPC_TBool_TRUE;
Error:
    return NyLPC_TBool_FALSE;
}

/*
 * TcpIpのRxバッファをセットします。
 * @todo いらない？
 */
/*
void* NyLPC_cIPv4Payload_setTcpRxBuf(NyLPC_TcIPv4Payload_t* i_inst,void* i_buf,NyLPC_TUInt16 i_flagment_size)
{
    if(!NyLPC_cIPv4Payload_setRxBuf(i_inst,i_buf,i_flagment_size)){
        return NULL;
    }
    i_inst->header=(struct NyLPC_TIPv4Header*)i_buf;
    return i_inst->payload.rawbuf+(i_inst->payload.tcp->tcpoffset>>4)*4;
}
*/


const void* NyLPC_cIPv4Payload_detachBuf(NyLPC_TcIPv4Payload_t* i_inst)
{
    const void* r=i_inst->header;
    NyLPC_ArgAssert(r!=NULL);
    i_inst->header=NULL;
    return r;
}
/* なんだっけっこれ？
//1の補数v1にv2を加算する。
static NyLPC_TUInt16 add16c(NyLPC_TUInt16 i_v1,NyLPC_TUInt16 i_v2)
{
    NyLPC_TUInt16 t;
    t=i_v1+i_v2;
    return (t>i_v1)?t:t+1;
}
//1の補数v1から、v2を減算する。
static NyLPC_TUInt16 sub16c(NyLPC_TUInt16 i_v1,NyLPC_TUInt16 i_v2)
{
    NyLPC_TUInt16 t;
    t=i_v1-i_v2;
    return (t<i_v1)?t:t-1;
}
*/


