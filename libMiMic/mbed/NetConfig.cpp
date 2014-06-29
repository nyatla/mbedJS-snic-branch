#include "NetConfig.h"
#include "NyLPC_uip.h"
#include "NyLPC_flash.h"
#include "NyLPC_uip_ethernet.h"
#include "utils/PlatformInfo.h"
#include <stdio.h>
#include <stdlib.h>
#include "mbed.h"

/**
 * Default UPnP Description
 */
const static char* UPNP_DEVICE_TYPE ="urn:schemas-upnp-org:device:Basic:1";
const static char* UPNP_DEFAULT_FRENDLY_NAME="MiMic Device";
const static char* UPNP_DEFAULT_MANUFACTUR_NAME="nyatla.jp";
const static char* UPNP_DEFAULT_MANUFACTUR_URL="http://nyatla.jp/";
const static char* UPNP_DEFAULT_MODEL_NAME="MiMic UPnP device.";
const static char* STR_EMPTY="";
const static char* UPNP_DEFAULT_PRESENTATION_URL="/";
const static char* UPNP_DEFAULT_ICON_MIMETYPE="image/png";
const static char* UPNP_DEFAULT_ICON_URL="./icon.png";


/*
setUPnPFriendlyName
setUPnPManufactur(name,url,description)
setUPnPModel(name,number,url)
setUPnPPresentationUrl(name,number,url)
*/


//default mdns records
const static char* MDNS_SRV_HTTP="_http._tcp\0";
//const static char* MDNS_NAME="LPC176x(MiMic)\0";

extern "C" void mbed_mac_address(char *s);


static void overrideMacAddrIfmbed(NyLPC_TcNetConfig& v)
{
    mbed_mac_address((char*)(v.super.eth_mac.addr));
    //update default hostname
    strcpy(v.hostname,"MiMic");
    NyLPC_uitoa2(NyLPC_TcIPv4Config_getEtherMac000120203(&(v.super)),v.hostname+ 5,16,8);
    NyLPC_uitoa2((NyLPC_TcIPv4Config_getEtherMac0405xxxx(&(v.super))>>16)&0xffff,v.hostname+13,16,4);
    v.services.flags=NyLPC_TcNetConfig_SERVICE_FLAG_MDNS|NyLPC_TcNetConfig_SERVICE_FLAG_UPNP;
}

static void updateOnchipConfig(NyLPC_TcNetConfig& v)
{
    struct NyLPC_TMiMicConfigulation cfg_image;
    //パラメータ→ROMイメージ変換
    cfg_image.fast_boot=0xffffffff;
    cfg_image.mac_00_01_02_03=NyLPC_TcIPv4Config_getEtherMac000120203(&(v.super));
    cfg_image.mac_04_05_xx_xx=NyLPC_TcIPv4Config_getEtherMac0405xxxx(&(v.super));
    cfg_image.ipv4_addr_net  =NyLPC_ntohl(v.super.ip_addr.v);
    cfg_image.ipv4_mask_net  =NyLPC_ntohl(v.super.netmask.v);
    cfg_image.ipv4_drut_net  =NyLPC_ntohl(v.super.dr_addr.v);
    //additional information
    cfg_image.ipv4_flags=v.tcp_mode;
    cfg_image.http_port=v.services.http_port;
    cfg_image.srv_flags=v.services.flags;
    strcpy(cfg_image.hostname,v.hostname);
    //FreeRTOSの停止
    NyLPC_cIsr_enterCritical();
    //Flashへの書き込み
    NyLPC_cMiMicConfiglation_updateConfigulation(&cfg_image);
    //FreeRTOSの復帰
    NyLPC_cIsr_exitCritical();
}


namespace MiMic
{
NetConfig::NetConfig(bool i_is_factory_default)
{
    NyLPC_cNetConfig_initialize(&(this->_inst),i_is_factory_default);
    this->_ref_custom_dns_record=NULL;
    this->_ref_custom_upnp_desc=NULL;

    //check mbed
    if(PlatformInfo::getPlatformType()!=PlatformInfo::PF_MBED) {
        return;
    }

    if(!NyLPC_cMiMicConfiglation_hasUserConfigulation()) {
        //is 1st read?
        //mbed override
        overrideMacAddrIfmbed((this->_inst));
        //save
        updateOnchipConfig((this->_inst));
    } else {
        //2nd read
        if(i_is_factory_default) {
            //mbed override
            overrideMacAddrIfmbed((this->_inst));
        } else {
            //nothing to do
        }
    }
    //updateUUID
    this->setUPnPUdn(0xe29f7100,0x4ba2,0x01e0,0);
    this->_upnp_desc.device_type=UPNP_DEVICE_TYPE;
    this->_upnp_desc.frendly_name=UPNP_DEFAULT_FRENDLY_NAME;
    this->_upnp_desc.manufacturer=UPNP_DEFAULT_MANUFACTUR_NAME;
    this->_upnp_desc.manufacturer_url=UPNP_DEFAULT_MANUFACTUR_URL;
    this->_upnp_desc.model_descriprion=STR_EMPTY;
    this->_upnp_desc.model_name=UPNP_DEFAULT_MODEL_NAME;
    this->_upnp_desc.model_number=NULL;
    this->_upnp_desc.model_url=NULL;
    this->_upnp_desc.serial_number=STR_EMPTY;
    this->_upnp_desc.udn=this->_udn;//pointer
    this->_upnp_desc.upc=NULL;
    this->_upnp_icon.width=32;
    this->_upnp_icon.height=32;
    this->_upnp_icon.depth=8;
    this->_upnp_icon.mimetype=UPNP_DEFAULT_ICON_MIMETYPE;
    this->_upnp_icon.url=UPNP_DEFAULT_ICON_URL;
    this->_upnp_desc.icons=&this->_upnp_icon;//pointer
    this->_upnp_desc.presentation_url=UPNP_DEFAULT_PRESENTATION_URL;
    this->_upnp_desc.number_of_devices=0;
    this->_upnp_desc.number_of_service=0;
    this->_upnp_desc.number_of_icon=1;
    this->_upnp_desc.devices=NULL;
    this->_upnp_desc.services=NULL;
    //mdns
    this->_dns_record.name=UPNP_DEFAULT_FRENDLY_NAME;
    this->_dns_record.a=this->_inst.hostname;//pointer
    this->_dns_record.num_of_srv=1;
    this->_srv_record.protocol=MDNS_SRV_HTTP;
    this->_srv_record.port=this->_inst.services.http_port;
    this->_dns_record.srv=&(this->_srv_record);

}

NetConfig::~NetConfig()
{
    NyLPC_cNetConfig_finalize(&(this->_inst));
}

const struct NyLPC_TUPnPDevDescDevice* NetConfig::refUPnPDevDesc()const
{
    return this->_ref_custom_upnp_desc!=NULL?this->_ref_custom_upnp_desc:&this->_upnp_desc;
}
/** internal dns record*/
const struct NyLPC_TDnsRecord* NetConfig::refMdnsRecord()const
{
    return this->_ref_custom_dns_record!=NULL?this->_ref_custom_dns_record:&this->_dns_record;
}


void NetConfig::setZeroconf(bool v)
{
    this->_inst.tcp_mode=(v?NyLPC_TcNetConfig_IPV4_FLAG_MODE_APIPA:NyLPC_TcNetConfig_IPV4_FLAG_MODE_MANUAL);
}
/**
  * Set IPv4 ip address to instance.
  */
void NetConfig::setIpAddr(unsigned char ip1,unsigned char ip2,unsigned char ip3,unsigned char ip4)
{
    NyLPC_TIPv4Addr_set(&(this->_inst.super.ip_addr),ip1,ip2,ip3,ip4);
}

void NetConfig::setIpAddr(const IpAddr& i_addr)
{
    this->_inst.super.ip_addr=i_addr.addr.v4;
}

/**
 * Set IPv4 network mask value to instance.
 */
void NetConfig::setNetMask(unsigned char ip1,unsigned char ip2,unsigned char ip3,unsigned char ip4)
{
    NyLPC_TIPv4Addr_set(&(this->_inst.super.netmask),ip1,ip2,ip3,ip4);
}
void NetConfig::setNetMask(const IpAddr& i_mask)
{
    this->_inst.super.netmask=i_mask.addr.v4;
}

/**
 * Set IPv4 default gateway address to instance.
 */
void NetConfig::setGateway(unsigned char ip1,unsigned char ip2,unsigned char ip3,unsigned char ip4)
{
    NyLPC_TIPv4Addr_set(&(this->_inst.super.dr_addr),ip1,ip2,ip3,ip4);
}
void NetConfig::setGateway(const IpAddr& i_addr)
{
    this->_inst.super.dr_addr=i_addr.addr.v4;
}

/**
 * Set ethernet mac address to instance.
 */
void NetConfig::setEmac(unsigned char a1,unsigned char a2,unsigned char a3,unsigned char a4,unsigned char a5,unsigned char a6)
{
    NyLPC_TEthAddr_set(&(this->_inst.super.eth_mac),a1,a2,a3,a4,a5,a6);
    //update only node field
    for(NyLPC_TInt16 i=0;i<6;i++){
        NyLPC_uitoa2(this->_inst.super.eth_mac.addr[i],&this->_udn[5+24+i*2],16,2);
    }
    
}
void NetConfig::setSrvHttpPort(unsigned short port)
{
    this->_srv_record.port=this->_inst.services.http_port=port;
}
void NetConfig::setSrvMdns(bool i_enable)
{
    if(i_enable) {
        this->_inst.services.flags|=NyLPC_TcNetConfig_SERVICE_FLAG_MDNS;
    } else {
        this->_inst.services.flags&=(~NyLPC_TcNetConfig_SERVICE_FLAG_MDNS);
    }
}
void NetConfig::setSrvUPnP(bool i_enable)
{
    if(i_enable) {
        this->_inst.services.flags|=NyLPC_TcNetConfig_SERVICE_FLAG_UPNP;
    } else {
        this->_inst.services.flags&=(~NyLPC_TcNetConfig_SERVICE_FLAG_UPNP);
    }
}

void NetConfig::setHostName(const char* i_hostname)
{
    this->setHostName(i_hostname,strlen(i_hostname));
}
void NetConfig::setHostName(const char* i_hostname,int len)
{
    int l=(len>(NyLPC_TcNetConfig_HOSTNAME_LEN-1))?NyLPC_TcNetConfig_HOSTNAME_LEN-1:len;
    memcpy(this->_inst.hostname,i_hostname,l);
    *(this->_inst.hostname+l)='\0';
}
void NetConfig::setFriendlyName(const char* i_name)
{
    this->_dns_record.name=this->_upnp_desc.frendly_name=(i_name==NULL?STR_EMPTY:i_name);
}
void NetConfig::setUPnPManufactur(const char* i_name,const char* i_url)
{
    this->_upnp_desc.manufacturer=i_name==NULL?STR_EMPTY:i_name;
    this->_upnp_desc.manufacturer_url=i_name==NULL?STR_EMPTY:i_url;
}
void NetConfig::setUPnPModel(const char* i_name,const char* i_number,const char* i_url,const char* i_description)
{
    this->_upnp_desc.model_descriprion=i_description==NULL?STR_EMPTY:i_description;
    this->_upnp_desc.model_name=i_name==NULL?STR_EMPTY:i_name;
    this->_upnp_desc.model_number=i_number;
    this->_upnp_desc.model_url=i_url;
}
void NetConfig::setUPnPSerialNumber(const char* i_number)
{
    this->_upnp_desc.serial_number=i_number;
}
void NetConfig::setUPnPUdn(unsigned long i_time_l,unsigned short i_time_m,unsigned short i_time_h,unsigned short i_sq)
{
    NyLPC_TcUuid_t uuid;
    NyLPC_cUuid_initialize(&uuid);
    NyLPC_cUuid_setTimeBase(&uuid,i_time_l,(i_time_h<<16)|i_time_m,i_sq,&(this->_inst.super.eth_mac));
    strcpy(this->_udn,"uuid:");
    NyLPC_cUuid_toString(&uuid,this->_udn+5);
    NyLPC_cUuid_finalize(&uuid);
}
void NetConfig::setUPnPIcon(unsigned short i_width,unsigned short i_height,unsigned short i_depth,const char* i_mimetype,const char* i_url)
{
    this->_upnp_icon.width=i_width;
    this->_upnp_icon.height=i_height;
    this->_upnp_icon.depth=i_depth;
    this->_upnp_icon.mimetype=i_mimetype;
    this->_upnp_icon.url=i_url;    
}

void NetConfig::setUPnPPresentationURL(const char* i_url)
{
    this->_upnp_desc.presentation_url=i_url;
}
void NetConfig::setCustomUPnPDescription(const struct NyLPC_TUPnPDevDescDevice* i_ref_description)
{
    this->_ref_custom_upnp_desc=i_ref_description;
}
void NetConfig::setCustomMdnsRecord(const struct NyLPC_TDnsRecord* i_ref_record)
{
    this->_ref_custom_dns_record=i_ref_record;
}

bool NetConfig::loadFromFile(const char* i_file)
{
#define NUMBER_OF_NAME 8
    const static char* tbl[]= {
        "macaddr",  //0
        "ipaddr",   //1
        "netmask",  //2
        "gateway",  //3
        "srv_http_port",//4
        "srv_mdns",//5
        "srv_upnp"  //6
        "host",//7
    };
    char tmp[32];
    union {
        unsigned char u8[6];
        NyLPC_TUInt32 u32;
    } v;
    const char* p;//pointer to read
    const char* key;
    const char* t;
    int l;
    FILE* fp = fopen(i_file,"r");
    if(fp==NULL) {
        return false;
    }
    //read from values
    while(fgets(tmp,31,fp)) {
        p=NyLPC_cFormatTextReader_seekSpace(tmp)+tmp;//skip space
        l=NyLPC_cFormatTextReader_readWord(p,&key);
        for(int i=0; i<NUMBER_OF_NAME; i++) {
            if(l>=0 && NyLPC_strnicmp(key,tbl[i],l)==0) {
                p+=l;//skip keyname
                p=NyLPC_cFormatTextReader_seekSpace(p)+p;//skip space
                if(*p!='=') {
                    break;//check equal
                }
                p++;
                //skip space
                p=NyLPC_cFormatTextReader_seekSpace(p)+p;//skip space
                switch(i) {
                    case 0://macaddr
                        if(NyLPC_cFormatTextReader_readMacAddr(p,v.u8)!=0) {
                            this->setEmac(v.u8[0],v.u8[1],v.u8[2],v.u8[3],v.u8[4],v.u8[5]);
                        }
                        break;
                    case 1://ipaddr
                        if(NyLPC_cFormatTextReader_readIpAddr(p,v.u8)!=0) {
                            this->setIpAddr(v.u8[0],v.u8[1],v.u8[2],v.u8[3]);
                            this->setZeroconf(false);
                        } else {
                            if(NyLPC_cFormatTextReader_readWord(p,&t)==4) {
                                if(NyLPC_strnicmp(t,"auto",4)==0) {
                                    this->setZeroconf(true);
                                }
                            }
                        }
                        break;
                    case 2:
                        if(NyLPC_cFormatTextReader_readIpAddr(p,v.u8)!=0) {
                            this->setNetMask(v.u8[0],v.u8[1],v.u8[2],v.u8[3]);
                        }
                        break;
                    case 3:
                        if(NyLPC_cFormatTextReader_readIpAddr(p,v.u8)!=0) {
                            this->setGateway(v.u8[0],v.u8[1],v.u8[2],v.u8[3]);
                        }
                        break;
                    case 4:
                        if(NyLPC_cFormatTextReader_readUInt(p,&(v.u32))!=0) {
                            this->setSrvHttpPort((unsigned short)v.u32);
                        }
                        break;
                    case 5:
                        l=NyLPC_cFormatTextReader_readWord(p,&t);
                        if((*t=='y' || *t=='Y')) {
                            this->setSrvMdns(true);
                        } else if((*t=='n' || *t=='N')) {
                            this->setSrvMdns(false);
                        }
                        break;
                    case 6:
                        l=NyLPC_cFormatTextReader_readWord(p,&t);
                        if((*t=='y' || *t=='Y')) {
                            this->setSrvUPnP(true);
                        } else if((*t=='n' || *t=='N')) {
                            this->setSrvUPnP(false);
                        }
                        break;
                    case 7:
                        l=NyLPC_cFormatTextReader_readWord(p,&t);
                        if(l>1) {
                            this->setHostName(t,l);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
    fclose(fp);
    return true;
}
};