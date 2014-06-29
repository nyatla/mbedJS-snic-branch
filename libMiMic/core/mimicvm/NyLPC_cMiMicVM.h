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
#ifndef NYLPC_CMIMICVM_H_
#define NYLPC_CMIMICVM_H_

#include "NyLPC_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NyLPC_TcMiMicVM NyLPC_TcMiMicVM_t;
struct NyLPC_TcMiMicVM_TEvent;

#define NyLPC_cMiMicVM_RESULT_OK 0x00000000
#define NyLPC_cMiMicVM_RESULT_NG 0x80000000
#define NyLPC_cMiMicVM_RESULT_RUNTIME_NG (NyLPC_cMiMicVM_RESULT_NG|0x00010000)
#define NyLPC_cMiMicVM_RESULT_RUNTIME_NG_UNKNOWN_CALL (NyLPC_cMiMicVM_RESULT_RUNTIME_NG|0x00000021)
#define NyLPC_cMiMicVM_RESULT_RUNTIME_NG_CALL (NyLPC_cMiMicVM_RESULT_RUNTIME_NG|0x00000022)
#define NyLPC_cMiMicVM_RESULT_isOK(v) ((v&0x80000000)==0x00000000)

/**
 * MiMicVMのワークメモリの個数
 */
#define NyLPC_TcMiMicVM_NUMBER_OF_WM 8

typedef NyLPC_TUInt8 NyLPC_TcMiMicVM_OP_TYPE;
typedef NyLPC_TUInt8 NyLPC_TcMiMicVM_OPR_TYPE;

typedef NyLPC_TBool (*NyLPC_TcMiMicVM_putStream)(struct NyLPC_TcMiMicVM_TEvent* i_evh,NyLPC_TUInt32 i_val);
typedef NyLPC_TBool (*NyLPC_TcMiMicVM_getStream)(struct NyLPC_TcMiMicVM_TEvent* i_evh,NyLPC_TUInt32* o_val);
/**
 * MiMicVMのCALL命令ハンドラ。
 * i_idに関数IDを指定する。
 * @param i_id
 * CALL命令のパラメタ
 * @param i_vm
 * VMのインスタンス。
 * @return
 * MiMicVMのエラーコード。
 * 関数コールが成功したら、NyLPC_cMiMicVM_RESULT_OKを返すこと。エラーの場合はNyLPC_cMiMicVM_RESULT_NG又はカスタムエラーコードを返すこと。
 */
typedef NyLPC_TUInt32 (*NyLPC_TcMiMicVM_nativeCall)(struct NyLPC_TcMiMicVM_TEvent* i_evh,NyLPC_TUInt32 i_id,NyLPC_TcMiMicVM_t* i_vm);
typedef void (*NyLPC_TcMiMicVM_sleep)(struct NyLPC_TcMiMicVM_TEvent* i_evh,NyLPC_TUInt32 i_sleep_in_msec);
struct NyLPC_TcMiMicVM_TEvent
{
    NyLPC_TcMiMicVM_putStream put_stream;
    NyLPC_TcMiMicVM_getStream get_stream;
    NyLPC_TcMiMicVM_sleep sleep;
    NyLPC_TcMiMicVM_nativeCall native_call;
};


struct NyLPC_TcMiMicVM
{
    struct NyLPC_TcMiMicVM_TEvent* _event_handler;
    NyLPC_TUInt32 wm[NyLPC_TcMiMicVM_NUMBER_OF_WM];
};
void NyLPC_cMiMicVM_initialize(NyLPC_TcMiMicVM_t* i_inst,struct NyLPC_TcMiMicVM_TEvent* i_handler);
#define NyLPC_cMiMicVM_finalize(i);
NyLPC_TUInt32 NyLPC_cMiMicVM_run(NyLPC_TcMiMicVM_t* i_inst,const NyLPC_TUInt32* i_instruction,const NyLPC_TUInt16 i_size_of_instruction);
NyLPC_TBool NyLPC_cMiMicVM_sput(NyLPC_TcMiMicVM_t* i_inst,NyLPC_TUInt32 i_val);
NyLPC_TBool NyLPC_cMiMicVM_sget(NyLPC_TcMiMicVM_t* i_inst,NyLPC_TUInt32* o_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CMIMICVM_H_ */
