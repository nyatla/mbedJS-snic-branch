#include <stdlib.h>
#include "NyLPC_cUuid.h"

void NyLPC_cUuid_initialize(NyLPC_TcUuid_t* i_inst)
{
}
#define NyLPC_cUuid_finalize(i_inst)

void NyLPC_cUuid_setTimeBase(NyLPC_TcUuid_t* i_inst,NyLPC_TUInt32 i_time_l,NyLPC_TUInt32 i_time_h,NyLPC_TUInt16 i_seq,struct NyLPC_TEthAddr* eth_mac)
{
    i_inst->f1=i_time_l;
    i_inst->f2=(NyLPC_TUInt16)(i_time_h & 0x0000ffff);
    i_inst->f3=(NyLPC_TUInt16)(((i_time_h & 0x0fff0000)>>16)|0x1000);
    i_inst->f4=(NyLPC_TUInt8)((0x3f&(i_seq>>8))|0x80);
    i_inst->f5=(NyLPC_TUInt8)(0xff&(i_seq>>0));
    memcpy(i_inst->f6,eth_mac->addr,8);
}


void NyLPC_cUuid_toString(NyLPC_TcUuid_t* i_inst,NyLPC_TChar* i_buf)
{
    NyLPC_TChar* p=i_buf;
    NyLPC_TInt16 i;
    NyLPC_uitoa2(i_inst->f1,p,16,8);p+=8;
    *p='-';p++;
    NyLPC_uitoa2(i_inst->f2,p,16,4);p+=4;
    *p='-';p++;
    NyLPC_uitoa2(i_inst->f3,p,16,4);p+=4;
    *p='-';p++;
    NyLPC_uitoa2(i_inst->f4,p,16,2);p+=2;
    NyLPC_uitoa2(i_inst->f5,p,16,2);p+=2;
    *p='-';p++;
    for(i=0;i<6;i++){
        NyLPC_uitoa2(i_inst->f6[i],p,16,2);p+=2;
    }
    *p='\0';
}
