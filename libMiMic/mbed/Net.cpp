#pragma once
////////////////////////////////////////////////////////////////////////////////
// Net.h
////////////////////////////////////////////////////////////////////////////////
#include "Net.h"
#include "NetConfig.h"
#include "mbed.h"
DigitalOut l2(LED2);    
DigitalOut l3(LED3);
DigitalOut l4(LED4);
namespace MiMic
{

    const char* Net::UPNP_ROOT_PATH="upnp";

    Net::Net()
    {
        NyLPC_cNet_initialize(&(this->_inst));
        this->_mdns=NULL;
        this->_upnp=NULL;
    }
    Net::~Net()
    {
        NyLPC_cNet_finalize(&(this->_inst));
    }
    void Net::start(NetConfig& i_cfg)
    {
        NyLPC_TcNetConfig_t* base_cfg=i_cfg.refBaseInstance();
        //DHCP & autoIP request
        if(base_cfg->tcp_mode & NyLPC_TcNetConfig_IPV4_FLAG_MODE_MASK!=0){
            for(;;){
                //DHCP
                if((base_cfg->tcp_mode & NyLPC_TcNetConfig_IPV4_FLAG_MODE_DHCP)!=0){
                    if(NyLPC_cNet_requestAddrDhcp(&(this->_inst),&(base_cfg->super),3)){
                        break;
                    }
                }
                //AUTOIP
                if((base_cfg->tcp_mode & NyLPC_TcNetConfig_IPV4_FLAG_MODE_AUTOIP)!=0){
                    NyLPC_TcApipa_t apipa;
                    NyLPC_cApipa_initialize(&apipa);
                    if(NyLPC_cApipa_requestAddr(&apipa,&(base_cfg->super),3)){
                        break;
                    }
                }
            }
        }
        //start mDNS
        if((base_cfg->services.flags & NyLPC_TcNetConfig_SERVICE_FLAG_MDNS) !=0){
            this->_mdns=(NyLPC_TcMDnsServer_t*)malloc(sizeof(NyLPC_TcMDnsServer_t));
            NyLPC_cMDnsServer_initialize(this->_mdns,i_cfg.refMdnsRecord());        
        }
        //start UPnP
        if((base_cfg->services.flags & NyLPC_TcNetConfig_SERVICE_FLAG_UPNP) !=0){
            this->_upnp=(NyLPC_TcUPnP_t*)malloc(sizeof(NyLPC_TcUPnP_t));
            NyLPC_cUPnP_initialize(this->_upnp,i_cfg.getHttpPort(),UPNP_ROOT_PATH,i_cfg.refUPnPDevDesc());        

        }
        NyLPC_cNet_start(&(this->_inst),base_cfg);
        if(this->_upnp!=NULL){
            NyLPC_cUPnP_start(this->_upnp);
        }        
    }
    void Net::stop()
    {
        NyLPC_cNet_stop(&(this->_inst));
        //stop mDNS
        if(this->_mdns!=NULL){
            NyLPC_cMDnsServer_finalize(this->_mdns);        
            free(this->_mdns);
            this->_mdns=NULL;
        }
    }
    
}