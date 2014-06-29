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
#include "NyLPC_stdlib.h"

NyLPC_TUInt32 NyLPC_TUInt32_bswap(NyLPC_TUInt32 n)
{
    return(
        ((((NyLPC_TUInt32)(n))<<24)&0xff000000)|
        ((((NyLPC_TUInt32)(n))<< 8)&0x00ff0000)|
        ((((NyLPC_TUInt32)(n))>> 8)&0x0000ff00)|
        ((((NyLPC_TUInt32)(n))>>24)&0x000000ff));
}
NyLPC_TUInt16 NyLPC_TUInt16_bswap(NyLPC_TUInt16 n)
{
    return NyLPC_TUInt16_BSWAP(n);
}

static int _line_log_l;
static const char* _line_log_m;
unsigned int NyLPC_assert_counter=0;
unsigned int NyLPC_abort_counter=0;
unsigned int NyLPC_debug_counter=0;

void NyLPC_assertHook(const char* m,int l)
{
    _line_log_l=l;
    _line_log_m=m;
    NyLPC_assert_counter++;
    return;
}
void NyLPC_abortHook(const char* m,int l)
{
    _line_log_l=l;
    _line_log_m=m;
    NyLPC_abort_counter++;
}
void NyLPC_debugHook(const char* m,int l)
{
    _line_log_l=l;
    _line_log_m=m;
    NyLPC_debug_counter++;
    return;
}

NyLPC_TBool NyLPC_TCharArrayPtr_seek(struct NyLPC_TCharArrayPtr* i_struct,NyLPC_TUInt16 i_seek)
{
    if(i_struct->len<i_seek){
        return NyLPC_TBool_FALSE;
    }
    i_struct->ptr+=i_seek;
    i_struct->len-=i_seek;
    return NyLPC_TBool_TRUE;
}

NyLPC_TBool NyLPC_TUInt32ArrayPtr_seek(struct NyLPC_TUInt32ArrayPtr* i_struct,NyLPC_TUInt16 i_seek)
{
    if(i_struct->len<i_seek){
        return NyLPC_TBool_FALSE;
    }
    i_struct->ptr+=i_seek;
    i_struct->len-=i_seek;
    return NyLPC_TBool_TRUE;
}
void NyLPC_TUInt32ArrayPtr_setBuf(struct NyLPC_TUInt32ArrayPtr* i_struct,NyLPC_TUInt32* i_ptr,NyLPC_TUInt16 i_len)
{
    i_struct->ptr=i_ptr;
    i_struct->len=i_len;
}


/** ----------
 * NyLPC_TTextIdTbl
 ---------- */

/**
 * IDテーブル
 */

NyLPC_TUInt8 NyLPC_TTextIdTbl_getMatchId(const NyLPC_TChar* i_str,const struct NyLPC_TTextIdTbl i_tbl[])
{
    int i;
    for(i=0;i_tbl[i].n!=NULL;i++){
        if(strcmp(i_str,i_tbl[i].n)==0){
            break;
        }
    }
    return i_tbl[i].id;
}
NyLPC_TUInt8 NyLPC_TTextIdTbl_getMatchIdIgnoreCase(const NyLPC_TChar* i_str,const struct NyLPC_TTextIdTbl i_tbl[])
{
    int i;
    for(i=0;i_tbl[i].n!=NULL;i++){
        if(NyLPC_stricmp(i_str,i_tbl[i].n)==0){
            break;
        }
    }
    return i_tbl[i].id;
}
const NyLPC_TChar* NyLPC_TTextIdTbl_getTextById(NyLPC_TUInt8 i_id,const struct NyLPC_TTextIdTbl i_tbl[])
{
    int i;
    for(i=0;i_tbl[i].n!=NULL;i++){
        if(i_id==i_tbl[i].id){
            return i_tbl[i].n;
        }
    }
    return NULL;
}




/** ----------
 * Standard functions
 ---------- */



NyLPC_TInt8 NyLPC_itoa(int i_n,char* o_out,NyLPC_TInt8 i_base)
{
    NyLPC_TInt8 i,v;
    int sign;
     if ((sign = i_n) < 0){
         i_n = -i_n;
     }
     i = 0;
     do{
         v=(NyLPC_TInt8)(i_n % i_base);
         o_out[i++] = (v<10)?(v+'0'):(v+'a'-10);
     }while ((i_n /= i_base) > 0);
     if (sign < 0){
         o_out[i++] = '-';
     }
     o_out[i] = '\0';
     NyLPC_reverse(o_out);
     return i;
}
NyLPC_TInt8 NyLPC_uitoa(unsigned int i_n,char* o_out,NyLPC_TInt8 i_base)
{
    NyLPC_TInt8 i = 0;
    NyLPC_TInt8 v;
     do{
         v=(NyLPC_TInt8)(i_n % i_base);
         o_out[i++] = (v<10)?(v+'0'):(v+'a'-10);
     }while ((i_n /= i_base) > 0);
     o_out[i] = '\0';
     NyLPC_reverse(o_out);
     return i;
}

/**
 * 桁数の指定できるuitoaです。
 */
NyLPC_TInt8 NyLPC_uitoa2(unsigned int i_n,char* o_out,NyLPC_TInt8 i_base,NyLPC_TInt8 i_digit)
{
     NyLPC_TInt8 i = 0;
     NyLPC_TInt8 v;
     do{
         v=(NyLPC_TInt8)(i_n % i_base);
         o_out[i++] = (v<10)?(v+'0'):(v+'a'-10);
     }while ((i_n /= i_base) > 0);
     while(i<i_digit){
         o_out[i++] = '0';
     }
     o_out[i] = '\0';
     NyLPC_reverse(o_out);
     return i;
}

void NyLPC_reverse(char* s)
{
    char *j;
    char c;
    j = s + strlen(s) - 1;
    while(s < j){
        c = *s;
        *s++ = *j;
        *j-- = c;
    }
}


int NyLPC_stricmp(const char *i_s1, const char *i_s2)
{
    const char* s1 =i_s1;
    const char* s2 =i_s2;
    for (;*s1 != '\0';s1++, s2++)
    {
        if(*s1!=*s2){
            if(NyLPC_tolower(*s1) != NyLPC_tolower(*s2)){
              break;
            }
        }
    }
    return (int)((unsigned char)*s1) - (int)((unsigned char)(*s2));
}

int NyLPC_strnicmp(const char *i_s1, const char *i_s2,int n)
{
    char c;
    int n2=n;
    for(;n2>0;n2--,i_s1++,i_s2++)
    {
        if(*i_s1!=*i_s2){
            c=(int)((unsigned char)NyLPC_tolower(*i_s1)) - (int)((unsigned char)NyLPC_tolower(*i_s2));
            if(c!=0){
                return c;
            }
        }
    }
    return 0;
}


int NyLPC_ctoi(char i)
{
    if('0'<=i && i<='9') return (i-'0');
    return 0;
}
int NyLPC_ctox(char i)
{
    if('0'<=i && i<='9') return (i-'0');
    if('a'<=i && i<='f') return (i-'a'+10);
    if('A'<=i && i<='F') return (i-'A'+10);
    return 0;
}





