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
#ifndef NYLPC_CMIMICVM_PROTECTED_H_
#define NYLPC_CMIMICVM_PROTECTED_H_
#include "NyLPC_cMiMicVM.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
//----------------------------------------------------------------------
//MiMicOpCodeのタイプ値
//----------------------------------------------------------------------

#define NyLPC_TcMiMicVM_OP_TYPE_AND    0x11
#define NyLPC_TcMiMicVM_OP_TYPE_OR     0x12
#define NyLPC_TcMiMicVM_OP_TYPE_XOR    0x13
#define NyLPC_TcMiMicVM_OP_TYPE_NOT    0x14

#define NyLPC_TcMiMicVM_OP_TYPE_SHL    0x21
#define NyLPC_TcMiMicVM_OP_TYPE_SHR    0x22

#define NyLPC_TcMiMicVM_OP_TYPE_ADD    0x31
#define NyLPC_TcMiMicVM_OP_TYPE_SUB    0x32
#define NyLPC_TcMiMicVM_OP_TYPE_MUL    0x33

#define NyLPC_TcMiMicVM_OP_TYPE_MGET    0x41
#define NyLPC_TcMiMicVM_OP_TYPE_MPUT    0x42

#define NyLPC_TcMiMicVM_OP_TYPE_SGET    0x51
#define NyLPC_TcMiMicVM_OP_TYPE_SPUT    0x52

#define NyLPC_TcMiMicVM_OP_TYPE_NOP     0x61
#define NyLPC_TcMiMicVM_OP_TYPE_EXIT    0x62
#define NyLPC_TcMiMicVM_OP_TYPE_CALL    0x63

#define NyLPC_TcMiMicVM_OP_TYPE_LD      0x71


#define NyLPC_TcMiMicVM_CP_TYPE_END    0x01
/*
#define NyLPC_TcMiMicVM_OP_TYPE_V_END  0x1001
#define NyLPC_TcMiMicVM_OP_TYPE_V_DB8  0x1011
#define NyLPC_TcMiMicVM_OP_TYPE_V_DB16 0x1012
#define NyLPC_TcMiMicVM_OP_TYPE_V_DB32 0x1013
*/

//----------------------------------------------------------------------
//MiMicOprandのタイプ値
//----------------------------------------------------------------------

#define NyLPC_TcMiMicVM_OPR_TYPE_NONE    0x01
#define NyLPC_TcMiMicVM_OPR_TYPE_WM_WM   0x11
#define NyLPC_TcMiMicVM_OPR_TYPE_WM_H08  0x12
#define NyLPC_TcMiMicVM_OPR_TYPE_WM_H16  0x13
#define NyLPC_TcMiMicVM_OPR_TYPE_WM_H32  0x14
#define NyLPC_TcMiMicVM_OPR_TYPE_WM      0x21
#define NyLPC_TcMiMicVM_OPR_TYPE_H08     0x22
#define NyLPC_TcMiMicVM_OPR_TYPE_H16     0x23
#define NyLPC_TcMiMicVM_OPR_TYPE_H32     0x24





/**
 * インストラクションセットのバイナリデータ型。
 * MiMicVMで実行するインストラクションをバッファから参照するために使います。
 * キャスト元のバッファは、32bitのunsigned intを想定しています。境界は32bit単位です。
 *
 * 構造体は、
 *
 */
union NyLPC_TcMiMicVM_TInstruction{
    /**
     * オペコード情報のみを格納します
     */
    struct{
        NyLPC_TcMiMicVM_OP_TYPE opc;
        NyLPC_TUInt8 oprtype;
    }op;
    //32ビットコード

    /**
     * WM[8],WM[8]オペランドの命令を格納します。
     */
    struct{
        NyLPC_TcMiMicVM_OP_TYPE opc;
        NyLPC_TUInt8 oprtype;
        NyLPC_TUInt8 wm1;
        NyLPC_TUInt8 wm2;
    }wmwm_32;
    struct{
        NyLPC_TcMiMicVM_OP_TYPE opc;
        NyLPC_TUInt8 oprtype;
        NyLPC_TUInt8 wm;
        NyLPC_TUInt8 h8;
    }wmh08_32;
    struct{
        NyLPC_TcMiMicVM_OP_TYPE opc;
        NyLPC_TUInt8 oprtype;
        NyLPC_TUInt8  wm;
    }wm_32;
    struct{
        NyLPC_TcMiMicVM_OP_TYPE opc;
        NyLPC_TUInt8 oprtype;
        NyLPC_TUInt8  h8;
    }h8_32;
    struct{
        NyLPC_TcMiMicVM_OP_TYPE opc;
        NyLPC_TUInt8 oprtype;
        NyLPC_TUInt8  h16;
    }h16_32;
    //64bit命令
    struct{
        NyLPC_TcMiMicVM_OP_TYPE opc;
        NyLPC_TUInt8 oprtype;
        NyLPC_TUInt8  wm;
        NyLPC_TUInt8  _padding;
        NyLPC_TUInt16 h16;
    }wmh16_64;
    struct{
        NyLPC_TcMiMicVM_OP_TYPE opc;
        NyLPC_TUInt8 oprtype;
        NyLPC_TUInt8  wm;
        NyLPC_TUInt8  _padding;
        NyLPC_TUInt32 h32;
    }wmh32_64;
    struct{
        NyLPC_TcMiMicVM_OP_TYPE opc;
        NyLPC_TUInt8 oprtype;
        NyLPC_TUInt8  _padding[2];
        NyLPC_TUInt32 h32;
    }h32_64;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CMIMICVM_PROTECTED_H_ */
