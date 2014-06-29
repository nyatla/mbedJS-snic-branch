#include "NyLPC_cUrlReader.h"
#include "NyLPC_utils.h"
#include <ctype.h>
NyLPC_TBool NyLPC_cUrlReader_getPath(const NyLPC_TChar* i_src,const NyLPC_TChar** path,NyLPC_TInt32* path_len)
{
    const NyLPC_TChar* p=i_src;
    for(;strchr("?#\0",*p)==NULL;p++);
    *path=i_src;
    *path_len=p-i_src;
    return NyLPC_TBool_TRUE;
}

/**
 * 指定したURLクエリキーの値を探します。
 * @return
 * クエリ値の直前のポインタです。
 * 例えばキーがabcの場合、[^\?]*\?abc=cdfの場合、=の位置を返します。[^\?]*\?abc&cdfの場合、&の位置を返します。
 * cdfの場合、[^\?]*\?abc=cdfはNULL,[^\?]*\?abc&cdfは終端'\0'の位置を返します。
 */
const NyLPC_TChar* NyLPC_cUrlReader_findKeyValue(const NyLPC_TChar* i_src,const NyLPC_TChar* i_key_name)
{
    const NyLPC_TChar* p=i_src;
    const NyLPC_TChar* k;
    NyLPC_TInt32 kn,kl;
    //?検索
    for(;*p!='\0' && *p!='?';p++);
    if(*p!='?'){
        return NULL;
    }
    p++;
    kl=strlen(i_key_name);
    //search key value
    for(;;){
        //word検索(alnum_-のみ)
        kn=NyLPC_cFormatTextReader_readWord(p,&k);
        if(strncmp(i_key_name,k,kl)==0){
            break;
        }
        p+=kn;
        for(;*p!='\0' && *p!='&';p++);
        if(*p!='&'){
            return NULL;
        }
        p++;
    }
    return p+kn;
}

/**
 * URLから指定キー[:KEY:]のURLクエリ値[:VALUE:]を取得します。
 * [:query:] := [^\?]*\?(&[:KEY:](=[:VALUE:])?&)*([:KEY:](=[:VALUE:])?)
 * [:KEY:]   := [a-zA-Z0-9_-]
 * [:VALUE:] := [^\#&]
 */
NyLPC_TBool NyLPC_cUrlReader_getStr(const NyLPC_TChar* i_src,const NyLPC_TChar* i_key_name,const NyLPC_TChar** str,NyLPC_TInt32* str_len)
{
    const NyLPC_TChar* p;
    p=NyLPC_cUrlReader_findKeyValue(i_src,i_key_name);
    if(p==NULL || *p!='='){
        *str=p;
        *str_len=0;
        return NyLPC_TBool_FALSE;
    }
    p++;
    *str=p;
    for(;strchr("&#\0",*p)==NULL;p++);
    *str_len=p-*str;
    return NyLPC_TBool_TRUE;
}

NyLPC_TBool NyLPC_cUrlReader_getUInt(const NyLPC_TChar* i_buf,const NyLPC_TChar* i_key_name,NyLPC_TUInt32* value)
{
    NyLPC_TUInt32 l,r;
    const NyLPC_TChar* p;
    p=NyLPC_cUrlReader_findKeyValue(i_buf,i_key_name);
    if(p==NULL || *p!='='){
        return NyLPC_TBool_FALSE;
    }
    p++;
    //prefixの確認
    if((*p=='0') && (NyLPC_tolower(*(p+1))=='x'))
    {
        //16進数
        p+=2;
        r=l=0;
        while(isxdigit(*p)){
            r=r*16+NyLPC_ctox(*p);
            if(l!=(r>>4)){return NyLPC_TBool_FALSE;}
            l=r;
            p++;
        }
    }else{
        r=l=0;
        while(isdigit(*p)){
            r=r*10+NyLPC_ctoi(*p);
            if(l!=(r/10)){return NyLPC_TBool_FALSE;}
            l=r;
            p++;
        }
    }
    if(strchr("&#\0",*p)==NULL){
        //An error if it is not terminator
        return NyLPC_TBool_FALSE;
    }
    *value=r;
    return NyLPC_TBool_TRUE;
}
NyLPC_TBool NyLPC_cUrlReader_getInt(const NyLPC_TChar* i_buf,const NyLPC_TChar* i_key_name,NyLPC_TInt32* value)
{
    NyLPC_TInt32 l,r;
    const NyLPC_TChar* p;
    p=NyLPC_cUrlReader_findKeyValue(i_buf,i_key_name);
    if(p==NULL || *p!='='){
        return NyLPC_TBool_FALSE;
    }
    p++;
    //prefixの確認
    if((*p=='0') && (NyLPC_tolower(*(p+1))=='x'))
    {
        //16進数
        p+=2;
        r=l=0;
        while(isxdigit(*p)){
            r=r*16+NyLPC_ctox(*p);
            if(l!=(r>>4)){return NyLPC_TBool_FALSE;}
            l=r;
            p++;
        }
    }else{
        r=l=0;
        if(*p!='-'){
            while(isdigit(*p)){
                r=r*10+NyLPC_ctoi(*p);
                if(l!=(r/10)){return NyLPC_TBool_FALSE;}
                l=r;
                p++;
            }
        }else{
            p++;
            while(isdigit(*p)){
                r=r*10+NyLPC_ctoi(*p);
                if(l!=(r/10)){return NyLPC_TBool_FALSE;}
                l=r;
                p++;
            }
            r*=-1;
        }
    }
    if(strchr("&#\0",*p)==NULL){
        //An error if it is not terminator
        return NyLPC_TBool_FALSE;
    }
    *value=(int)r;
    return NyLPC_TBool_TRUE;
}
