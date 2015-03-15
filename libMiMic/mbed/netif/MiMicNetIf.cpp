
////////////////////////////////////////////////////////////////////////////////
// Httpd.h
////////////////////////////////////////////////////////////////////////////////

#include "MiMicNetIf.h"
#include "mbed.h"

namespace MiMic
{
    MiMicNetIf::MiMicNetIf(){
 	}
    MiMicNetIf::~MiMicNetIf()
    {
//    	NyLPC_cMiMicIpNetIf_finalize();
    }
    const NyLPC_TiNetInterface_Interface* MiMicNetIf::getInterface()
	{
       	if(this->_inst==NULL){
       		this->_inst=NyLPC_cMiMicIpNetIf_getNetInterface();
       	}
		return this->_inst;
	}

}
