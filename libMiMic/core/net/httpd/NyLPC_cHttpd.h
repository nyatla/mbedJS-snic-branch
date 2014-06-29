
#ifndef NYLPC_CHTTPD_H_
#define NYLPC_CHTTPD_H_

#include "NyLPC_stdlib.h"
#include "../NyLPC_cNetConfig.h"
#include "NyLPC_cHttpdThread.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * HTTPコネクションスレッドの数
 */
#ifndef NyLPC_cHttpd_NUMBER_OF_CONNECTION_THREAD
#   define NyLPC_cHttpd_NUMBER_OF_CONNECTION_THREAD 3
#endif

/**
 * 持続性接続を許可するコネクションの数
 * NyLPC_cHttpd_NUMBER_OF_CONNECTION_THREAD-1以下にしてください。
 */
#ifndef NyLPC_cHttpd_MAX_PERSISTENT_CONNECTION
#   define NyLPC_cHttpd_MAX_PERSISTENT_CONNECTION (NyLPC_cHttpd_NUMBER_OF_CONNECTION_THREAD-1)
#endif





#ifndef DEFINE_NyLPC_TcHttpdConnection_t
    typedef struct NyLPC_TcHttpdConnection NyLPC_TcHttpdConnection_t;
    #define DEFINE_NyLPC_TcHttpdConnection_t
#endif





typedef void (*NyLPC_TcHttpd_onRequest)(NyLPC_TcHttpdConnection_t* i_inst);

/**
 * class definition
 */
#ifndef DEFINE_NyLPC_TcHttpd_t
    typedef struct NyLPC_TcHttpd NyLPC_TcHttpd_t;
    #define DEFINE_NyLPC_TcHttpd_t
#endif
struct NyLPC_TcHttpd
{
    struct{
        NyLPC_TcHttpd_onRequest onRequest;
    }function;
    NyLPC_TcMutex_t _mutex;
    NyLPC_TcTcpListener_t _listener;
    NyLPC_TcHttpdThread_t _thread[NyLPC_cHttpd_NUMBER_OF_CONNECTION_THREAD];
    NyLPC_TInt16 _num_of_active_connection;
};




void NyLPC_cHttpd_initialize(NyLPC_TcHttpd_t* i_inst,NyLPC_TUInt16 i_port_number);
void NyLPC_cHttpd_finalize(NyLPC_TcHttpd_t* i_inst);
void NyLPC_cHttpd_loop(NyLPC_TcHttpd_t* i_inst);
void NyLPC_cHttpd_lock(NyLPC_TcHttpd_t* i_inst);
void NyLPC_cHttpd_unlock(NyLPC_TcHttpd_t* i_inst);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CHTTPD_H_ */
