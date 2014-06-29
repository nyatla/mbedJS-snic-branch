#include "NyLPC_mimicvm_utils_protected.h"
/**
 * i_txtを16進数に変換する。
 * @param i_txt
 * テキスト形式の16進数。[a-f0-9]+であること
 * @param i_num
 * i_txtの長さ。2,4,8のいずれかであること。
 * @param out
 * 出力値を格納するメモリアドレス。
 * i_numにより、必要なサイズが変化する。
 *
 */
NyLPC_TBool NyLPC_mimicvm_txt2UInt(const NyLPC_TChar* i_txt,NyLPC_TUInt8 i_num,void* out)
{
    NyLPC_TUInt32 ret=0;
    NyLPC_TChar c;
    int i;

    for(i=0;i<i_num;i++){
        c=(*(i_txt+i));
        if('f'>=c && c>='a'){
            c=c-(NyLPC_TUInt8)'a'+10;
        }else if('9'>=c && c>='0'){
            c-=(NyLPC_TUInt8)'0';
        }else{
            return NyLPC_TBool_FALSE;
        }
        ret=(ret<<4)|c;
    }
    //2,4,8だけ。
    switch(i_num){
    case 2:
        *((NyLPC_TUInt8*)out)=(NyLPC_TUInt8)ret;
        break;
    case 4:
        *((NyLPC_TUInt16*)out)=(NyLPC_TUInt16)ret;
        break;
    case 8:
        *((NyLPC_TUInt32*)out)=(NyLPC_TUInt32)ret;
        break;
    default:
        return NyLPC_TBool_FALSE;
    }
    return NyLPC_TBool_TRUE;
}

