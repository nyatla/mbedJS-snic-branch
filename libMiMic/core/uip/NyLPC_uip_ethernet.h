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
#ifndef NyLPC_uip_ethernet_h
#define NyLPC_uip_ethernet_h
#include "../include/NyLPC_config.h"
#include "../include/NyLPC_stdlib.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifndef PACK_STRUCT_END
    #define PACK_STRUCT_END __attribute((packed))
#endif

/**********************************************************************
 *
 * struct NyLPC_TEthAddr
 *
 **********************************************************************/

/**
 * この構造体は、48bitのイーサネットアドレスを格納します。
 */
struct NyLPC_TEthAddr
{
  NyLPC_TUInt8 addr[6];
}PACK_STRUCT_END;


/**
 * 構造体にEthernetアドレスをセットします。
 * 次のように使います。
 \code
 struct NyLPC_TEthAddr en=NyLPC_TEthAddr_pack(1,2,3,4,5,6);
 \endcode
 */
#define NyLPC_TEthAddr_pack(a1,a2,a3,a4,a5,a6) {{(a1),(a2),(a3),(a4),(a5),(a6)}}
/**
 * 変数にEthernetアドレスをセットします。
 * 次のように使います。
 \code
 struct NyLPC_TEthAddr en;
 NyLPC_TEthAddr_set(&en,1,2,3,4,5,6);
 \endcode
 */
#define NyLPC_TEthAddr_set(v,a1,a2,a3,a4,a5,a6) {(v)->addr[0]=(a1);(v)->addr[1]=(a2);(v)->addr[2]=(a3);(v)->addr[3]=(a4);(v)->addr[4]=(a5);(v)->addr[5]=(a6);}

extern const struct NyLPC_TEthAddr NyLPC_TEthAddr_BROADCAST;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

