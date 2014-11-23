
#include "NyLPC_cNetIf.h"
#include "./mimicip/NyLPC_cMiMicIpNetIf_protected.h"
#include "dhcp/NyLPC_cDhcpClient.h"
#include "apipa/NyLPC_cApipa.h"


/**
 * 唯一のネットワークインタフェイス
 */
const static struct NyLPC_TiNetInterface_Interface* netif;





void NyLPC_cNetIf_initialize(void)
{
	if(netif==NULL){
		//ここでネットワークインタフェイスを切り替えてくれ。
		netif=NyLPC_cMiMicIpNetIf_getNetInterface();
	}
}

void NyLPC_cNetIf_start(const NyLPC_TcIPv4Config_t* i_ref_config)
{
	netif->start(i_ref_config);
    return;
}

void NyLPC_cNetIf_stop(void)
{
	netif->stop();
    return;
}



/**
 * 指定したIPアドレスを要求するARPリクエストを発行します。
 */
void NyLPC_cNetIf_sendArpRequest(const struct NyLPC_TIPv4Addr* i_addr)
{
	netif->sendarprequest(i_addr);
}
/**
 * ARPテーブルに指定したIPがあるかを返します。
 */
NyLPC_TBool NyLPC_cNetIf_hasArpInfo(const struct NyLPC_TIPv4Addr* i_addr)
{
	return netif->hasarpinfo(i_addr);
}

NyLPC_TBool NyLPC_cNetIf_isInitService(void)
{
	return netif->isinitservice();
}

NyLPC_TiTcpSocket_t* NyLPC_cNetIf_createTcpSocketEx(NyLPC_TSocketType i_socktype)
{
	return netif->createTcpSocketEx(i_socktype);
}
NyLPC_TiUdpSocket_t* NyLPC_cNetIf_createUdpSocketEx(NyLPC_TUInt16 i_port,NyLPC_TSocketType i_socktype)
{
	return netif->createUdpSocetEx(i_port,i_socktype);
}
NyLPC_TiTcpListener_t* NyLPC_cNetIf_createTcpListenerEx(NyLPC_TUInt16 i_port)
{
	return netif->createTcpListener(i_port);
}

const struct NyLPC_TNetInterfaceInfo* NyLPC_cNetIf_getInterfaceInfo(void)
{
	return netif->getinterfaceinfo();
}



NyLPC_TBool NyLPC_cNetIf_requestAddrDhcp(NyLPC_TcIPv4Config_t* i_cfg,NyLPC_TInt16 i_repeat)
{
    NyLPC_TBool ret;
    NyLPC_TcDhcpClient_t sock;
    //netを開始
    NyLPC_cDhcpClient_initialize(&sock);
    ret=NyLPC_cDhcpClient_requestAddr(&sock,i_cfg,i_repeat);
    NyLPC_cDhcpClient_finalize(&sock);
    return ret;
}

NyLPC_TBool NyLPC_cNetIf_requestAddrApipa(NyLPC_TcIPv4Config_t* i_cfg,NyLPC_TInt16 i_repeat)
{
    NyLPC_TBool ret;
    NyLPC_TcApipa_t sock;
    //netを開始
    NyLPC_cApipa_initialize(&sock);
    ret=NyLPC_cApipa_requestAddr(&sock,i_cfg,i_repeat);
    NyLPC_cApipa_finalize(&sock);
    return ret;
}



