#include "NyLPC_cHttpResponseWriter.h"
#include "NyLPC_stdlib.h"
#include <stdlib.h>
/*
typedef NyLPC_TUInt8 NyLPC_NyLPC_TcJsonWeiter_ST;
#define NyLPC_NyLPC_TcJsonWeiter_ST_STR ((NyLPC_NyLPC_TcJsonWeiter_ST)(2))
#define NyLPC_NyLPC_TcJsonWeiter_ST_ARRAY ((NyLPC_NyLPC_TcJsonWeiter_ST)(3))
#define NyLPC_NyLPC_TcJsonWeiter_ST_NODE ((NyLPC_NyLPC_TcJsonWeiter_ST)(4))
typedef struct NyLPC_TcJsonWeiter NyLPC_TcJsonWeiter_t;
struct NyLPC_TcJsonWeiter
{
    NyLPC_NyLPC_TcJsonWeiter_ST _stack[8];
    NyLPC_TUInt8 _stack_level;
    NyLPC_TBool _noerror;
};
void NyLPC_cJsonWeiter_initialize(NyLPC_TcJsonWeiter_t* i_inst)
{
    i_inst->_stack_level=0;
}

NyLPC_TBool NyLPC_cJsonWeiter_putNode(NyLPC_TcJsonWeiter_t* i_inst,const NyLPC_TChar* i_name,const NyLPC_TChar* i_value)
{
    NyLPC_cHttpHeaderWriter_writeBody(NULL,i_name);
    NyLPC_cHttpHeaderWriter_writeBody(NULL,":");
    NyLPC_cHttpHeaderWriter_writeBody(NULL,i_value);
    NyLPC_cHttpHeaderWriter_writeBody(NULL,",");
}
NyLPC_TBool NyLPC_cJsonWeiter_putStrNode(NyLPC_TcJsonWeiter_t* i_inst,const NyLPC_TChar* i_name,const NyLPC_TChar* i_value)
{
    NyLPC_cHttpHeaderWriter_writeBody(NULL,i_name);
    NyLPC_cHttpHeaderWriter_writeBody(NULL,":\"");
    NyLPC_cHttpHeaderWriter_writeBody(NULL,i_value);
    NyLPC_cHttpHeaderWriter_writeBody(NULL,"\",");
}
NyLPC_TBool NyLPC_cJsonWeiter_putIntNode(NyLPC_TcJsonWeiter_t* i_inst,const NyLPC_TChar* i_name,const NyLPC_TInt32 i_value,int i_base)
{
    NyLPC_TChar v[12];
    if(!i_inst->_noerror){
        return NyLPC_TBool_FALSE;
    }
    itoa(i_value,v,i_base);
    i_inst->_noerror=NyLPC_cJsonWeiter_putNode(i_inst,i_name,v);
    return i_inst->_noerror;
}
NyLPC_TBool NyLPC_cJsonWeiter_putBoolNode(NyLPC_TcJsonWeiter_t* i_inst,const NyLPC_TChar* i_name,NyLPC_TBool i_value)
{
    if(!i_inst->_noerror){
        return NyLPC_TBool_FALSE;
    }
    i_inst->_noerror=NyLPC_cJsonWeiter_putNode(i_inst,i_name,i_value?"true":"false");
    return i_inst->_noerror;
}


NyLPC_TBool NyLPC_cJsonWeiter_startNode(NyLPC_TcJsonWeiter_t* i_inst,const NyLPC_TChar* i_name)
{
    sprintf
    NyLPC_cHttpHeaderWriter_writeBody(NULL,i_name);
    NyLPC_cHttpHeaderWriter_writeBody(NULL,":{");
}
NyLPC_TBool NyLPC_cJsonWeiter_endNode(NyLPC_TcJsonWeiter_t* i_inst)
{
    NyLPC_cHttpHeaderWriter_writeBody(NULL,"},");
}
NyLPC_TBool NyLPC_cJsonWeiter_startArray(NyLPC_TcJsonWeiter_t* i_inst,const NyLPC_TChar* i_name)
{
    NyLPC_cHttpHeaderWriter_writeBody(NULL,i_name);
    NyLPC_cHttpHeaderWriter_writeBody(NULL,":[");
}
NyLPC_TBool NyLPC_cJsonWeiter_endArray(NyLPC_TcJsonWeiter_t* i_inst,const NyLPC_TChar* i_name)
{
    NyLPC_cHttpHeaderWriter_writeBody(NULL,"],");
}*/
