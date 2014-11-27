#include "mbed.h"
#include "rtos.h"
#include "SDFileSystem.h"
#include "mimic.h"
#include "fsdata.h"
#include "rpctbl.h"

#include "netif/snicip/NyLPC_cSnicNetIf_protected.h"
#include "netif/snicip/NyLPC_cSnicGeneral.h"
#include "netif/snicip/NyLPC_cSnicWifi.h"

#define NYLPC_TiSnicDevice_H_

#define DEMO_AP_SSID                  "nyanyatlanet"
#define DEMO_AP_SECURITY_TYPE         SNIC_WIFI_SECURITY_WPA2_MIXED_PSK
#define DEMO_AP_SECUTIRY_KEY          "xLaTlA19781003x"
int main()
{
	NyLPC_TcSnicGeneral_t gen;
	NyLPC_TcSnicWifi_t wifi;
	char name[64];
	NyLPC_cSonicNetIf_initialize();
//	NyLPC_cSnicNetIf_start();
	NyLPC_cSnicGeneral_initialize(&gen);
	NyLPC_cSnicGeneral_getFirmwareVersionInfo(&gen,name,64);
	NyLPC_cSnicWifi_initialize(&wifi);
	NyLPC_cSnicWifi_off(&wifi);
	NyLPC_cSnicWifi_on(&wifi,NyLPC_Snic_TCuntryCode_JAPAN);
	NyLPC_cSnicWifi_disconnect(&wifi);
	NyLPC_cSnicWifi_join(&wifi,DEMO_AP_SSID,strlen(DEMO_AP_SSID),DEMO_AP_SECURITY_TYPE,DEMO_AP_SECUTIRY_KEY,strlen(DEMO_AP_SECUTIRY_KEY));
	NyLPC_cSnicGeneral_finalize(&gen);


	NyLPC_cSonicNetIf_finalize();
}













#ifdef COM
/**
 * local filesystem support.
 * MiMic::LocalFileSystem2 do not freeze on LPCXpresso.
 */
//LocalFileSystem2 lf("local");
//SDFileSystem sd(p5, p6, p7, p8,"sd");

Net* net;


static int memory_size=0;
/**
 * MiMic RemoteMCU httpd.<br/>
 * <p>Service list</p>
 * <pre>
 * /rom/ - romfs
 * /setup/ - MiMic configulation REST API.
 * /local/ - mbed LocalFileSystem
 * /mvm/   - MiMicVM REST API
 * </pre>
 */
class MiMicRemoteMcu:public MiMic::Httpd
{
private:
    ModRomFiles modromfs; //ROM file module
    ModMiMicSetting mimicsetting; //mimic setting API
    ModLocalFileSystem modlocal; //FileSystem mounter
    ModLocalFileSystem modsd; //FileSystem mounter
    ModFileIo modfio;   //fileupload API
    ModUPnPDevice modupnp;
    ModJsonRpc modrpc;
public:
    MiMicRemoteMcu(NetConfig& i_cfg):Httpd(i_cfg.getHttpPort())
    {
        this->modromfs.setParam("rom",RMCU_FSDATA,0);
        this->mimicsetting.setParam("setup");
        this->modlocal.setParam("local");
        this->modsd.setParam("sd",ModLocalFileSystem::FST_SDFATFS);
        this->modfio.setParam("fio");
        this->modupnp.setParam(*net);
        this->modrpc.setParam("rpc",RPCTBL);
    }
    /**
     * Http handler
     */
    virtual void onRequest(HttpdConnection& i_connection)
    {
		for(int i=0;i<10000;i+=4){
			void* m=malloc(i);
			if(m){
				free(m);
			}else{
				memory_size=i;
				break;
			}
		}
        //pause persistent mode if websocket ready.
        if(this->modrpc.isStarted()){
            i_connection.breakPersistentConnection();
        }
        //try to ModRomFS module.
        if(this->modromfs.execute(i_connection)){
            return;
        }
        //try to ModMiMicSetting module.
        if(this->mimicsetting.execute(i_connection)){
            return;
        }
        //try to ModLocalFileSystem
        if(this->modlocal.execute(i_connection)){
            return;
        }
        //try to ModLocalFileSystem(SD)
        if(this->modsd.execute(i_connection)){
            return;
        }
        //try to FileUpload
        if(this->modfio.execute(i_connection)){
            return;
        }
        //try to UPnP
        if(this->modupnp.execute(i_connection)){
            return;
        }
        if(this->modrpc.execute(i_connection)){
            this->modrpc.dispatchRpc();
            return;
        }
        
        //Otherwise, Send the redirect response to /rom/index.html
        i_connection.sendHeader(302,
            "text/html",
            "Status: 302:Moved Temporarily\r\n"
            "Location: /rom/index.html\r\n");        
        return;
    }
};
MiMicRemoteMcu* httpd;
NetConfig cfg; //create network configulation  with onchip-setting.
int main()
{
	for(int i=0;i<100000;i+=4){
		void* m=malloc(i);
		if(m){
			free(m);
		}else{
			memory_size=i;
			break;
		}
	}
    net=new Net();//Net constructor must be created after started RTOS
    //Prepare configulation.
    cfg.setUPnPIcon(64,64,8,"image/png","/rom/icon.png");
    cfg.setUPnPUdn(0xe29f7101,0x4ba2,0x01e0,0);
    cfg.setFriendlyName("mbedJS");
    cfg.setUPnPPresentationURL("/rom/index.html");
    cfg.setIpAddr(192,168,128,39);
    cfg.setZeroconf(true);//setIpAddr(192,168,128,39);

/*
    //try to override setting by local file.
    if(!cfg.loadFromFile("/local/mimic.cfg")){
        Thread::wait(2000);//wait for SD card initialization.
        cfg.loadFromFile("/sd/mimic.cfg");
    }
 */
    httpd=new MiMicRemoteMcu(cfg); //create a httpd instance.
    net->start(cfg);
	for(int i=0;i<100000;i+=4){
		void* m=malloc(i);
		if(m){
			free(m);
		}else{
			memory_size=i;
			break;
		}
	}
    httpd->loop();  //start httpd loop.
    return 0;
}
#endif
