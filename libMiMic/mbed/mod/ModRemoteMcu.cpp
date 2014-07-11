#include "ModRemoteMcu.h"
#include "HttpdConnection.h"

namespace MiMic
{
    ModRemoteMcu::ModRemoteMcu(const char* i_path):ModBaseClass(i_path)
    {
    }
    ModRemoteMcu::ModRemoteMcu()
    {
    }
    ModRemoteMcu::~ModRemoteMcu()
    {
    }
    void ModRemoteMcu::setParam(const char* i_path)
    {
        ModBaseClass::setParam(i_path);
    }
    bool ModRemoteMcu::execute(HttpdConnection& i_connection)
    {
        NyLPC_TcModRemoteMcu_t mod;

        //check parametor
        if(this->_path==NULL){
            return false;
        }
        NyLPC_cModRemoteMcu_initialize(&mod,this->_path);
        if(NyLPC_cModRemoteMcu_canHandle(&mod,i_connection._ref_inst)){
            NyLPC_cModRemoteMcu_execute(&mod,i_connection._ref_inst);
            NyLPC_cModRemoteMcu_finalize(&mod);
            return true;
        }
        NyLPC_cModRemoteMcu_finalize(&mod);
        return false;
    }

}
