#include "ModJsonRpc.h"
#include "../net/httpd/mod/NyLPC_cModWebSocket_protected.h"
#include "HttpdConnection.h"

namespace MiMic
{
    #define NUM_OF_OBJECTS 16
    ModJsonRpc::BasicRpcObject** newObjectArray()
    {
        ModJsonRpc::BasicRpcObject** r=new ModJsonRpc::BasicRpcObject*[NUM_OF_OBJECTS];
        memset(r,0,sizeof(ModJsonRpc::BasicRpcObject*)*NUM_OF_OBJECTS);
        return r;
    }
    void deleteObjectArray(ModJsonRpc::BasicRpcObject** v)
    {
        for(int i=0;i<NUM_OF_OBJECTS;i++){
            if(v[i]!=NULL){
                delete v[i];
            }
        }
        delete[] v;
    }    
    
    ModJsonRpc::ModJsonRpc(const char* i_path,const struct NyLPC_TJsonRpcClassDef** i_rpc_table):ModBaseClass(i_path)
    {
        this->_mod=NULL;
        this->_rpc_table=i_rpc_table;
    }
    ModJsonRpc::ModJsonRpc()
    {
        this->_mod=NULL;
    }
    ModJsonRpc::~ModJsonRpc()
    {
        if(this->_mod!=NULL){
            NyLPC_cModJsonRpc_finalize(&(this->_mod->super));
            free(this->_mod);
            deleteObjectArray(this->_objects);
            this->_mod=NULL;
        }
    }
    void ModJsonRpc::setParam(const char* i_path,const struct NyLPC_TJsonRpcClassDef** i_rpc_table)
    {
        ModBaseClass::setParam(i_path);
        this->_rpc_table=i_rpc_table;
    }
    bool ModJsonRpc::isStarted(){
        return this->_mod!=NULL;
    }
    bool ModJsonRpc::execute(HttpdConnection& i_connection)
    {
        i_connection.lockHttpd();
        if(this->_mod!=NULL){
            i_connection.unlockHttpd();
            return false;
        }
        this->_mod=(TcJsonRpcEx_t*)malloc(sizeof(TcJsonRpcEx_t));
        i_connection.unlockHttpd();
        if(this->_mod==NULL){
            return false;
        }
        this->_mod->cppmod_ptr=this;

        //initialize websocket
        NyLPC_cModJsonRpc_initialize(&(this->_mod->super),this->_path,this->_rpc_table);
        if(NyLPC_cModJsonRpc_canHandle(&(this->_mod->super),i_connection._ref_inst)){
            if(NyLPC_cModJsonRpc_execute(&(this->_mod->super),i_connection._ref_inst)){
                //initialize object array
                this->_objects=newObjectArray();
                return true;
            }
        }
        NyLPC_cModJsonRpc_finalize(&(this->_mod->super));
        free(this->_mod);
        i_connection.lockHttpd();
        this->_mod=NULL;
        i_connection.unlockHttpd();
        return false;
    }

    void ModJsonRpc::dispatchRpc()
    {
        const union NyLPC_TJsonRpcParserResult* rpc_result;
        if(this->_mod==NULL){
            return;
        }        
        for(;;){
            if(!NyLPC_cModJsonRpc_processRpcMessage(&(this->_mod->super))){
                break;
            }
            //メッセージ取得を試行
            rpc_result=NyLPC_cModJsonRpc_getMessage(&(this->_mod->super));
            if(rpc_result==NULL){
                //nothing
                continue;
            }
            if(NyLPC_TJsonRpcParserResult_hasMethodHandler(rpc_result)){
                if(NyLPC_TJsonRpcParserResult_callMethodHandler(rpc_result,this->_mod)){
                    continue;
                }else{
                    //function failed.
                    break;
                }
            }else{
                //no handler
                break;
            }
        }
        NyLPC_cModJsonRpc_close(&(this->_mod->super),1000);
        NyLPC_cModJsonRpc_finalize(&(this->_mod->super));
        deleteObjectArray(this->_objects);
        free(this->_mod);
        this->_mod=NULL;
        return;
    }
    bool ModJsonRpc::putResult(unsigned int i_id)
    {
        return this->putResult(i_id,"");
    }
    bool ModJsonRpc::putResult(unsigned int i_id,const char* i_params_fmt,...)
    {
        bool ret;
        va_list a;
        va_start(a,i_params_fmt);
        ret=NyLPC_cModJsonRpc_putResultV(&(this->_mod->super),i_id,i_params_fmt,a)?true:false;
        va_end(a);
        return ret;
    }
    bool ModJsonRpc::putError(unsigned int i_id,int i_code)
    {
        return NyLPC_cModJsonRpc_putError(&(this->_mod->super),i_id,i_code)?true:false;
    }
    
    int ModJsonRpc::addObject(BasicRpcObject* i_object)
    {
        for(int i=0;i<NUM_OF_OBJECTS;i++){
            if(this->_objects[i]==NULL){
                this->_objects[i]=i_object;
                return i;
            }
        }
        return -1;
    }
    bool ModJsonRpc::removeObject(int i_id)
    {
        if(0<=i_id && i_id<NUM_OF_OBJECTS){
            if(this->_objects[i_id]!=NULL){
                delete this->_objects[i_id];
                this->_objects[i_id]=NULL;
                return true;
            }
        }
        return false;
    }
    void* ModJsonRpc::getObject(int i_oid)
    {
        return this->_objects[i_oid]->obj;
    }

}
