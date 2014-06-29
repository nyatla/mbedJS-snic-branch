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
#include "../NyLPC_cThread.h"

#if NyLPC_ARCH==NyLPC_ARCH_MBEDRTOS
#include "mbed.h"
#include "rtos.h"

static osPriority prio_table[]={
    osPriorityNormal,osPriorityHigh};



static void proc(void const *argument)
{
    NyLPC_TcThread_t* t=(NyLPC_TcThread_t*)argument;
    for(;;){
        do{
           Thread::wait(30);// danger wait!
        }while(NyLPC_TUInt32_isBitOn(t->_sbit,NyLPC_TcThread_BIT_IS_TERMINATED));
        t->_func(t->_arg);
        NyLPC_TUInt32_setBit(t->_sbit,NyLPC_TcThread_BIT_IS_TERMINATED);
    }
}


void NyLPC_cThread_initialize(NyLPC_TcThread_t* i_inst,NyLPC_TInt32 i_stack,NyLPC_TInt32 i_prio)
{
    NyLPC_TUInt32_setBit(i_inst->_sbit,NyLPC_TcThread_BIT_IS_TERMINATED);
    i_inst->_thread=new Thread(proc,i_inst,prio_table[i_prio],i_stack);
}
void NyLPC_cThread_finalize(NyLPC_TcThread_t* i_inst)
{
    NyLPC_cThread_join(i_inst);
    delete (Thread*)(i_inst->_thread);
}


void NyLPC_cThread_start(NyLPC_TcThread_t* i_inst,NyLPC_TcThread_ThreadFunc i_func,void* i_param)
{
    NyLPC_ArgAssert(i_inst!=NULL);
    NyLPC_ArgAssert(i_func!=NULL);
    i_inst->_sbit=0;
    i_inst->_func=i_func;
    i_inst->_arg=i_param;
    return;
}
void NyLPC_cThread_join(NyLPC_TcThread_t* i_inst)
{
    NyLPC_TUInt32_setBit(i_inst->_sbit,NyLPC_TcThread_BIT_IS_JOIN_REQ);
    while(!NyLPC_TUInt32_isBitOn(i_inst->_sbit,NyLPC_TcThread_BIT_IS_TERMINATED))
    {
        Thread::wait(10);
    }
    return;
}
void NyLPC_cThread_sleep(NyLPC_TUInt32 i_time_in_msec)
{
    //ミリ秒単位で待つ
    Thread::wait(i_time_in_msec);
}
void NyLPC_cThread_yield(void)
{
    Thread::yield();
}

#endif
