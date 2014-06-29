#include <ModUPnPDevice.h>
#include <NyLPC_net.h>
#include <stdio.h>
#include "mbed.h"
   #include "../NyLPC_cHttpdConnection_protected.h"
   
namespace MiMic
{  
    ModUPnPDevice::ModUPnPDevice():ModBaseClass(Net::UPNP_ROOT_PATH)
    {
        this->_ref_net=NULL;
    }
    ModUPnPDevice::~ModUPnPDevice()
    {
    }
    void ModUPnPDevice::setParam(const Net& i_ref_net)
    {
        this->_ref_net=&i_ref_net;
        ModBaseClass::setParam(Net::UPNP_ROOT_PATH);
    }    
    bool ModUPnPDevice::execute(HttpdConnection& i_connection)
    {
        NyLPC_TcModUPnPDevice_t mod;

        //check Net has UPnP Instance
        const NyLPC_TcUPnP_t* upnp=this->_ref_net->refUPnPInstance();
        if(upnp==NULL || this->_path==NULL){
            return false;
        }
        //
        NyLPC_cModUPnPDevice_initialize(&mod,upnp);
        if(NyLPC_cModUPnPDevice_canHandle(&mod,i_connection._ref_inst)){
            NyLPC_cModUPnPDevice_execute(&mod,i_connection._ref_inst);
            NyLPC_cModUPnPDevice_finalize(&mod);
            return true;
        }
        NyLPC_cModUPnPDevice_finalize(&mod);
        return false;
    }   
   
   
    

}