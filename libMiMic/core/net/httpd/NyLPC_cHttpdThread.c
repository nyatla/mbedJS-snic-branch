#include "NyLPC_cHttpd_protected.h"
#include "NyLPC_cHttpdConnection.h"
#include "NyLPC_cHttpdConnection_protected.h"
#include "NyLPC_cHttpdThread_protected.h"


static int server(void* p);

void NyLPC_cHttpdThread_initialize(NyLPC_TcHttpdThread_t* i_inst,NyLPC_TcHttpd_t* i_parent,NyLPC_TInt32 i_prio)
{
    NyLPC_cHttpdConnection_initialize(&(i_inst->_connection),i_parent);
    NyLPC_cThread_initialize(&(i_inst->_super),NyLPC_cHttpdThread_SIZE_OF_THREAD_STACK,i_prio);
}
void NyLPC_cHttpdThread_finalize(NyLPC_TcHttpdThread_t* i_inst)
{
    NyLPC_cThread_finalize(&i_inst->_super);
    NyLPC_cHttpdConnection_finalize(&(i_inst->_connection));
}

NyLPC_TBool NyLPC_cHttpdThread_start(NyLPC_TcHttpdThread_t* i_inst,NyLPC_TcTcpListener_t* i_listener)
{
    //停止中？
    if(!NyLPC_cThread_isTerminated(&(i_inst->_super))){
        return NyLPC_TBool_FALSE;
    }
    //リスニング
    if(!NyLPC_cHttpdConnection_listenSocket(&(i_inst->_connection),i_listener)){
        return NyLPC_TBool_FALSE;
    }
    //Accept可能なので開始。
    NyLPC_cThread_start(&(i_inst->_super),server,&i_inst->_connection);
    return NyLPC_TBool_TRUE;

}




//Httpのセッション関数
static int server(void* p)
{
    NyLPC_TcHttpdConnection_t* inst=(NyLPC_TcHttpdConnection_t*)p;
    //コネクションをAccept
    if(!NyLPC_cHttpdConnection_acceptSocket(inst)){
        NyLPC_OnErrorGoto(Error);
    }
    //コネクション数の追加
    NyLPC_cHttpd_incNumOfConnection(inst->_parent_httpd);


    //サブネットアクセスの確認
    for(;;){
        //リクエストのプレフィクスを取得
        if(!NyLPC_cHttpdConnection_prefetch(inst)){
            //Prefetch出来ないならループ終了。
            break;
        }
        //持続性接続の初期モードを設定
        if(NyLPC_cHttpd_getNumOfConnection(inst->_parent_httpd)>NyLPC_cHttpd_MAX_PERSISTENT_CONNECTION){
            NyLPC_cHttpdConnection_setConnectionMode(inst,NyLPC_TcHttpdConnection_CONNECTION_MODE_CLOSE);
        }else{
            NyLPC_cHttpdConnection_setConnectionMode(inst,NyLPC_TcHttpdConnection_CONNECTION_MODE_CONTINUE);
        }
        {//handler
            (inst->_parent_httpd->function.onRequest)(inst);
        }
        //HTTP層のクローズ
        if(!NyLPC_cHttpdConnection_closeResponse(inst)){
            break;
        }
        //次のプリフェッチを準備。
        if(!NyLPC_cHttpdConnection_prevNextPrefetch(inst)){
            break;
        }
    }
    NyLPC_cHttpd_decNumOfConnection(inst->_parent_httpd);
    NyLPC_cHttpdConnection_closeSocket(inst);
    return 0;
Error:
    NyLPC_cHttpdConnection_closeSocket(inst);
    return -1;
}

