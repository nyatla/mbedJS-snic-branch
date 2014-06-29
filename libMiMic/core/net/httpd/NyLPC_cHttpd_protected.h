
#ifndef NYLPC_CHTTPD_PROTECTED_H_
#define NYLPC_CHTTPD_PROTECTED_H_

#include "NyLPC_stdlib.h"
#include "../NyLPC_cNetConfig.h"
#include "NyLPC_cHttpd.h"
#include "NyLPC_cHttpdThread.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef DEFINE_NyLPC_TcHttpdConnection_t
    typedef struct NyLPC_TcHttpdConnection NyLPC_TcHttpdConnection_t;
    #define DEFINE_NyLPC_TcHttpdConnection_t
#endif




#define NyLPC_cHttpd_getNumOfConnection(i_inst) ((i_inst)->_num_of_active_connection)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* NYLPC_CHTTPD_H_ */
