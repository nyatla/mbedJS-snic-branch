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
#include <ctype.h>
#include <stdlib.h>
#include "NyLPC_cMiMicVM_protected.h"

static NyLPC_TUInt8 process_instruction(NyLPC_TcMiMicVM_t* i_inst,const union NyLPC_TcMiMicVM_TInstruction* ist,NyLPC_TUInt32* o_code);



void NyLPC_cMiMicVM_initialize(NyLPC_TcMiMicVM_t* i_inst,struct NyLPC_TcMiMicVM_TEvent* i_handler)
{
    NyLPC_Assert(i_inst!=NULL);
    NyLPC_Assert(i_handler!=NULL);
    NyLPC_Assert(i_handler->get_stream!=NULL);
    NyLPC_Assert(i_handler->put_stream!=NULL);
    NyLPC_Assert(i_handler->sleep!=NULL);
    i_inst->_event_handler=i_handler;
    return;
}


/**
 * 固定長命令+固定長データを実行します。
 * 関数の終了条件は、1.EXIT命令に到達する。2.インストラクションの終端に到達する。3.エラーが発生する。
 * 
 */
NyLPC_TUInt32 NyLPC_cMiMicVM_run(NyLPC_TcMiMicVM_t* i_inst,const NyLPC_TUInt32* i_instruction,const NyLPC_TUInt16 i_size_of_instruction)
{
    //データ部をgetstreamと連動させること。
    NyLPC_TUInt32 retcode=NyLPC_cMiMicVM_RESULT_OK;
    NyLPC_TUInt16 pc=0;
    NyLPC_TUInt8 proc_in_byte;
    if(i_size_of_instruction>0){
        proc_in_byte=process_instruction(i_inst,(const union NyLPC_TcMiMicVM_TInstruction*)(i_instruction+pc),&retcode);
        pc+=proc_in_byte;
        //プログラムの終端に到達するか、0バイト処理の場合にブレーク。
        while(proc_in_byte>0 && pc<i_size_of_instruction){
            proc_in_byte=process_instruction(i_inst,(const union NyLPC_TcMiMicVM_TInstruction*)(i_instruction+pc),&retcode);
            pc+=proc_in_byte;
        }
    }
    return retcode;
}
/**
 * 出力ストリームへ32ビット値を書き出す。
 */
NyLPC_TBool NyLPC_cMiMicVM_sput(NyLPC_TcMiMicVM_t* i_inst,NyLPC_TUInt32 i_val)
{
    if(!i_inst->_event_handler->put_stream(i_inst->_event_handler,i_val)){
        return NyLPC_TBool_FALSE;
    }
    return NyLPC_TBool_TRUE;
}
/**
 * 入力ストリームから32ビット値を読み出す。
 */
NyLPC_TBool NyLPC_cMiMicVM_sget(NyLPC_TcMiMicVM_t* i_inst,NyLPC_TUInt32* o_val)
{
    if(!i_inst->_event_handler->get_stream(i_inst->_event_handler,o_val)){
        return NyLPC_TBool_FALSE;
    }
    return NyLPC_TBool_TRUE;
}



/**
 * インストラクションを1個処理し、処理したバイト数を返す。
 * インストラクションの境界値はチェックされないので、コンパイル時にチェックしておくこと。
 * @return
 * 処理したインストラクションのワード数(UInt32単位)。
 * 終了した場合は0を返す。0を返したときは、ret_codeにMiMicVMの終了コードを返す。
 */
static NyLPC_TUInt8 process_instruction(NyLPC_TcMiMicVM_t* i_inst,const union NyLPC_TcMiMicVM_TInstruction* ist,NyLPC_TUInt32* o_code)
{
    NyLPC_TUInt32 tret;
    switch(ist->op.opc){
    case NyLPC_TcMiMicVM_OP_TYPE_AND:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_WM:
            i_inst->wm[ist->wmwm_32.wm1]&=i_inst->wm[ist->wmwm_32.wm2];
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_H32:
            i_inst->wm[ist->wmh32_64.wm]&=ist->wmh32_64.h32;
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_OR:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_WM:
            i_inst->wm[ist->wmwm_32.wm1]|=i_inst->wm[ist->wmwm_32.wm2];
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_H32:
            i_inst->wm[ist->wmh32_64.wm]|=ist->wmh32_64.h32;
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_XOR:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_WM:
            i_inst->wm[ist->wmwm_32.wm1]^=i_inst->wm[ist->wmwm_32.wm2];
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_H32:
            i_inst->wm[ist->wmh32_64.wm]^=ist->wmh32_64.h32;
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_NOT:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM:
            i_inst->wm[ist->wm_32.wm]=~i_inst->wm[ist->wm_32.wm];
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_SHL:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_H08:
            i_inst->wm[ist->wmh08_32.wm]=i_inst->wm[ist->wmh08_32.wm]<<ist->wmh08_32.h8;
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_WM:
            i_inst->wm[ist->wmwm_32.wm1]=i_inst->wm[ist->wmwm_32.wm1]<<i_inst->wm[ist->wmwm_32.wm2];
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_SHR:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_H08:
            i_inst->wm[ist->wmh08_32.wm]=i_inst->wm[ist->wmh08_32.wm]>>ist->wmh08_32.h8;
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_WM:
            i_inst->wm[ist->wmwm_32.wm1]=i_inst->wm[ist->wmwm_32.wm1]>>i_inst->wm[ist->wmwm_32.wm2];
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_ADD:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_WM:
            i_inst->wm[ist->wmwm_32.wm1]+=i_inst->wm[ist->wmwm_32.wm2];
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_H32:
            i_inst->wm[ist->wmh32_64.wm]+=ist->wmh32_64.h32;
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_SUB:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_WM:
            i_inst->wm[ist->wmwm_32.wm1]-=i_inst->wm[ist->wmwm_32.wm2];
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_H32:
            i_inst->wm[ist->wmh32_64.wm]-=ist->wmh32_64.h32;
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_MUL:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_WM:
            i_inst->wm[ist->wmwm_32.wm1]*=i_inst->wm[ist->wmwm_32.wm2];
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_H32:
            i_inst->wm[ist->wmh32_64.wm]*=ist->wmh32_64.h32;
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_MGET:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_H32:
            i_inst->wm[ist->wmh32_64.wm]=*((NyLPC_TUInt32*)(ist->wmh32_64.h32));
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_WM:
            i_inst->wm[ist->wmwm_32.wm1]=*((NyLPC_TUInt32*)(i_inst->wm[ist->wmwm_32.wm2]));
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_MPUT:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_H32:
            *((NyLPC_TUInt32*)(ist->wmh32_64.h32))=i_inst->wm[ist->wmh32_64.wm];
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_WM:
            *((NyLPC_TUInt32*)(i_inst->wm[ist->wmwm_32.wm2]))=i_inst->wm[ist->wmwm_32.wm1];
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_SGET:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM:
            if(!i_inst->_event_handler->get_stream(i_inst->_event_handler,&(i_inst->wm[ist->wm_32.wm]))){
                NyLPC_OnErrorGoto(ERROR);
            }
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_SPUT:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM:
            if(!i_inst->_event_handler->put_stream(i_inst->_event_handler,i_inst->wm[ist->wm_32.wm])){
                NyLPC_OnErrorGoto(ERROR);
            }
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_H32:
            if(!i_inst->_event_handler->put_stream(i_inst->_event_handler,ist->h32_64.h32)){
                NyLPC_OnErrorGoto(ERROR);
            }
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_LD:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_WM:
            i_inst->wm[ist->wmwm_32.wm1]=i_inst->wm[ist->wmwm_32.wm2];
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_WM_H32:
            i_inst->wm[ist->wmh32_64.wm]=ist->wmh32_64.h32;
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_NOP:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_NONE:
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_H08:
            i_inst->_event_handler->sleep(i_inst->_event_handler,ist->h8_32.h8);
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    //native call
    case NyLPC_TcMiMicVM_OP_TYPE_CALL:
        switch(ist->op.oprtype){
        case NyLPC_TcMiMicVM_OPR_TYPE_WM:
            tret=i_inst->_event_handler->native_call(i_inst->_event_handler,i_inst->wm[ist->wm_32.wm],i_inst);
            if(!NyLPC_cMiMicVM_RESULT_isOK(tret)){
                *o_code=tret;
                NyLPC_OnErrorGoto(ERROR_INHERIT);//エラー継承
            }
            break;
        case NyLPC_TcMiMicVM_OPR_TYPE_H32:
            tret=i_inst->_event_handler->native_call(i_inst->_event_handler,ist->h32_64.h32,i_inst);
            if(!NyLPC_cMiMicVM_RESULT_isOK(tret)){
                *o_code=tret;
                NyLPC_OnErrorGoto(ERROR_INHERIT);//エラー継承
            }
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TcMiMicVM_OP_TYPE_EXIT:
        *o_code=NyLPC_cMiMicVM_RESULT_OK;//OKに上書き
        return 0;
    default:
        NyLPC_OnErrorGoto(ERROR);
    }
    //実行したコードのワード長を返す。
    switch(ist->op.oprtype){
    case NyLPC_TcMiMicVM_OPR_TYPE_NONE:
    case NyLPC_TcMiMicVM_OPR_TYPE_WM_WM:
    case NyLPC_TcMiMicVM_OPR_TYPE_WM_H08:
    case NyLPC_TcMiMicVM_OPR_TYPE_WM:
    case NyLPC_TcMiMicVM_OPR_TYPE_H08:
    case NyLPC_TcMiMicVM_OPR_TYPE_H16:
        *o_code=NyLPC_cMiMicVM_RESULT_OK;//OKに上書き
        return 1;
    case NyLPC_TcMiMicVM_OPR_TYPE_WM_H16:
    case NyLPC_TcMiMicVM_OPR_TYPE_WM_H32:
    case NyLPC_TcMiMicVM_OPR_TYPE_H32:
        *o_code=NyLPC_cMiMicVM_RESULT_OK;//OKに上書き
        return 2;
    }
ERROR:
    *o_code=NyLPC_cMiMicVM_RESULT_RUNTIME_NG;//ランタイムNG
ERROR_INHERIT:
    return 0;
}

#define TEST
#ifndef TEST

#include "NyLPC_cMiMicTxtCompiler.h"
void main(void)
{
    struct NyLPC_TcMiMicVM_TEvent eh;
    NyLPC_TUInt32 ap;
    NyLPC_TcMiMicVM_t vm;
    struct NyLPC_TCharArrayPtr bc;
    NyLPC_TcMiMicTxtCompiler_t inst;
    struct NyLPC_TUInt32ArrayPtr bin;
    char BC[1024];
    int ist_len;

    NyLPC_TUInt16 l,bl;
    NyLPC_TUInt32 obuf[1024];
    NyLPC_cMiMicBcCompiler_initialize(&inst);
    sprintf(BC,"AA0102AB0100000001AE0203AF0200000003AI0304AJ0300000004AM07BA0505BE0607CA0304CB0300000005CE0304CF0300000005CI0304CJ0300000005ZA.E",&ap,&ap);
    bc.ptr=(char* )BC;
    bc.len=strlen(BC);
    bin.ptr=obuf;
    bin.len=100;
    ist_len=0;

    for(;;){

        switch(NyLPC_cMiMicBcCompiler_compileFragment(&inst,&bc,&bin,&bl,&l))
        {
        case NyLPC_TcMiMicTxtCompiler_RET_OK:
            //命令確定。
            NyLPC_TCharArrayPtr_seek(&bc,l);
            ist_len+=bl;
            break;
        case NyLPC_TcMiMicTxtCompiler_RET_OK_END:
            //命令終端
            NyLPC_cMiMicVM_initialize(&vm,&eh);
            if(!NyLPC_cMiMicVM_run(&vm,obuf,ist_len)){
                printf("ｴﾝﾀﾞｧ");
            }
            printf("OK");
            break;
        case NyLPC_TcMiMicTxtCompiler_RET_CONTINUE:
            //蓄積中。
            NyLPC_TCharArrayPtr_seek(&bc,l);
            break;
        case NyLPC_TcMiMicTxtCompiler_RET_NG:
            printf("エラー");
            return;
        default:
            break;
        }
    }
}
#endif
