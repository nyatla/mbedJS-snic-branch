#include "NyLPC_cApipa.h"
#include "../NyLPC_cNetIf.h"
#include <stdio.h>
#include <string.h>


/**
 * ARPテーブルに指定IPが現れるまで待ちます。
 */
static NyLPC_TBool waitForArpResponse(const struct NyLPC_TIPv4Addr* i_ip,NyLPC_TUInt32 i_wait_in_ms)
{
    NyLPC_TcStopwatch_t sw;
    NyLPC_cStopwatch_initialize(&sw);
    NyLPC_cStopwatch_startExpire(&sw,i_wait_in_ms);
    while(!NyLPC_cStopwatch_isExpired(&sw)){
        NyLPC_cThread_yield();
        if(NyLPC_cNetIf_hasArpInfo(i_ip)){
            return NyLPC_TBool_TRUE;
        }
    }
    NyLPC_cStopwatch_finalize(&sw);
    return NyLPC_TBool_FALSE;
}
static void makeIP(NyLPC_TcApipa_t* i_inst,struct NyLPC_TIPv4Addr* i_ip)
{
//  NyLPC_TIPv4Addr_set(i_ip,192,168,128,206);//for conflict test!
    NyLPC_TIPv4Addr_set(i_ip,169,254,(i_inst->_seed>>8)&0xff,i_inst->_seed & 0xff);
}
static void updateSeed(NyLPC_TcApipa_t* i_inst)
{
    do{
        i_inst->_seed=(391*i_inst->_seed+392);
    }while(((i_inst->_seed & 0xff)==0) || ((i_inst->_seed & 0xff00)==0));
}



void NyLPC_cApipa_initialize(NyLPC_TcApipa_t* i_inst)
{
    i_inst->_seed=0;
}


/**
 * この関数はuipを操作します。
 * cNetは停止中である必要があります。
 */
NyLPC_TBool NyLPC_cApipa_requestAddr(NyLPC_TcApipa_t* i_inst,NyLPC_TcIPv4Config_t* i_cfg,NyLPC_TInt16 i_repeat)
{
    int i;
    NyLPC_TcIPv4Config_t cfg;
    struct NyLPC_TIPv4Addr caip;

    //ゼロコンフィギュレーション用のIPを設定
    NyLPC_cIPv4Config_initialzeCopy(&cfg,i_cfg);
    //seedを更新
    for(i=0;i<6;i++)
    {
        i_inst->_seed+=i_cfg->eth_mac.addr[i];
    }
    NyLPC_cIPv4Config_setDefaultRoute(&cfg,&NyLPC_TIPv4Addr_ZERO);
    for(i=i_repeat-1;i>=0;i--){
        NyLPC_cIPv4Config_setIp(&cfg,&NyLPC_TIPv4Addr_ZERO,&NyLPC_TIPv4Addr_ZERO);
        updateSeed(i_inst);
        makeIP(i_inst,&caip);
        //startInterface
        NyLPC_cNetIf_start(&cfg);
        NyLPC_cNetIf_sendArpRequest(&caip);
        //テーブル更新待ち
        if(waitForArpResponse(&caip,512+(i_inst->_seed % 256))){
        	NyLPC_cNetIf_stop();
            continue;
        }
        NyLPC_cNetIf_stop();
        //IPのコンフリクトテスト
        NyLPC_cIPv4Config_setIp(&cfg,&caip,&NyLPC_TIPv4Addr_APIPA_MASK);
        NyLPC_cNetIf_start(&cfg);
        //!ARP送信
        NyLPC_cNetIf_sendArpRequest(&caip);
        if(waitForArpResponse(&caip,512+(256-(i_inst->_seed % 256)))){
            //応答があったらエラー
        	 NyLPC_cNetIf_stop();
            continue;
        }
        //OK
        NyLPC_cNetIf_stop();
        NyLPC_cIPv4Config_setIp(i_cfg,&cfg.ip_addr,&cfg.netmask);
        return NyLPC_TBool_TRUE;
    }
    return NyLPC_TBool_FALSE;
}
