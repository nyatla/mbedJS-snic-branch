#include <ModFileIo.h>
#include <stdio.h>
#include "mbed.h"
using namespace MiMic;
/*
 * FileIO class
 */
typedef struct TcModFileIoDumy
{
    NyLPC_TcModFileIoBaseClass_t super;
}TcFileIoDumy_t;

const static char* STR_TEXT_HTML="text/html";
#define SIZE_OF_FBUF 256



static NyLPC_TBool upload_handler(NyLPC_TcHttpdConnection_t* i_connection,const NyLPC_TChar* i_fname,NyLPC_TcHttpBodyParser_t* i_body_parser)
{
    char* buf=Httpd::_shared_buf;// This handler called with lock!

    //save to new file
    FILE *fp;
    fp = fopen(i_fname, "w");
    if(fp==NULL){
        NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
        return NyLPC_TBool_FALSE;
    }
    NyLPC_TInt16 l;
    if(NyLPC_cHttpBodyParser_parseStream(i_body_parser,NyLPC_cHttpdConnection_refStream(i_connection),buf,SIZE_OF_FBUF,&l)){
        while(l>0){
            fwrite(buf,1,l,fp);            
            if(!NyLPC_cHttpBodyParser_parseStream(i_body_parser,NyLPC_cHttpdConnection_refStream(i_connection),buf,SIZE_OF_FBUF,&l)){
                NyLPC_OnErrorGoto(Error_FILE);
            }
        }
    }
    fclose(fp);    
    //write response
    NyLPC_cHttpdConnection_sendResponseHeader2(i_connection,200,STR_TEXT_HTML,0,NULL);
    return NyLPC_TBool_TRUE;
Error_FILE:
    fclose(fp);
    NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
    return NyLPC_TBool_FALSE;
}

static NyLPC_TBool create_handler(NyLPC_TcHttpdConnection_t* i_connection,const NyLPC_TChar* i_fname)
{
    FILE *fp; 
    fp = fopen(i_fname, "w");
    if(fp!=NULL){
        fclose(fp);
        NyLPC_cHttpdConnection_sendResponseHeader2(i_connection,200,STR_TEXT_HTML,0,NULL);
        return NyLPC_TBool_TRUE;
    }
    NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
    return NyLPC_TBool_FALSE;
}

static NyLPC_TBool remove_handler(NyLPC_TcHttpdConnection_t* i_connection,const NyLPC_TChar* i_fname)
{
    if(remove(i_fname)==0){
        NyLPC_cHttpdConnection_sendResponseHeader2(i_connection,200,STR_TEXT_HTML,0,NULL);
        return NyLPC_TBool_TRUE;
    }
    //500ERROR
    NyLPC_cHttpdUtils_sendErrorResponse(i_connection,500);
    return NyLPC_TBool_FALSE;
}



static void cModFileIoDumy_initialize(TcFileIoDumy_t* i_inst,const char* i_root_path)
{
    NyLPC_cModFileIoBaseClass_initialize(&(i_inst->super),i_root_path);
    i_inst->super._abstruct_function.upload=upload_handler;
    i_inst->super._abstruct_function.create=create_handler;
    i_inst->super._abstruct_function.remove=remove_handler;
}
#define cModFileIoDumy_finalize(i_inst) NyLPC_cModFileIoBaseClass_finalize(&(i_inst)->super)
#define cModFileIoDumy_canHandle(i_inst,i_connection) NyLPC_cModFileIoBaseClass_canHandle(&(i_inst)->super,i_connection)
#define cModFileIoDumy_execute(i_inst,i_connection) NyLPC_cModFileIoBaseClass_execute(&(i_inst)->super,i_connection)





   
   
namespace MiMic
{  
   
   
    ModFileIo::ModFileIo(const char* i_path):ModBaseClass(i_path)
    {
    }
    ModFileIo::ModFileIo()
    {
    }
    ModFileIo::~ModFileIo()
    {
    }
    void ModFileIo::setParam(const char* i_path)
    {
        ModBaseClass::setParam(i_path);
    }
    bool ModFileIo::execute(HttpdConnection& i_connection)
    {
        //check parametor
        if(this->_path==NULL){
            return false;
        }
        TcModFileIoDumy mod;
        cModFileIoDumy_initialize(&mod,this->_path);
        if(cModFileIoDumy_canHandle(&mod,i_connection._ref_inst)){
            cModFileIoDumy_execute(&mod,i_connection._ref_inst);
            cModFileIoDumy_finalize(&mod);
            return true;
        }
        cModFileIoDumy_finalize(&mod);
        return false;
    }   
   
   
    

}