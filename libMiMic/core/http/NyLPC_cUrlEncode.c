#include "NyLPC_cUrlEncode.h"
#include <stdarg.h>
#include <ctype.h>


NyLPC_TBool NyLPC_cUrlEncode_initialize(NyLPC_TcUrlEncode_t* i_inst)
{
    i_inst->_len=0;
    return NyLPC_TBool_TRUE;
}


NyLPC_TcUrlEncode_ST NyLPC_cUrlEncode_decode(NyLPC_TcUrlEncode_t* i_inst,NyLPC_TChar c,NyLPC_TChar* out)
{
    int t;
    if(c=='%'){
        if(i_inst->_len!=0){
            NyLPC_OnErrorGoto(Error);
        }
        i_inst->_len=1;
        return NyLPC_TcUrlEncode_ST_NEXT;
    }else{
        switch(i_inst->_len){
        case 0:
            *out=c;
            return NyLPC_TcUrlEncode_ST_DONE;
        case 1:
            if(!isxdigit((int)c)){
                NyLPC_OnErrorGoto(Error);
            }
            t=NyLPC_ctox(c);
            i_inst->v=(NyLPC_TChar)t;//16進文字→HEX
            i_inst->_len++;
            return NyLPC_TcUrlEncode_ST_NEXT;
        case 2:
            if(!isxdigit((int)c)){
                NyLPC_OnErrorGoto(Error);
            }
            t=NyLPC_ctox(c);
            *out=(NyLPC_TChar)((i_inst->v<<4) | t);
            i_inst->v=0;
            i_inst->_len=0;
            return NyLPC_TcUrlEncode_ST_DONE;
        default:
            NyLPC_OnErrorGoto(Error);
        }
    }
Error:
    i_inst->v=0;
    i_inst->_len=0;
    return NyLPC_TcUrlEncode_ST_ERROR;
}
