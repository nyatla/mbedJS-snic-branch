/**
 * @file
 * NyLPC_cFormatTextReader.c
 * このクラスは、書式テキスト読み出し関数を集約します。
 *  Created on: 2013/04/18
 *      Author: nyatla
 */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "NyLPC_cFormatTextReader.h"
/**
 * [a-zA-Z0-9_-]で構成されるワードを取得します。
 * This function peek a word from string.
 * @return
 * size of seeked.
 */
NyLPC_TInt32 NyLPC_cFormatTextReader_readWord(const NyLPC_TChar* buf,const NyLPC_TChar** top)
{
    const NyLPC_TChar* p=buf;
    *top=p;
    for(;*p!='\0' && (isalnum(*p)|| (strchr("_-",*p)!=NULL));p++);//skip words
    return p-(*top);
}

/**
 * 文字列からIPアドレスを取得します。
 * [:number:]\.[:number:]\.[:number:]\.[:number:]
 * [:number:]は0-255までに制限されます。
 * @param v
 * uint8[4]
 * @return
 * next pointer
 */
NyLPC_TInt32 NyLPC_cFormatTextReader_readIpAddr(const NyLPC_TChar* buf,NyLPC_TUInt8* v)
{
    NyLPC_TInt32 t;
    const NyLPC_TChar* p=buf;
    NyLPC_TInt32 i;
    for(i=0;i<4;i++){
        t=0;
        for(;isdigit(*p);p++){
            t=t*10+NyLPC_ctoi(*p);
            if(t>255){
                return 0;
            }
        }
        v[i]=t;
        if(i<3){
            if(*p!='.'){
                return 0;
            }
            p++;
        }else if(!isspace(*p) && *p!='\0'){
            return 0;
        }
    }
    return (p-buf);
}
/**
 * 文字列から10進数の数値を読み出します。
 * @return
 * 読み出した文字数
 */
NyLPC_TInt32 NyLPC_cFormatTextReader_readUInt(const NyLPC_TChar* buf,NyLPC_TUInt32* v)
{
    NyLPC_TUInt32 t;
    const NyLPC_TChar* p=buf;
    t=0;
    for(;isdigit(*p);p++){
        t=t*10+NyLPC_ctoi(*p);
    }
    *v=t;
    return (p-buf);
}

/**
 * 文字列からMACアドレスを取得します。
 * [:hex:]:[:hex:]:[:hex:]:[:hex:]
 * @param v
 * uint8[6]
 */
NyLPC_TInt32 NyLPC_cFormatTextReader_readMacAddr(const NyLPC_TChar* buf,NyLPC_TUInt8* v)
{
    NyLPC_TInt32 t,i;
    const NyLPC_TChar* p=buf;
    for(i=0;i<6;i++){
        t=0;
        for(;isxdigit(*p);p++){
            t=t*16+NyLPC_ctox(*p);
            if(t>255){
                return 0;
            }
        }
        v[i]=t;
        if(i<5){
            if(*p!=':'){
                return 0;
            }
            p++;
        }else if(!isspace(*p) && *p!='\0'){
            return 0;
        }
    }
    return (p-buf);
}
/**
 * 連続するスペースを読み飛ばします。
 * @return
 * 読み飛ばしたスペース
 */
NyLPC_TInt32 NyLPC_cFormatTextReader_seekSpace(const NyLPC_TChar* s)
{
    const NyLPC_TChar* p=s;
    for(;*p!='\0' && isspace(*p);p++);
    return p-s;
}



