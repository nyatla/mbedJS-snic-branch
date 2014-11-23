/*
 * NyLPC_TcHttpd_Thread.h
 *
 *  Created on: 2013/02/07
 *      Author: nyatla
 */

#ifndef NYLPC_TCHTTPDTHREAD_H_
#define NYLPC_TCHTTPDTHREAD_H_
#include "NyLPC_stdlib.h"
#include "../NyLPC_cNetConfig.h"
#include "NyLPC_cHttpdConnection.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef DEFINE_NyLPC_TcHttpd_t
    typedef struct NyLPC_TcHttpd NyLPC_TcHttpd_t;
    #define DEFINE_NyLPC_TcHttpd_t
#endif

/** Httpdセッションスレッドのスタックサイズ*/
#ifndef NyLPC_cHttpdThread_SIZE_OF_THREAD_STACK
#   define NyLPC_cHttpdThread_SIZE_OF_THREAD_STACK 1024
#endif

typedef struct NyLPC_TcHttpdThread NyLPC_TcHttpdThread_t;
/**
 * このクラスは、httpdのワーカースレッドです。1セッションを担当します。
 */
struct NyLPC_TcHttpdThread
{
    NyLPC_TcThread_t _super;
    /** 関数アドレスを格納したポインタ*/
    NyLPC_TcHttpdConnection_t _connection;
};


NyLPC_TBool NyLPC_cHttpdThread_initialize(NyLPC_TcHttpdThread_t* i_inst,NyLPC_TcHttpd_t* i_parent,NyLPC_TInt32 i_prio);
void NyLPC_cHttpdThread_finalize(NyLPC_TcHttpdThread_t* i_inst);
NyLPC_TBool NyLPC_cHttpdThread_start(NyLPC_TcHttpdThread_t* i_inst,NyLPC_TiTcpListener_t* i_listener);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* NYLPC_TCHTTPDTHREAD_H_ */
