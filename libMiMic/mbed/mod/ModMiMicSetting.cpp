#include "ModMiMicSetting.h"
#include "HttpdConnection.h"
#include "NyLPC_net.h"

namespace MiMic
{
    ModMiMicSetting::ModMiMicSetting(const char* i_path):ModBaseClass(i_path)
    {
    }
    ModMiMicSetting::ModMiMicSetting():ModBaseClass()
    {
    }
    ModMiMicSetting::~ModMiMicSetting()
    {
    }
    void ModMiMicSetting::setParam(const char* i_path)
    {
        ModBaseClass::setParam(i_path);
    }
    bool ModMiMicSetting::execute(HttpdConnection& i_connection)
    {
        NyLPC_TcModMiMicSetting_t mod;

        //check parametor
        if(this->_path==NULL){
            return false;
        }
        NyLPC_cModMiMicSetting_initialize(&mod,this->_path);
        if(NyLPC_cModMiMicSetting_canHandle(&mod,i_connection._ref_inst)){
            NyLPC_cModMiMicSetting_execute(&mod,i_connection._ref_inst);
            NyLPC_cModMiMicSetting_finalize(&mod);
            return true;
        }
        NyLPC_cModMiMicSetting_finalize(&mod);
        return false;
    }

}