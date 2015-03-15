#include "NyLPC_cHttpd_protected.h"


NyLPC_TBool NyLPC_cHttpd_initialize(NyLPC_TcHttpd_t* i_inst,NyLPC_TUInt16 i_port_number)
{
    int i;
    i_inst->_num_of_active_connection=0;
    NyLPC_cMutex_initialize(&i_inst->_mutex);
    i_inst->_listener=NyLPC_cNet_createTcpListenerEx(i_port_number);
    if(i_inst->_listener==NULL){
    	return NyLPC_TBool_FALSE;
    }
    for(i=0;i<NyLPC_cHttpd_NUMBER_OF_CONNECTION_THREAD;i++){
        if(!NyLPC_cHttpdThread_initialize(&(i_inst->_thread[i]),i_inst,NyLPC_TcThread_PRIORITY_IDLE)){
        	return NyLPC_TBool_FALSE;
        }
    }
	return NyLPC_TBool_TRUE;
}
void NyLPC_cHttpd_finalize(NyLPC_TcHttpd_t* i_inst)
{
	NyLPC_iTcpListener_finaize(i_inst->_listener);
    NyLPC_cMutex_finalize(&i_inst->_mutex);
}

void NyLPC_cHttpd_loop(NyLPC_TcHttpd_t* i_inst)
{
    int i;
    for(;;){
        //ターミネイト状態のタスクを検索
        for(i=0;i<NyLPC_cHttpd_NUMBER_OF_CONNECTION_THREAD;i++){
            NyLPC_cHttpdThread_start(&(i_inst->_thread[i]),i_inst->_listener);
        }
        NyLPC_cThread_yield();
    }
}



NyLPC_TUInt16 NyLPC_cHttpd_incNumOfConnection(NyLPC_TcHttpd_t* i_inst)
{
    i_inst->_num_of_active_connection++;
    NyLPC_Assert(i_inst->_num_of_active_connection<=NyLPC_cHttpd_NUMBER_OF_CONNECTION_THREAD);
    return i_inst->_num_of_active_connection;
}
NyLPC_TUInt16 NyLPC_cHttpd_decNumOfConnection(NyLPC_TcHttpd_t* i_inst)
{
    i_inst->_num_of_active_connection--;
    NyLPC_Assert(i_inst->_num_of_active_connection>=0);
    return i_inst->_num_of_active_connection;
}

/**
* Httpd全体で唯一のロックを取得する。
*/
void NyLPC_cHttpd_lock(NyLPC_TcHttpd_t* i_inst)
{
    NyLPC_cMutex_lock(&i_inst->_mutex);
}
/**
* Httpd全体で唯一のロックを開放する。
*/
void NyLPC_cHttpd_unlock(NyLPC_TcHttpd_t* i_inst)
{
    NyLPC_cMutex_unlock(&i_inst->_mutex);
}
