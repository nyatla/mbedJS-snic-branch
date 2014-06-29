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
#include "NyLPC_cMiMicTxtCompiler.h"
#include "NyLPC_cMiMicVM_protected.h"
#include "NyLPC_mimicvm_utils_protected.h"


struct TInstructionDef
{
    /** MiMicBCのオペコード */
    const char* bcopcode;
    /** インストラクションID */
    NyLPC_TcMiMicVM_OP_TYPE opid;
    /** オペコードのタイプ  */
    NyLPC_TcMiMicVM_OPR_TYPE optype;
};
static NyLPC_TBool bc2opc(const NyLPC_TChar* i_char,NyLPC_TcMiMicVM_OP_TYPE* o_opc,NyLPC_TcMiMicVM_OPR_TYPE* o_opt);
static NyLPC_TBool bc2ctrlc(const NyLPC_TChar* i_char,NyLPC_TcMiMicVM_OP_TYPE* o_opc,NyLPC_TcMiMicVM_OPR_TYPE* o_opt);
static NyLPC_TBool txt2UInt(const NyLPC_TChar* i_txt,NyLPC_TUInt8 i_num,void* out);
static NyLPC_TBool txt2WMId(const NyLPC_TChar* i_txt,NyLPC_TUInt8* out);
static void NyLPC_TcMiMicVM_OPR_TYPE_getOpInfo(NyLPC_TcMiMicVM_OPR_TYPE i_type,NyLPC_TUInt8* oprbc_len,NyLPC_TUInt8* o_istlen);


void NyLPC_cMiMicTxtCompiler_initialize(NyLPC_TcMiMicTxtCompiler_t* i_inst)
{
    i_inst->out_buf=NULL;
    i_inst->st=NyLPC_TcMiMicTxtCompiler_ST_OPC;
    i_inst->tmp_len=0;
}



#define NyLPC_TcMiMicTxtCompiler_ST_OPC  0x01   //OPをパース中
#define NyLPC_TcMiMicTxtCompiler_ST_OPR  0x02   //オペランドを解析中
#define NyLPC_TcMiMicTxtCompiler_ST_CTR  0x03   //制御パラメータを解析中
#define NyLPC_TcMiMicTxtCompiler_ST_OK   0x04   //パースを完了した。(.EMD検出)
#define NyLPC_TcMiMicTxtCompiler_ST_NG   0xff   //パースでエラーが発生ｳｪｰｲ



/**
 * バイトコードフラグメントから1命令をコンパイルして、i_binへ出力します。
 * バイトコードは、バイナリ値に変換されます。
 * バイナリ値は、MiMicVMの実行形式です。
 * @param i_bin
 * コンパイルしたBCを格納する配列を指定します。
 * 関数が成功した場合、配列のポインターは追加したBCの数だけ進行します。
 * @return
 * 実行結果を返します。NyLPC_TcMiMicTxtCompiler_RET_OKの場合に、i_binへo_bin_lenの長さのインストラクションを出力します。
 *
 */
NyLPC_TcMiMicTxtCompiler_RET NyLPC_cMiMicTxtCompiler_compileFragment(NyLPC_TcMiMicTxtCompiler_t* i_inst,const struct NyLPC_TCharArrayPtr* i_bc,struct NyLPC_TUInt32ArrayPtr* i_bin,NyLPC_TUInt16* o_bin_len,NyLPC_TUInt16* o_parsed_bc)
{
    union NyLPC_TcMiMicVM_TInstruction* wptr;
    int i;
    for(i=0;i<i_bc->len;i++)
    {
        switch(i_inst->st){
        case NyLPC_TcMiMicTxtCompiler_ST_OPC:
            if(i_inst->tmp_len>2){
                NyLPC_OnErrorGoto(ERROR);
            }
            i_inst->tmp[i_inst->tmp_len]=*(i_bc->ptr+i);
            i_inst->tmp_len++;
            if(i_inst->tmp_len==2){
                //[A-Z]{2}がそろった。命令コードか制御命令か判定
                if(bc2opc(i_inst->tmp,&(i_inst->_current_opc),&(i_inst->_current_oprtype))){
                    //命令コードならインストラクションの情報をもらってくる。
                    NyLPC_TcMiMicVM_OPR_TYPE_getOpInfo(i_inst->_current_oprtype,&i_inst->_oprbc_len,&i_inst->_inst_len);
                    //オペランドが無ければ、命令確定。
                    if(i_inst->_oprbc_len<=0){
                        //書込みポインタ保管
                        wptr=(union NyLPC_TcMiMicVM_TInstruction*)i_bin->ptr;
                        //バッファのシーク
                        if(!NyLPC_TUInt32ArrayPtr_seek(i_bin,1)){
                            NyLPC_OnErrorGoto(ERROR);
                        }
                        //インストラクションの出力と処理したBC長の計算
                        wptr->op.opc=i_inst->_current_opc;
                        wptr->op.oprtype=i_inst->_current_oprtype;
                        *o_parsed_bc=i+1;       //パースしたBCの長さ
                        *o_bin_len=i_inst->_inst_len;
                        //解析バッファの長さをリセットして、次のBCブロックのパース準備
                        i_inst->tmp_len=0;
                        return NyLPC_TcMiMicTxtCompiler_RET_OK;//命令確定。
                    }
                    //オペランドがあるなら、パース対象を変更。
                    i_inst->tmp_len=0;
                    i_inst->st=NyLPC_TcMiMicTxtCompiler_ST_OPR;
                }else if(bc2ctrlc(i_inst->tmp,&(i_inst->_current_opc),&(i_inst->_current_oprtype))){
                    //命令コードならインストラクションの情報をもらってくる。
                    NyLPC_TcMiMicVM_OPR_TYPE_getOpInfo(i_inst->_current_oprtype,&i_inst->_oprbc_len,&i_inst->_inst_len);
                    //制御コードの解析
                    if(i_inst->_oprbc_len>0){
                        //パラメータのある制御命令未定義だからエラー。
                        NyLPC_OnErrorGoto(ERROR);
                    }
                    //END制御命令?
                    if(i_inst->_current_opc==NyLPC_TcMiMicVM_CP_TYPE_END){
                        *o_bin_len=0;
                        *o_parsed_bc=i+1;       //パースしたBCの長さ
                        i_inst->tmp_len=0;
                        return NyLPC_TcMiMicTxtCompiler_RET_OK_END;//命令確定。(パース完了)
                    }
                    //END制御命令以外ならエラー
                    NyLPC_OnErrorGoto(ERROR);
                }else{
                    //不明な命令
                    NyLPC_OnErrorGoto(ERROR);
                }
            }
            break;
        case NyLPC_TcMiMicTxtCompiler_ST_CTR:
            NyLPC_OnErrorGoto(ERROR);
        case NyLPC_TcMiMicTxtCompiler_ST_OPR:
            //オペランド蓄積.
            i_inst->tmp[i_inst->tmp_len]=*(i_bc->ptr+i);
            i_inst->tmp_len++;
            //オペランド長さになるまで値を追記
            if(i_inst->_oprbc_len==i_inst->tmp_len){
                //書込みポインタ保管
                wptr=(union NyLPC_TcMiMicVM_TInstruction*)i_bin->ptr;
                //シーク
                if(!NyLPC_TUInt32ArrayPtr_seek(i_bin,i_inst->_inst_len)){
                    NyLPC_OnErrorGoto(ERROR);
                }
                wptr->op.opc=i_inst->_current_opc;
                wptr->op.oprtype=i_inst->_current_oprtype;
                //オペランドの変換処理
                switch(i_inst->_current_oprtype){
                case NyLPC_TcMiMicVM_OPR_TYPE_WM_WM:
                    if(!(   txt2WMId(i_inst->tmp,&(wptr->wmwm_32.wm1)) &&
                            txt2WMId(i_inst->tmp+2,&(wptr->wmwm_32.wm2))))
                    {
                        NyLPC_OnErrorGoto(ERROR);
                    }
                    break;
                case NyLPC_TcMiMicVM_OPR_TYPE_WM_H08:
                    if(!(   txt2WMId(i_inst->tmp,&(wptr->wmh08_32.wm)) &&
                            txt2UInt(i_inst->tmp+2,2,&(wptr->wmh08_32.h8))))
                    {
                        NyLPC_OnErrorGoto(ERROR);
                    }
                    break;
                case NyLPC_TcMiMicVM_OPR_TYPE_WM_H32:
                    if(!(   txt2WMId(i_inst->tmp,&(wptr->wmh32_64.wm)) &&
                            txt2UInt(i_inst->tmp+2,8,&(wptr->wmh32_64.h32)))){
                        NyLPC_OnErrorGoto(ERROR);
                    }
                    break;
                case NyLPC_TcMiMicVM_OPR_TYPE_WM:
                    if(!txt2WMId(i_inst->tmp,&(wptr->wm_32.wm))){
                        NyLPC_OnErrorGoto(ERROR);
                    }
                    break;
                case NyLPC_TcMiMicVM_OPR_TYPE_H32:
                    if(!txt2UInt(i_inst->tmp,8,&(wptr->h32_64.h32))){
                        NyLPC_OnErrorGoto(ERROR);
                    }
                    break;
                case NyLPC_TcMiMicVM_OPR_TYPE_H08:
                    if(!txt2UInt(i_inst->tmp,2,&(wptr->h8_32.h8)))
                    {
                        NyLPC_OnErrorGoto(ERROR);
                    }
                    break;
                default:
                    NyLPC_OnErrorGoto(ERROR);
                }
                //OPR解析成功。パースしたブロックサイズの計算。
                *o_parsed_bc=i+1;       //パースしたBCの長さ
                *o_bin_len=i_inst->_inst_len;
                i_inst->tmp_len=0;
                i_inst->st=NyLPC_TcMiMicTxtCompiler_ST_OPC;
                return NyLPC_TcMiMicTxtCompiler_RET_OK;
            }
            break;
        default:
            NyLPC_OnErrorGoto(ERROR);
        }
    }
    *o_bin_len=0;
    *o_parsed_bc=i_bc->len;
    return NyLPC_TcMiMicTxtCompiler_RET_CONTINUE;
ERROR:
    i_inst->st=NyLPC_TcMiMicTxtCompiler_ST_NG;
    return NyLPC_TcMiMicTxtCompiler_RET_NG;
}



/**
 * フラグメント入力のMiMicBCをコンパイルします。
 * この関数は、1文字のフラグメントMiMicBCをコンパイラに入力します。
 * @param i_bin
 * コンパイルしたBCを格納する配列を指定します。
 * 関数が成功した場合、配列のポインターは追加したBCの数だけ進行します。
 * @return
 * 実行結果を返します。TRUEのときは、ステータスをチェックしてください。
 *
 */
NyLPC_TcMiMicTxtCompiler_RET NyLPC_cMiMicTxtCompiler_compileFragment2(NyLPC_TcMiMicTxtCompiler_t* i_inst,NyLPC_TChar i_bc,struct NyLPC_TUInt32ArrayPtr* i_bin,NyLPC_TUInt16* o_bin_len)
{
    struct NyLPC_TCharArrayPtr bc;
    NyLPC_TUInt16 bc_len;
    bc.ptr=&i_bc;
    bc.len=1;
    return NyLPC_cMiMicTxtCompiler_compileFragment(i_inst,&bc,i_bin,o_bin_len,&bc_len);
}



/**
 * 2バイトのバイトコードを、制御コードに変換します。
 */
static NyLPC_TBool bc2ctrlc(const NyLPC_TChar* i_char,NyLPC_TcMiMicVM_OP_TYPE* o_opc,NyLPC_TcMiMicVM_OPR_TYPE* o_opt)
{
    //バイトコード変換の為のテーブル
    const struct TInstructionDef _bc_type_tbl[]=
    {
        //制御命令
        {".E",NyLPC_TcMiMicVM_CP_TYPE_END,NyLPC_TcMiMicVM_OPR_TYPE_NONE},
        {NULL}
    };
    int i;
    for(i=0;_bc_type_tbl[i].bcopcode!=NULL;i++){
        //2バイト一致？
        if((*i_char==_bc_type_tbl[i].bcopcode[0])&&(*(i_char+1)==_bc_type_tbl[i].bcopcode[1])){
            *o_opc=_bc_type_tbl[i].opid;
            *o_opt=_bc_type_tbl[i].optype;
            return NyLPC_TBool_TRUE;
        }
    }
    return NyLPC_TBool_FALSE;
}


/**
 * 2バイトのバイトコードを、命令定義に変換します。
 */
static NyLPC_TBool bc2opc(const NyLPC_TChar* i_char,NyLPC_TcMiMicVM_OP_TYPE* o_opc,NyLPC_TcMiMicVM_OPR_TYPE* o_opt)
{
    //バイトコード変換の為のテーブル
    const struct TInstructionDef _bc_type_tbl[]=
    {
        {"AA",NyLPC_TcMiMicVM_OP_TYPE_AND,NyLPC_TcMiMicVM_OPR_TYPE_WM_WM},
        {"AB",NyLPC_TcMiMicVM_OP_TYPE_AND,NyLPC_TcMiMicVM_OPR_TYPE_WM_H32},
        {"AE",NyLPC_TcMiMicVM_OP_TYPE_OR,NyLPC_TcMiMicVM_OPR_TYPE_WM_WM},
        {"AF",NyLPC_TcMiMicVM_OP_TYPE_OR,NyLPC_TcMiMicVM_OPR_TYPE_WM_H32},
        {"AI",NyLPC_TcMiMicVM_OP_TYPE_XOR,NyLPC_TcMiMicVM_OPR_TYPE_WM_WM},
        {"AJ",NyLPC_TcMiMicVM_OP_TYPE_XOR,NyLPC_TcMiMicVM_OPR_TYPE_WM_H32},
        {"AM",NyLPC_TcMiMicVM_OP_TYPE_NOT,NyLPC_TcMiMicVM_OPR_TYPE_WM},

        {"BA",NyLPC_TcMiMicVM_OP_TYPE_SHL,NyLPC_TcMiMicVM_OPR_TYPE_WM_H08},
        {"BB",NyLPC_TcMiMicVM_OP_TYPE_SHL,NyLPC_TcMiMicVM_OPR_TYPE_WM_WM},
        {"BE",NyLPC_TcMiMicVM_OP_TYPE_SHR,NyLPC_TcMiMicVM_OPR_TYPE_WM_H08},
        {"BF",NyLPC_TcMiMicVM_OP_TYPE_SHR,NyLPC_TcMiMicVM_OPR_TYPE_WM_WM},

        {"CA",NyLPC_TcMiMicVM_OP_TYPE_ADD,NyLPC_TcMiMicVM_OPR_TYPE_WM_WM},
        {"CB",NyLPC_TcMiMicVM_OP_TYPE_ADD,NyLPC_TcMiMicVM_OPR_TYPE_WM_H32},
        {"CE",NyLPC_TcMiMicVM_OP_TYPE_SUB,NyLPC_TcMiMicVM_OPR_TYPE_WM_WM},
        {"CF",NyLPC_TcMiMicVM_OP_TYPE_SUB,NyLPC_TcMiMicVM_OPR_TYPE_WM_H32},
        {"CI",NyLPC_TcMiMicVM_OP_TYPE_MUL,NyLPC_TcMiMicVM_OPR_TYPE_WM_WM},
        {"CJ",NyLPC_TcMiMicVM_OP_TYPE_MUL,NyLPC_TcMiMicVM_OPR_TYPE_WM_H32},

        {"DA",NyLPC_TcMiMicVM_OP_TYPE_MGET,NyLPC_TcMiMicVM_OPR_TYPE_WM_H32},
        {"DB",NyLPC_TcMiMicVM_OP_TYPE_MGET,NyLPC_TcMiMicVM_OPR_TYPE_WM_WM},
        {"DE",NyLPC_TcMiMicVM_OP_TYPE_MPUT,NyLPC_TcMiMicVM_OPR_TYPE_WM_H32},
        {"DF",NyLPC_TcMiMicVM_OP_TYPE_MPUT,NyLPC_TcMiMicVM_OPR_TYPE_WM_WM},

        {"EA",NyLPC_TcMiMicVM_OP_TYPE_SGET,NyLPC_TcMiMicVM_OPR_TYPE_WM},
        {"EE",NyLPC_TcMiMicVM_OP_TYPE_SPUT,NyLPC_TcMiMicVM_OPR_TYPE_WM},
        {"EF",NyLPC_TcMiMicVM_OP_TYPE_SPUT,NyLPC_TcMiMicVM_OPR_TYPE_H32},

        {"FA",NyLPC_TcMiMicVM_OP_TYPE_LD,NyLPC_TcMiMicVM_OPR_TYPE_WM_WM},
        {"FB",NyLPC_TcMiMicVM_OP_TYPE_LD,NyLPC_TcMiMicVM_OPR_TYPE_WM_H32},

        {"ZA",NyLPC_TcMiMicVM_OP_TYPE_NOP,NyLPC_TcMiMicVM_OPR_TYPE_NONE},
        {"ZB",NyLPC_TcMiMicVM_OP_TYPE_NOP,NyLPC_TcMiMicVM_OPR_TYPE_H08},

        {"ZE",NyLPC_TcMiMicVM_OP_TYPE_CALL,NyLPC_TcMiMicVM_OPR_TYPE_WM},
        {"ZF",NyLPC_TcMiMicVM_OP_TYPE_CALL,NyLPC_TcMiMicVM_OPR_TYPE_H32},


        {"ZZ",NyLPC_TcMiMicVM_OP_TYPE_EXIT,NyLPC_TcMiMicVM_OPR_TYPE_NONE},
        {NULL}
    };
    int i;
    //ここ早くできますよね。
    for(i=0;_bc_type_tbl[i].bcopcode!=NULL;i++){
        //2バイト一致？
        if((*i_char==_bc_type_tbl[i].bcopcode[0])&&(*(i_char+1)==_bc_type_tbl[i].bcopcode[1])){
            *o_opc=_bc_type_tbl[i].opid;
            *o_opt=_bc_type_tbl[i].optype;
            return NyLPC_TBool_TRUE;
        }
    }
    return NyLPC_TBool_FALSE;
}


/**
 * 長さi_numの16進数文字列を数値に変換する。アルファベットは小文字であること。
 * @param i_num
 * 変換する文字数
 * @param out
 *
 */
static NyLPC_TBool txt2UInt(const NyLPC_TChar* i_txt,NyLPC_TUInt8 i_num,void* out)
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
/**
 * テキストデータをWMIDに変換する。WMIDは、VMの使用の影響を受ける。
 */
static NyLPC_TBool txt2WMId(const NyLPC_TChar* i_txt,NyLPC_TUInt8* out)
{
    if(txt2UInt(i_txt,2,out)){
        if(*out<=NyLPC_TcMiMicVM_NUMBER_OF_WM){
            return NyLPC_TBool_TRUE;
        }
    }
    return NyLPC_TBool_FALSE;
}
/**
 * オペランドタイプからオペランドのBC長と、インストラクションサイズを計算
 */
static void NyLPC_TcMiMicVM_OPR_TYPE_getOpInfo(NyLPC_TcMiMicVM_OPR_TYPE i_type,NyLPC_TUInt8* oprbc_len,NyLPC_TUInt8* o_istlen)
{
    const struct{
        NyLPC_TcMiMicVM_OPR_TYPE t;
        NyLPC_TUInt8 oprbc_len;
        NyLPC_TUInt8 ist_len;
    }_tbl[]={
        {NyLPC_TcMiMicVM_OPR_TYPE_NONE,     0,          1},
        {NyLPC_TcMiMicVM_OPR_TYPE_WM_WM,    (1+1)*2,    1},
        {NyLPC_TcMiMicVM_OPR_TYPE_WM_H08,   (1+1)*2,    1},
        {NyLPC_TcMiMicVM_OPR_TYPE_WM_H16,   (1+2)*2,    2},
        {NyLPC_TcMiMicVM_OPR_TYPE_WM_H32,   (1+4)*2,    2},
        {NyLPC_TcMiMicVM_OPR_TYPE_WM,       (1)*2,      1},
        {NyLPC_TcMiMicVM_OPR_TYPE_H08,      (1)*2,      1},
        {NyLPC_TcMiMicVM_OPR_TYPE_H16,      (2)*2,      1},
        {NyLPC_TcMiMicVM_OPR_TYPE_H32,      (4)*2,      2},
        {0,0,0}
    };
    int i;
    for(i=0;_tbl[i].t!=0;i++){
        if(_tbl[i].t==i_type){
            *oprbc_len=_tbl[i].oprbc_len;
            *o_istlen=_tbl[i].ist_len;
            return;
        }
    }
    NyLPC_Abort();
    return;
}

#define TEST
#ifndef TEST
void main(void)
{
    struct NyLPC_TCharArrayPtr bc;
    const char* BC="AA0102AB0100000001AE0203AF0200000003AI0304AJ0300000004AM07BA0505BE0607CA0304CB0300000005CE0304CF0300000005CI0304CJ0300000005DA0400000000DE0400000000EA04EE04EF00000000ZAZZ.E";
    NyLPC_TcMiMicTxtCompiler_t inst;
    struct NyLPC_TUInt32ArrayPtr bin;

    NyLPC_TUInt16 l,bl;
    NyLPC_TUInt32 obuf[1024];
    NyLPC_cMiMicBcCompiler_initialize(&inst);
    bc.ptr=(char* )BC;
    bc.len=strlen(BC);
    bin.ptr=obuf;
    bin.len=5;

    for(;;){

        switch(NyLPC_cMiMicBcCompiler_compileFragment(&inst,&bc,&bin,&bl,&l))
        {
        case NyLPC_TcMiMicTxtCompiler_RET_OK:
            //命令確定。
            NyLPC_TUInt32ArrayPtr_seek(&bin,bl);
            NyLPC_TCharArrayPtr_seek(&bc,l);
            break;
        case NyLPC_TcMiMicTxtCompiler_RET_OK_END:
            //命令終端
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
