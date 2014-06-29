#include "NyLPC_cUPnP.h"

/**
 * コンストラクタ
 * @param i_ref_root_path
 * UPnPサービスのルートパスを指定します。このパスはNyLPC_cModUPnPDevice等のHTTPインタフェイスで使います。
 */
void NyLPC_cUPnP_initialize(NyLPC_TcUPnP_t* i_inst,NyLPC_TUInt16 i_http_port,const NyLPC_TChar* i_path,const struct NyLPC_TUPnPDevDescDevice* i_ref_description)
{
	i_inst->_ref_root_path=i_path;
	i_inst->ref_root_device=i_ref_description;
	NyLPC_cSsdpSocket_initialize(&i_inst->_ssdp,i_ref_description,i_http_port,i_path);
}
void NyLPC_cUPnP_finalize(NyLPC_TcUPnP_t* i_inst)
{
	NyLPC_Abort();//今は動かない
	NyLPC_cSsdpSocket_finalize(&i_inst->_ssdp);
}
/**
 * UPnPサービスを開始します。
 */
void NyLPC_cUPnP_start(NyLPC_TcUPnP_t* i_inst)
{
	//SSDPの開始
	NyLPC_cSsdpSocket_start(&i_inst->_ssdp);
}
