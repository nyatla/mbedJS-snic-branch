/*********************************************************************************
 * PROJECT: MiMic
 * --------------------------------------------------------------------------------
 *
 * This file is part of MiMic
 * Copyright (C)2011 Ryo Iizuka
 *
 * MiMic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by　the Free Software Foundation, either version 3 of the　License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information please contact.
 *  http://nyatla.jp/
 *  <airmail(at)ebony.plala.or.jp> or <nyatla(at)nyatla.jp>
 *
 *********************************************************************************/
#include "NyLPC_cNetConfig.h"
#include "../flash/NyLPC_cMiMicConfiglation.h"


static void update(NyLPC_TcNetConfig_t* i_inst,const struct NyLPC_TMiMicConfigulation* pdata);


void NyLPC_cNetConfig_initialize(NyLPC_TcNetConfig_t* i_inst,NyLPC_TBool i_is_factory_default)
{
    if(i_is_factory_default)
    {
        update(i_inst,NyLPC_cMiMicConfiglation_loadFactoryDefault());
    }else{
        update(i_inst,NyLPC_cMiMicConfiglation_loadFromFlash());
    }
}


/**
 * ユーザコンフィギュレーションを更新する。
 * この関数をコールするときは、割込/FreeRTOSを一時停止すること。
 */
/*
NyLPC_TBool NyLPC_cNetConfig_saveToOnchipFlash(NyLPC_TcNetConfig_t* i_inst)
{
    NyLPC_TUInt8* pdata;
    NyLPC_TcOnchipFlashWriter_t s;
    struct TNetConfigMemMap tmp;
    NyLPC_cOnchipFlashWriter_initialize(&s);
    //書込みデータを作成
    tmp.fast_boot=0xffffffff;
    tmp.version  =i_inst->version;
    tmp.interface_type=NyLPC_cNetConfig_INTERFACE_TYPE_ETHERNET;
    pdata=i_inst->interface_setting.ethernet.eth_mac.addr;
    tmp.mac_00_01_02_03=(((NyLPC_TUInt32)pdata[0])<<24)|(((NyLPC_TUInt32)pdata[1])<<16)|(((NyLPC_TUInt32)pdata[2])<<8)|(((NyLPC_TUInt32)pdata[3])<<0);
    tmp.mac_04_05_xx_xx=(((NyLPC_TUInt32)pdata[4])<<24)|(((NyLPC_TUInt32)pdata[5])<<16);
    tmp.ipv4_addr_net=i_inst->interface_setting.ethernet.ip_addr.v;
    tmp.ipv4_mask_net=i_inst->interface_setting.ethernet.netmask.v;
    tmp.ipv4_drut_net=i_inst->interface_setting.ethernet.dr_addr.v;

    //イレース
    if(!NyLPC_cOnchipFlashWriter_elase(&s,MIMIC_CONFIGLATION_FLASH_SECTOR,MIMIC_CONFIGLATION_FLASH_SECTOR)){
        NyLPC_OnErrorGoto(Error);
    }
    //コンフィギュレーションを書き込む。
    if(!NyLPC_cOnchipFlashWriter_writeSector(&s,MIMIC_CONFIGLATION_FLASH_SECTOR,0x00000000,&tmp,sizeof(struct TNetConfigMemMap))){
        NyLPC_OnErrorGoto(Error);
    }
    //ユーザコンフィギュレーションをONにする。
    if(setUserConfigulation()){
        NyLPC_OnErrorGoto(Error);
    }
    NyLPC_cOnchipFlashWriter_finalize(&s);
    return NyLPC_TBool_TRUE;
Error:
    NyLPC_cOnchipFlashWriter_finalize(&s);
    return NyLPC_TBool_FALSE;
}
*/


/**
 * ETHERNET NETWORKのみなら 1480
 * インターネットを通過するなら 1400程度が妥当。
 */
//#define ETHERNET_FRAME_LEN 1480
#define ETHERNET_FRAME_LEN 1400
static void update(NyLPC_TcNetConfig_t* i_inst,const struct NyLPC_TMiMicConfigulation* pdata)
{
    struct NyLPC_TEthAddr ea;
    struct NyLPC_TIPv4Addr ip,mask,drt;
    //値の読み出し
    ea.addr[0]=(NyLPC_TUInt8)((pdata->mac_00_01_02_03>>24)&0xff);
    ea.addr[1]=(NyLPC_TUInt8)((pdata->mac_00_01_02_03>>16)&0xff);
    ea.addr[2]=(NyLPC_TUInt8)((pdata->mac_00_01_02_03>> 8)&0xff);
    ea.addr[3]=(NyLPC_TUInt8)((pdata->mac_00_01_02_03>> 0)&0xff);
    ea.addr[4]=(NyLPC_TUInt8)((pdata->mac_04_05_xx_xx>>24)&0xff);
    ea.addr[5]=(NyLPC_TUInt8)((pdata->mac_04_05_xx_xx>>16)&0xff);
    ip.v=NyLPC_htonl(pdata->ipv4_addr_net);
    mask.v=NyLPC_htonl(pdata->ipv4_mask_net);
    drt.v=NyLPC_htonl(pdata->ipv4_drut_net);
    strcpy(i_inst->hostname,pdata->hostname);
    NyLPC_cIPv4Config_initialzeForEthernet(&i_inst->super,&ea,ETHERNET_FRAME_LEN);
    NyLPC_cIPv4Config_setDefaultRoute(&i_inst->super,&drt);
    NyLPC_cIPv4Config_setIp(&i_inst->super,&ip,&mask);
    i_inst->services.flags=pdata->srv_flags;
    i_inst->services.http_port=pdata->http_port;
    i_inst->tcp_mode=pdata->ipv4_flags;
    return;
}



