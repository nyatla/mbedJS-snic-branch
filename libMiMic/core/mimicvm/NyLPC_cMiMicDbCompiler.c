#include "NyLPC_mimicvm_utils_protected.h"
#include "NyLPC_cMiMicDbCompiler.h"



void NyLPC_cMiMicDbCompiler_initialize(NyLPC_TcMiMicDbCompiler_t* i_inst)
{
    i_inst->_bc_fragment_len=0;
}

/**
 * この関数は、MiMicDBのフラグメントをコンパイルします。
 * @param o_parsed_len
 * 変換した文字列の長さ。
 * @return
 * ステータスを返す。
 */
NyLPC_TcMiMicDbCompiler_RET NyLPC_cMiMicDbCompiler_compileFragment(NyLPC_TcMiMicDbCompiler_t* i_inst,const struct NyLPC_TCharArrayPtr* i_bc,NyLPC_TUInt32* o_val,NyLPC_TUInt16* o_parsed_bc)
{
    int i;
    NyLPC_TUInt8 c=i_inst->_bc_fragment_len;
    if(c==0 && i_bc->len>=8){
        //キャッシュ0でソースが十分あるときは直接変換
        if(NyLPC_mimicvm_txt2UInt(i_bc->ptr,8,o_val)){
            *o_parsed_bc=8;
            return NyLPC_TcMiMicDbCompiler_RET_OK;
        }else{
            i_inst->error_reason=NyLPC_TcMiMicDbCompiler_ERROR_FORMAT;
            return NyLPC_TcMiMicDbCompiler_RET_ERROR;
        }
    }else{
        //キャッシュが0でなければ、パディングして変換
        for(i=0;i<i_bc->len;i++){
            i_inst->_tmp[c]=i_bc->ptr[i];
            c++;
            //8個のバイトコードが溜まったら変換
            if(c==8){
                i_inst->_bc_fragment_len=0;
                if(NyLPC_mimicvm_txt2UInt(i_inst->_tmp,8,o_val)){
                    *o_parsed_bc=i+1;//見チェック
                    return NyLPC_TcMiMicDbCompiler_RET_OK;
                }else{
                    i_inst->error_reason=NyLPC_TcMiMicDbCompiler_ERROR_FORMAT;
                    return NyLPC_TcMiMicDbCompiler_RET_ERROR;
                }
            }
        }
        i_inst->_bc_fragment_len=c;
        *o_parsed_bc=i_bc->len;
        return NyLPC_TcMiMicDbCompiler_RET_CONTINUE;
    }
}
/**
 * MiMicDBフラグメントを1文字パースします。
 */
NyLPC_TcMiMicDbCompiler_RET NyLPC_cMiMicDbCompiler_compileFragment2(NyLPC_TcMiMicDbCompiler_t* i_inst,NyLPC_TChar i_bc,NyLPC_TUInt32* o_val)
{
    struct NyLPC_TCharArrayPtr bc;
    NyLPC_TUInt16 l;
    bc.ptr=&i_bc;
    bc.len=1;
    return NyLPC_cMiMicDbCompiler_compileFragment(i_inst,&bc,o_val,&l);
}

/**
 * 複数の数値を一括して変換する。テストしえてないから注意な。
 * @return
 * 0   - エラー。変換エラーが発生した。
 * 0<n - 成功。o_valに出力したデータの数
 */
NyLPC_TUInt16 NyLPC_cMiMicDbCompiler_compile(NyLPC_TcMiMicDbCompiler_t* i_inst,const struct NyLPC_TCharArrayPtr* i_bc,struct NyLPC_TUInt32ArrayPtr* o_val)
{
    struct NyLPC_TUInt32ArrayPtr wp=*o_val;
    struct NyLPC_TCharArrayPtr rp=*i_bc;
    NyLPC_TUInt16 s;
    NyLPC_Assert(i_bc->len>0);
    while(i_bc->len>0){
        //空き領域チェック
        if(wp.len==0){
            //空き領域不足
            i_inst->error_reason=NyLPC_TcMiMicDbCompiler_ERROR_OUT_BUFFER_TOO_SHORT;
            return 0;
        }
        switch(NyLPC_cMiMicDbCompiler_compileFragment(i_inst,&rp,o_val->ptr,&s)){
        case NyLPC_TcMiMicDbCompiler_RET_OK:
            //入力ポインタの移動
            if(!NyLPC_TCharArrayPtr_seek(&rp,s)){
                return 0;
            }
            //出力ポインタの移動
            if(!NyLPC_TUInt32ArrayPtr_seek(&wp,1)){
                return 0;//エラー
            }
            continue;
        case NyLPC_TcMiMicDbCompiler_RET_CONTINUE:
            //フラグメント化してたらエラー
            i_inst->error_reason=NyLPC_TcMiMicDbCompiler_ERROR_FRAGMENT_UNIT;
        default:
            return 0;
        }
    }
    //変換完了
    return o_val->len-wp.len;
}


#define TEST
#ifndef TEST
void main(void)
{
    int i=0;
    struct NyLPC_TCharArrayPtr bc;
    const char* BC="0000000100000002";
    const char* rp;
    NyLPC_TcMiMicDbCompiler_t inst;
    NyLPC_TUInt32 obuf[1024];
    NyLPC_TUInt16 pl;
    NyLPC_cMiMicDbCompiler_initialize(&inst);
    bc.ptr=(char*)BC;
    bc.len=strlen(BC);
    rp=BC;
    NyLPC_cMiMicDbCompiler_compileFragment(&inst,&bc,obuf,&pl);
/*
    for(;;){
        switch(NyLPC_cMiMicDbCompiler_compileFragment2(&inst,*bc.ptr,&obuf[i]))
        {
        case NyLPC_TcMiMicDbCompiler_RET_ERROR:
            printf("ERROR");
            return;
        case NyLPC_TcMiMicDbCompiler_RET_CONTINUE:
            NyLPC_TCharArrayPtr_seek(&bc,1);
            break;
        case NyLPC_TcMiMicDbCompiler_RET_OK:
            i++;
            NyLPC_TCharArrayPtr_seek(&bc,1);
            break;
        }
    }
*/
    return;
}
#endif
