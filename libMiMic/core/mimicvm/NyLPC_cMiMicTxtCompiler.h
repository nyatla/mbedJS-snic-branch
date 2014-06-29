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
#ifndef NYLPC_CMIMICTXTCOMPILER_H_
#define NYLPC_CMIMICTXTCOMPILER_H_
#include "NyLPC_cMiMicVM.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * クラス型を定義します。
 * NyLPC_cMiMicTxtCompilerクラスは、MiMicBCのTXTパートを、MiMicVMのインストラクション配列に変換します。
 * MiMicBCは任意区切りのフラグメントテキストで入力できます。
 */
typedef struct NyLPC_TcMiMicTxtCompiler NyLPC_TcMiMicTxtCompiler_t;


typedef NyLPC_TUInt8 NyLPC_TcMiMicTxtCompiler_ST;
#define NyLPC_TcMiMicTxtCompiler_ST_OPC  0x01   //OPをパース中
#define NyLPC_TcMiMicTxtCompiler_ST_OPR  0x02   //オペランドを解析中
#define NyLPC_TcMiMicTxtCompiler_ST_CTR  0x03   //制御パラメータを解析中
#define NyLPC_TcMiMicTxtCompiler_ST_OK   0x04   //パースを完了した。
#define NyLPC_TcMiMicTxtCompiler_ST_NG   0xff   //パースでエラーが発生ｳｪｰｲ


struct NyLPC_TcMiMicTxtCompiler{
    NyLPC_TUInt8* out_buf;
    NyLPC_TUInt8 st;//ステータス
    NyLPC_TcMiMicVM_OP_TYPE _current_opc;       //解析中のオペコード
    NyLPC_TUInt8            _current_oprtype;   //解析中のオペランドタイプ
    NyLPC_TUInt8 _inst_len;                     //解析中のインストラクションセットの長さ
    NyLPC_TUInt8 _oprbc_len;                    //オペコード解析の一時変数
    NyLPC_TUInt8 tmp_len;//tmpに格納したデータの数を数えたり。
    NyLPC_TChar  tmp[24];
};

typedef NyLPC_TUInt8 NyLPC_TcMiMicTxtCompiler_RET;
#define NyLPC_TcMiMicTxtCompiler_RET_OK 0x00
#define NyLPC_TcMiMicTxtCompiler_RET_OK_END 0x01        //命令のパースに成功し、かつEND命令を検出。
#define NyLPC_TcMiMicTxtCompiler_RET_CONTINUE   0x02    //続きのBCを要求している。
#define NyLPC_TcMiMicTxtCompiler_RET_NG         0x80



void NyLPC_cMiMicTxtCompiler_initialize(NyLPC_TcMiMicTxtCompiler_t* i_inst);
#define NyLPC_cMiMicTxtCompiler_finalize(i)
NyLPC_TcMiMicTxtCompiler_RET NyLPC_cMiMicTxtCompiler_compileFragment(NyLPC_TcMiMicTxtCompiler_t* i_inst,const struct NyLPC_TCharArrayPtr* i_bc,struct NyLPC_TUInt32ArrayPtr* i_bin,NyLPC_TUInt16* o_bin_len,NyLPC_TUInt16* o_parsed_bc);
NyLPC_TcMiMicTxtCompiler_RET NyLPC_cMiMicTxtCompiler_compileFragment2(NyLPC_TcMiMicTxtCompiler_t* i_inst,NyLPC_TChar i_bc,struct NyLPC_TUInt32ArrayPtr* i_bin,NyLPC_TUInt16* o_bin_len);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* NYLPC_CMIMICTXTCOMPILER_H_ */