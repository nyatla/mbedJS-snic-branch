#include "NyLPC_cMiMicIpBaseSocket.h"
#include "NyLPC_cMiMicIpNetIf_protected.h"


void NyLPC_cMiMicIpBaseSocket_initialize(NyLPC_TcMiMicIpBaseSocket_t* i_inst,NyLPC_TUInt8 i_typeid)
{
	NyLPC_TcMiMicIpNetIf_t* srv=_NyLPC_TcMiMicIpNetIf_inst;
    i_inst->_typeid=i_typeid;
    i_inst->_parent_ipv4=&(srv->_tcpv4);
}
