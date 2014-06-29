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
#ifndef NYLPC_CTHREAD_H_
#define NYLPC_CTHREAD_H_

////////////////////////////////////////////////////////////////////////////////
// Include
////////////////////////////////////////////////////////////////////////////////
#include "NyLPC_stdlib.h"
#if NyLPC_ARCH==NyLPC_ARCH_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#elif NyLPC_ARCH==NyLPC_ARCH_MBEDRTOS
#elif NyLPC_ARCH==NyLPC_ARCH_WIN32
    //not work!
#else
#error Bad NyLPC_ARCH!
#endif

/**********************************************************************
 *
 * NyLPC_TcThread class
 *
 **********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * 関数スレッドを実行します。
 */
typedef struct NyLPC_TcThread NyLPC_TcThread_t;

/** スレッドの関数型 */
typedef int (*NyLPC_TcThread_ThreadFunc)(void* i_param);
/** */
#define NyLPC_TcThread_DEFAULT_STACKSIZE configMINIMAL_STACK_SIZE

/** ターミネイトしたときに１*/
#define NyLPC_TcThread_BIT_IS_TERMINATED 0
/** JOINリクエストがある場合に1*/
#define NyLPC_TcThread_BIT_IS_JOIN_REQ   1

/**
 * Thread priorities
 */
#define NyLPC_TcThread_PRIORITY_IDLE    0
#define NyLPC_TcThread_PRIORITY_SERVICE 1

#if NyLPC_ARCH==NyLPC_ARCH_FREERTOS

struct NyLPC_TcThread {
    NyLPC_TUInt32 _sbit;
    xTaskHandle _taskid;
    NyLPC_TcThread_ThreadFunc _func;
    void* _param;
};
#elif NyLPC_ARCH==NyLPC_ARCH_MBEDRTOS

struct NyLPC_TcThread
{
    NyLPC_TUInt32 _sbit;
    void* _thread; //Thread* type.
    NyLPC_TcThread_ThreadFunc _func;
    void* _arg;
};
#elif NyLPC_ARCH==NyLPC_ARCH_WIN32
//this is dummy definition.
struct NyLPC_TcThread
{
    int dummy;
    NyLPC_TcThread_ThreadFunc _func;
};
#else
#error Bad NyLPC_ARCH!
#endif

/**
 * 終了要求フラグの値を返します。
 * スレッドのループ内で、Join要求の有無を確認するために使います。
 * スレッドは定期的にこの関数を実行して、trueの場合は速やかに関数を終了する必要があります。
 */
#define NyLPC_cThread_isJoinReqest(i_inst) NyLPC_TUInt32_isBitOn((i_inst)->_sbit,NyLPC_TcThread_BIT_IS_JOIN_REQ)
#define NyLPC_cThread_isTerminated(i_inst) NyLPC_TUInt32_isBitOn((i_inst)->_sbit,NyLPC_TcThread_BIT_IS_TERMINATED)

void NyLPC_cThread_initialize(NyLPC_TcThread_t* i_inst, NyLPC_TInt32 i_stack,NyLPC_TInt32 i_prio);
void NyLPC_cThread_finalize(NyLPC_TcThread_t* i_inst);
void NyLPC_cThread_start(NyLPC_TcThread_t* i_inst,
        NyLPC_TcThread_ThreadFunc i_func, void* i_param);
void NyLPC_cThread_join(NyLPC_TcThread_t* i_inst);
void NyLPC_cThread_yield(void);
/**
 * 一定時間、スレッドをスリープする。
 * この関数はstatic関数です。
 */
void NyLPC_cThread_sleep(NyLPC_TUInt32 i_time_in_msec);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CTHREAD_H_ */
