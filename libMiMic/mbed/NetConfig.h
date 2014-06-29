#pragma once
////////////////////////////////////////////////////////////////////////////////
// NetConfig.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"
#include "IpAddr.h"


namespace MiMic
{
    class IpAddr;
    /**
     * Network configulation class.
     * The class is used by Net constructor.
     */
    class NetConfig
    {
    private:
        /**service record*/
        NyLPC_TcNetConfig_t _inst;
        struct NyLPC_TMDnsServiceRecord _srv_record;
        void _syncServiceInfo();
        struct NyLPC_TDnsRecord _dns_record;
        struct NyLPC_TUPnPDevDescDevice _upnp_desc;
        struct NyLPC_TUPnPDevDescIcon _upnp_icon;
        const struct NyLPC_TDnsRecord* _ref_custom_dns_record;
        const struct NyLPC_TUPnPDevDescDevice* _ref_custom_upnp_desc;
        char _udn[42];//uuid:00000000-0000-0000-0000-000000000000+'\0'
    public:
        /** wrapped base LPC class.*/
        NyLPC_TcNetConfig_t* refBaseInstance(){return &this->_inst;}
        /** internal UPnP description*/
        const struct NyLPC_TUPnPDevDescDevice* refUPnPDevDesc()const;
        /** internal dns record*/
        const struct NyLPC_TDnsRecord* refMdnsRecord()const;
        /** Thi sfunction returnes recommended HTTP service port number.*/
        unsigned short getHttpPort()const{return this->_inst.services.http_port;}
    public:
        /**
         * The constructor.
         * This function initializes instance by onchip configulation data.
         * @param i_is_factory_default
         * Factory default flag.
         * <ul>
         * <li>true - The function sets factory default setting.
         * <li>false -The function sets onchip memory configulation (default)
         * </ul>
         */
        NetConfig(bool i_is_factory_default=false);
        virtual ~NetConfig();        
        /**
         * Set IPv4 ip address to instance.
         */
        void setIpAddr(unsigned char ip4,unsigned char ip3,unsigned char ip2,unsigned char ip1);
        void setIpAddr(const IpAddr& i_addr);
        /**
         * Set IPv4 network mask value to instance.
         */
        void setNetMask(unsigned char ip4,unsigned char ip3,unsigned char ip2,unsigned char ip1);
        void setNetMask(const IpAddr& i_mask);
        /**
         * Set IPv4 default gateway address to instance.
         */
        void setGateway(unsigned char ip4,unsigned char ip3,unsigned char ip2,unsigned char ip1);
        void setGateway(const IpAddr& i_addr);
        /**
         * Set Zero configuration enable flag.
         * @param v
         * True, Zero configuration mode. The mimic will try DHCP and AutoIP configuration.
         * ipaddress,netmask,gateway are ignored.
         * False, Manual mode. The mimic will set ip address from on chip data.
         */
        void setZeroconf(bool v);
        /**
         * This function set a recommended HTTP port number to the application.
         */
        void setSrvHttpPort(unsigned short port);
        /**
         * Set mDNS operation flag.
         * This function recommends to the application to provide mDNS service to network.
         */
        void setSrvMdns(bool i_enable);
        /**
         * Set UPnP operation flag.
         * This function recommends to the application to provide UPnP service to network.
         */
        void setSrvUPnP(bool i_enable);
        /**
         * This function sets host name for mDNS.
         * This value has effect to the mDNS a record.
         * @param i_hostname
         * host name.
         * NULL terminated string.
         */
        void setHostName(const char* i_hostname);
        /**
         * This function sets host name for mDNS.
         * This is effective for mDNS a record.
         * @param i_hostname
         * host name.
         * @param i_len
         * length of host name.
         * maximum length is NyLPC_TcNetConfig_HOSTNAME_LEN-1
         */
        void setHostName(const char* i_hostname,int len);
        /**
         * Set ethernet mac address to instance.
         */
        void setEmac(unsigned char ip6,unsigned char ip5,unsigned char ip4,unsigned char ip3,unsigned char ip2,unsigned char ip1);
        /**
         * Set FrendlyName to instance.
         * This value has effect to the mDNS name and UPnP frendlyName.
         */
        void setFriendlyName(const char* i_name);
        void setUPnPManufactur(const char* i_name,const char* i_url);
        void setUPnPModel(const char* i_name,const char* i_number,const char* i_url,const char* i_description);
        void setUPnPSerialNumber(const char* i_number);
        void setUPnPPresentationURL(const char* i_url);
        
        /**
         * Set Time base UDN value.
         * UDN is expressed as follows.
         * The value of the UDN, must be different for each device.
         * [i_time_l]-[i_time_m]-[i_time_h|0x80]-[i_sq&0x3F|0x80]-[MAC ADDRESS]
         */
        void setUPnPUdn(unsigned long i_time_l,unsigned short i_time_m,unsigned short i_time_h,unsigned short i_sq);
        void setUPnPIcon(unsigned short i_width,unsigned short i_height,unsigned short i_depth,const char* i_mimetype,const char* i_url);
        /**
         * This function sets a custom UPnP device description to instance.
         * All UPnP description will be overwritten if set it.
         * @param i_ref_description
         * Perfect description structure.
         */
        void setCustomUPnPDescription(const struct NyLPC_TUPnPDevDescDevice* i_ref_description);
        /**
         * This function sets a custom mDNS record to instance.
         * All mDNS record will be overwritten if set it.
         * @param i_ref_record
         * Perfect DNS record structure.
         */
        void setCustomMdnsRecord(const struct NyLPC_TDnsRecord* i_ref_record);
        
        
        
        /**
         * Load configulation from text file.
         * <p>File format example
         * <pre>
         * macaddr=00:00:00:00:00:00
         * host=MiMic01
         * ipaddr=192.168.0.1
         * netmask=255.255.255.0
         * gateway=192.168.0.254
         * srv_http_port=80
         * srv_mdns=yes
         * </pre>
         * <p>Keys
         * <ul>
         * <li>macaddr=[:macaddr:] - 48bit ethernet mac address that are separated by ':'</li>
         * <li>ipaddr=[:ip:] || AUTO
         * - 32 bit IP address or auto detection. AUTO will be to try DHCP and AUTOIP.
         * </li>
         * <li>netmask=[:ip:]- 32 bit network mask value.</li>
         * <li>gateway=[:ip:] - 32 bit default gateway address.</li>
         * <li>srv_http_port=[:UINT16:] - 16bit http service port number(not ZERO)</li>
         * <li>srv_mdns=[yes|no] - mDNS service flag.</li>
         * </ul>
         * </p>
         * <p>Default setting
         * <ul>
         * <li>macaddr=02:01:02:03:04:05 (In case of mbed it is preset value.)
         * <li>host=MiMic020102030405
         * <li>ipaddr =192.168.0.39
         * <li>netmask=255.255.255.0
         * <li>gateway=192.168.0.254
         * <li>srv_http_port=80
         * <li>srv_mdns=yes</li>
         * </ul>
         * </p>
         * Maximum line length is 31.
         * Specified values are override on-chip setting value.
         * If the same value appeared, then the last one is enabled.
         * In case of ipaddr=AUTO, gateway and netmask are ignored.
         * </p>
         * @return
         * true if file read. false is not read.
         */
         bool loadFromFile(const char* i_file);
   };
}