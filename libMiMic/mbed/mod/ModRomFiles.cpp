#include "ModRomFiles.h"
#include "HttpdConnection.h"

namespace MiMic
{
    ModRomFiles::ModRomFiles(const char* i_path,const NyLPC_TRomFileData* i_ref_fsdata,unsigned short i_num):ModBaseClass(i_path)
    {
    }
    ModRomFiles::ModRomFiles():ModBaseClass()
    {
    }
    ModRomFiles::~ModRomFiles()
    {
    }
    void ModRomFiles::setParam(const char* i_path,const NyLPC_TRomFileData* i_ref_fsdata,unsigned short i_num)
    {
        ModBaseClass::setParam(i_path);
        this->_ref_fsdata=i_ref_fsdata;
        this->_num=i_num;
    }
    bool ModRomFiles::execute(HttpdConnection& i_connection)
    {
        NyLPC_TcModRomFiles_t mod;

        //check parametor
        if(this->_path==NULL){
            return false;
        }
        NyLPC_cModRomFiles_initialize(&mod,this->_path,this->_ref_fsdata,this->_num);
        if(NyLPC_cModRomFiles_canHandle(&mod,i_connection._ref_inst)){
            NyLPC_cModRomFiles_execute(&mod,i_connection._ref_inst);
            NyLPC_cModRomFiles_finalize(&mod);
            return true;
        }
        NyLPC_cModRomFiles_finalize(&mod);
        return false;
    }

}