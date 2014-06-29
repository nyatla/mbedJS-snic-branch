#include "NyLPC_cHttpBodyParser.h"

#define HTTP_TIMEOUT NyLPC_TiHttpPtrStream_DEFAULT_HTTP_TIMEOUT

static NyLPC_TBool bodyHandler(NyLPC_TcHttpBasicBodyParser_t* i_inst,NyLPC_TChar i_c)
{
    NyLPC_TcHttpBodyParser_t* inst=(NyLPC_TcHttpBodyParser_t*)i_inst;
    *(inst->ref_buf+inst->len)=i_c;
    inst->len++;
    return inst->buf_size>inst->len;
}

static struct NyLPC_TcHttpBasicBodyParser_Handler _bh={bodyHandler};

void NyLPC_cHttpBodyParser_initialize(NyLPC_TcHttpBodyParser_t* i_inst)
{
    NyLPC_cHttpBasicBodyParser_initialize(&i_inst->_super,&_bh);
}
void NyLPC_cHttpBodyParser_finalize(NyLPC_TcHttpBodyParser_t* i_inst)
{
    NyLPC_cHttpBasicBodyParser_finalize(&i_inst->_super);
}


/**
 * @param i_out
 * 読み出したデータサイズ。戻り値trueの場合のみ有効。0の場合終端。
 * @return
 * エラーの発生状況
 */
NyLPC_TBool NyLPC_cHttpBodyParser_parseStream(NyLPC_TcHttpBodyParser_t* i_inst,NyLPC_TiHttpPtrStream_t* i_stream,NyLPC_TChar* i_buf,NyLPC_TInt16 i_buf_size,NyLPC_TInt16* i_out)
{
    NyLPC_TcHttpBodyParser_t* inst=(NyLPC_TcHttpBodyParser_t*)i_inst;
    const char* rp_base;
    NyLPC_TInt32 rsize;
    inst->len=0;
    inst->buf_size=i_buf_size;
    inst->ref_buf=i_buf;
    if(i_inst->_super._status==NyLPC_TcHttpBasicBodyParser_ST_EOB){
        *i_out=0;
        return NyLPC_TBool_TRUE;
    }
    for(;;){
        //タイムアウト付でストリームから読み出し。
        rsize=NyLPC_iHttpPtrStream_pread(i_stream,(const void**)(&rp_base),HTTP_TIMEOUT);
        if(rsize<=0){
            //Read失敗
            return NyLPC_TBool_FALSE;
        }
        rsize=NyLPC_cHttpBasicBodyParser_parseChar(&i_inst->_super,rp_base,rsize);
        if(i_inst->_super._status==NyLPC_TcHttpBasicBodyParser_ST_ERROR){
            //パース失敗
            return NyLPC_TBool_FALSE;
        }
        NyLPC_iHttpPtrStream_rseek(i_stream,(NyLPC_TUInt16)rsize);
        *i_out=i_inst->len;
        return NyLPC_TBool_TRUE;
    }
}
