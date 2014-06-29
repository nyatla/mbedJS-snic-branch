#include "NyLPC_cBaseSocket.h"
#include "NyLPC_cUipService_protected.h"

void NyLPC_cBaseSocket_initialize(NyLPC_TcBaseSocket_t* i_inst,NyLPC_TUInt8 i_typeid)
{
    NyLPC_TcUipService_t* srv=_NyLPC_TcUipService_inst;
    i_inst->_typeid=i_typeid;
    i_inst->_parent_ipv4=&(srv->_tcpv4);
}
