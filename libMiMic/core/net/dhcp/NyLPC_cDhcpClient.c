/*********************************************************************************
 * PROJECT: MiMic
 * --------------------------------------------------------------------------------
 *
 * This file is part of MiMic
 * Copyright (C)2011-2013 Ryo Iizuka
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
#include "NyLPC_cDhcpClient.h"
#include <stdio.h>
#include <string.h>

struct NyLPC_TDhcpHeader
{
    NyLPC_TUInt8 op;
    NyLPC_TUInt8 htype;
    NyLPC_TUInt8 hlen;
    NyLPC_TUInt8 hops;
    NyLPC_TUInt32 xid;
    NyLPC_TUInt16 secs;
    NyLPC_TUInt16 flags;
    NyLPC_TUInt32 ciaddr;
    NyLPC_TUInt32 yiaddr;
    NyLPC_TUInt32 siaddr;
    NyLPC_TUInt32 giaddr;
    struct{
        struct NyLPC_TEthAddr emac;
        NyLPC_TChar padding[10];
    }chaddr;
    NyLPC_TChar sname[64];
    NyLPC_TChar file[128];
}PACK_STRUCT_END;

#define NyLPC_TDhcpHeader_BOOTREQUEST 1
#define NyLPC_TDhcpHeader_BOOTREPLY   2

#define DHCP_OPT_ID_ROUTER 3
#define DHCP_OPT_ID_SERVER_ID 54
#define DHCP_OPT_ID_NETMASK 1
#define DHCP_OPT_ID_MESSAGETYPE 53



/**
 * DHCPパケットから32bit値を読み出す。
 * @return
 * ネットワークオーダー
 */
static NyLPC_TBool getUInt32Option(const NyLPC_TUInt8* i_buf,NyLPC_TUInt16 len,NyLPC_TUInt8 i_id,NyLPC_TUInt32* o_v)
{
    const NyLPC_TUInt8* p=i_buf+sizeof(struct NyLPC_TDhcpHeader)+4;
    while(*p!=0x00 && p<(i_buf+len-5)){
        if(*p==i_id){
            if(*(p+1)==4){
                *o_v=*((NyLPC_TUInt32*)(p+2));
                return NyLPC_TBool_TRUE;
            }
        }else{
            p+=(*(p+1))+2;
        }
    }
    return NyLPC_TBool_FALSE;
}
static NyLPC_TBool getUInt8Option(const NyLPC_TUInt8* i_buf,NyLPC_TUInt16 len,NyLPC_TUInt8 i_id,NyLPC_TUInt8* o_v)
{
    const NyLPC_TUInt8* p=i_buf+sizeof(struct NyLPC_TDhcpHeader)+4;
    while(*p!=0x00 && p<(i_buf+len-5)){
        if(*p==i_id){
            if(*(p+1)==1){
                *o_v=*(p+2);
                return NyLPC_TBool_TRUE;
            }
        }else{
            p+=(*(p+1))+2;
        }
    }
    return NyLPC_TBool_FALSE;
}
static NyLPC_TBool NyLPC_TDhcpHeader_parseDHCPOFFER(const NyLPC_TUInt8* i_buf,NyLPC_TUInt16 i_len,NyLPC_TUInt32 i_xid,NyLPC_TcDhcpClient_t* i_inst)
{
    struct NyLPC_TDhcpHeader* p=(struct NyLPC_TDhcpHeader*)i_buf;
    //XIDのチェック
    if(p->xid!=NyLPC_HTONL(i_xid)){
        return NyLPC_TBool_FALSE;
    }
    //OFFERのclient IPアドレスをresultへ保存情報の保存
    i_inst->_result->ip_addr.v=p->yiaddr;
    //SERVER IDを保存
    if(!getUInt32Option(i_buf,i_len,DHCP_OPT_ID_SERVER_ID,&i_inst->_offerserver.v)){
        return NyLPC_TBool_FALSE;
    }
    return NyLPC_TBool_TRUE;
}

static NyLPC_TBool NyLPC_TDhcpHeader_parseDHCPACK(const NyLPC_TUInt8* i_buf,NyLPC_TUInt16 i_len,NyLPC_TUInt32 i_xid,NyLPC_TcIPv4Config_t* result)
{
    struct NyLPC_TDhcpHeader* p=(struct NyLPC_TDhcpHeader*)i_buf;
    //XIDのチェック
    if(p->xid!=NyLPC_HTONL(i_xid)){
        return NyLPC_TBool_FALSE;
    }
    if(!getUInt32Option(i_buf,i_len,DHCP_OPT_ID_ROUTER,&result->dr_addr.v)){
        result->dr_addr=NyLPC_TIPv4Addr_ZERO;
    }
    if(!getUInt32Option(i_buf,i_len,DHCP_OPT_ID_NETMASK,&result->netmask.v)){
        result->netmask=NyLPC_TIPv4Addr_ZERO;
    }
    result->ip_addr.v=p->yiaddr;
    return NyLPC_TBool_TRUE;
}

static void NyLPC_TDhcpHeader_setDHCPDISCOVER(char* i_buf,NyLPC_TUInt32 i_xid,const struct NyLPC_TEthAddr* emac,NyLPC_TUInt16* o_len)
{
    struct NyLPC_TDhcpHeader* p=(struct NyLPC_TDhcpHeader*)i_buf;
    memset(i_buf,0,sizeof(struct NyLPC_TDhcpHeader));
    p->op=NyLPC_TDhcpHeader_BOOTREQUEST;
    p->htype=1;
    p->hlen=6;
    p->xid=NyLPC_HTONL(i_xid);
    p->chaddr.emac=*emac;
    p->flags=NyLPC_HTONS(0x8000);
    memcpy(i_buf+sizeof(struct NyLPC_TDhcpHeader),
        "\x63\x82\x53\x63"      //4
        "\x35\x01\x01"          //3 MESSAGE TYPE
        "\x37\x03\x01\x03\x06"  //5 REQUEST LIST(1,3,6)
        "\x3d\x07\x01\x00\x00\x00\x00\x00\x00" //9 CLIENT INDIFIRE
        "\xff",4+3+5+9+1);
    //emacの上書き
    memcpy((i_buf+sizeof(struct NyLPC_TDhcpHeader)+4+3+5+3),emac->addr,6);
    //送信するパケットの長さ
    *o_len=sizeof(struct NyLPC_TDhcpHeader)+4+3+5+9+1;
    return;
}
static void NyLPC_TDhcpHeader_setDHCPREQUEST(char* i_buf,NyLPC_TUInt32 i_xid,const struct NyLPC_TIPv4Addr* i_sid,const struct NyLPC_TIPv4Addr* i_reqid,const struct NyLPC_TEthAddr* emac,NyLPC_TUInt16* o_len)
{
    struct NyLPC_TDhcpHeader* p=(struct NyLPC_TDhcpHeader*)i_buf;
    memset(i_buf,0,sizeof(struct NyLPC_TDhcpHeader));
    p->op=NyLPC_TDhcpHeader_BOOTREQUEST;
    p->htype=1;
    p->hlen=6;
    p->xid=NyLPC_HTONL(i_xid);
    p->chaddr.emac=*emac;
    p->flags=NyLPC_HTONS(0x8000);
    memcpy(i_buf+sizeof(struct NyLPC_TDhcpHeader),
        "\x63\x82\x53\x63"      //4
        "\x35\x01\x03"          //3 MESSAGE TYPE
        "\x37\x03\x01\x03\x06"  //5 REQUEST LIST(1,3,6)
        "\x3d\x07\x01\x00\x00\x00\x00\x00\x00" //9 CLIENT INDIFIRE
        "\x36\x04\x00\x00\x00\x00" // 6 SERVER ID
        "\x32\x04\x00\x00\x00\x00" // 6 Reqested IP
        "\xff",4+3+5+9+6+6+1);
    //emacの上書き
    memcpy((i_buf+sizeof(struct NyLPC_TDhcpHeader)+4+3+5+3),emac->addr,6);
    //sidの上書き
    memcpy((i_buf+sizeof(struct NyLPC_TDhcpHeader)+4+3+5+9+2),i_sid,4);
    //reqidの上書き
    memcpy((i_buf+sizeof(struct NyLPC_TDhcpHeader)+4+3+5+9+6+2),i_reqid,4);
    //送信するパケットの長さ
    *o_len=sizeof(struct NyLPC_TDhcpHeader)+4+3+5+9+6+6+1;
    return;
}



#define TcDhcpSock_ST_WAIT_OFFER    1
#define TcDhcpSock_ST_WAIT_OFFER_OK 2
#define TcDhcpSock_ST_WAIT_ACK 3
#define TcDhcpSock_ST_WAIT_ACK_OK 4
#define TcDhcpSock_ST_DONE_NG 3
#define TcDhcpSock_ST_DONE_OK 4




#define DHCP_OPT_ID_MESSAGETYPE_ACK   5
#define DHCP_OPT_ID_MESSAGETYPE_OFFER 2

static NyLPC_TBool onPacket(NyLPC_TcUdpSocket_t* i_inst,const void* i_buf,const struct NyLPC_TIPv4RxInfo* i_info);

/**
 * DHCPソケットを作成します。
 */
NyLPC_TBool NyLPC_cDhcpClient_initialize(NyLPC_TcDhcpClient_t* i_inst)
{
    if(!NyLPC_cUdpSocket_initialize(&(i_inst->super),68,NULL,0)){
        return NyLPC_TBool_FALSE;
    }
    NyLPC_cUdpSocket_setBroadcast(&(i_inst->super));
    NyLPC_cUdpSocket_setOnRxHandler(&(i_inst->super),onPacket);
    return NyLPC_TBool_TRUE;
}
void NyLPC_cDhcpClient_finalize(NyLPC_TcDhcpClient_t* i_inst)
{
    NyLPC_cUdpSocket_finalize(&(i_inst->super));
}
#define TIMEOUT_SOCKAPI_MS 1000
#define TIMEOUT_RECVMSG_MS 3000

/**
 * ネットワークを更新します。
 * emac/default_mssを設定したネットワークが必要です。
 */
static NyLPC_TBool NyLPC_cDhcpClient_dhcpRequest(NyLPC_TcDhcpClient_t* i_sock,NyLPC_TcIPv4Config_t* i_result)
{
    char* buf;
    NyLPC_TcStopwatch_t sw;
    NyLPC_TUInt16 s;
    NyLPC_TInt16 hint=sizeof(struct NyLPC_TDhcpHeader)+128;
    i_sock->txid+=(*(NyLPC_TUInt16*)(&(i_result->eth_mac.addr[2])))+(*(NyLPC_TUInt16*)(&(i_result->eth_mac.addr[4])));
    i_sock->_result=i_result;
    buf=NyLPC_cUdpSocket_allocSendBuf(&i_sock->super,hint,&s,TIMEOUT_SOCKAPI_MS);
    if(buf==NULL || s<hint){
        return NyLPC_TBool_FALSE;
    }
    NyLPC_TDhcpHeader_setDHCPDISCOVER(buf,i_sock->txid,&i_sock->_result->eth_mac,&s);
    i_sock->_status=TcDhcpSock_ST_WAIT_OFFER;
    if(!NyLPC_cUdpSocket_psend(&i_sock->super,&NyLPC_TIPv4Addr_BROADCAST,67,buf,s)){
        NyLPC_cUdpSocket_releaseSendBuf(&i_sock->super,buf);
        return NyLPC_TBool_FALSE;
    }
    NyLPC_cStopwatch_initialize(&sw);
    NyLPC_cStopwatch_startExpire(&sw,TIMEOUT_RECVMSG_MS);
    while(i_sock->_status==TcDhcpSock_ST_WAIT_OFFER){
        if(NyLPC_cStopwatch_isExpired(&sw)){
            return NyLPC_TBool_FALSE;
        }
    }
    //レスポンスのチェック
    if(i_sock->_status!=TcDhcpSock_ST_WAIT_OFFER_OK)
    {
        return NyLPC_TBool_FALSE;
    }
    buf=NyLPC_cUdpSocket_allocSendBuf(&i_sock->super,hint,&s,TIMEOUT_SOCKAPI_MS);
    if(buf==NULL || s<hint){
        return NyLPC_TBool_FALSE;
    }
    NyLPC_TDhcpHeader_setDHCPREQUEST(buf,i_sock->txid,&(i_sock->_offerserver),&(i_sock->_result->ip_addr),&i_sock->_result->eth_mac,&s);
    i_sock->_status=TcDhcpSock_ST_WAIT_ACK;
    if(!NyLPC_cUdpSocket_psend(&i_sock->super,&NyLPC_TIPv4Addr_BROADCAST,67,buf,s)){
        NyLPC_cUdpSocket_releaseSendBuf(&i_sock->super,buf);
        return NyLPC_TBool_FALSE;
    }
    NyLPC_cStopwatch_startExpire(&sw,TIMEOUT_RECVMSG_MS);
    while(i_sock->_status==TcDhcpSock_ST_WAIT_ACK){
        if(NyLPC_cStopwatch_isExpired(&sw)){
            return NyLPC_TBool_FALSE;
        }
    }
    //レスポンスのチェック
    if(i_sock->_status!=TcDhcpSock_ST_WAIT_ACK_OK)
    {
        return NyLPC_TBool_FALSE;
    }
    return NyLPC_TBool_TRUE;
}

/**
 * NyLPC_TcIPv4Config_tをDHCPで更新します。
 * この関数をコールする時は、サービスは停止中でなければなりません。
 * @param i_cfg
 * 更新するi_cfg構造体。
 * emac,default_mssは設定済である必要があります。他のフィールド値は不定で構いません。
 * 更新されるフィールドは、ip,netmast,default_rootの3つです。
 * @return
 * 更新に成功した場合TRUE
 */
NyLPC_TBool NyLPC_cDhcpClient_requestAddr(NyLPC_TcDhcpClient_t* i_inst,NyLPC_TcIPv4Config_t* i_cfg,NyLPC_TInt16 i_repeat)
{
    NyLPC_TInt16 i;
    NyLPC_TBool ret=NyLPC_TBool_FALSE;
    NyLPC_TcIPv4Config_t c2;
    //工場出荷時設定でリセットしてIPを0に
    NyLPC_cIPv4Config_initialzeCopy(&c2,i_cfg);
    NyLPC_cIPv4Config_setIp(&c2,&NyLPC_TIPv4Addr_ZERO,&NyLPC_TIPv4Addr_ZERO);
    NyLPC_cIPv4Config_setDefaultRoute(&c2,&NyLPC_TIPv4Addr_ZERO);
    //netを開始
    NyLPC_cUipService_start(&c2);
    for(i=i_repeat-1;i>=0;i--){
        ret=NyLPC_cDhcpClient_dhcpRequest(i_inst,i_cfg);
        if(ret){
            break;
        }
    }
    NyLPC_cUipService_stop();
    NyLPC_cIPv4Config_finalize(&c2);
    return ret;
}



static NyLPC_TBool onPacket(NyLPC_TcUdpSocket_t* i_inst,const void* i_buf,const struct NyLPC_TIPv4RxInfo* i_info)
{
    NyLPC_TUInt8 mt;//message type
    NyLPC_TcDhcpClient_t* inst=(NyLPC_TcDhcpClient_t*)i_inst;
    struct NyLPC_TDhcpHeader* dnsh=(struct NyLPC_TDhcpHeader*)i_buf;
    if(i_info->size<sizeof(struct NyLPC_TDhcpHeader)+1){
        return NyLPC_TBool_FALSE;//DROP
    }
    switch(inst->_status)
    {
    case TcDhcpSock_ST_WAIT_ACK:
        if(dnsh->op!=NyLPC_TDhcpHeader_BOOTREPLY){
            return NyLPC_TBool_FALSE;
            }
        if(!getUInt8Option(i_buf,i_info->size,DHCP_OPT_ID_MESSAGETYPE,&mt)){
            return NyLPC_TBool_FALSE;
        }
        if(mt!=DHCP_OPT_ID_MESSAGETYPE_ACK){
            return NyLPC_TBool_FALSE;
        }
        if(!NyLPC_TDhcpHeader_parseDHCPACK(i_buf,i_info->size,inst->txid,inst->_result)){
            return NyLPC_TBool_FALSE;
        }
        inst->_status=TcDhcpSock_ST_WAIT_ACK_OK;
        break;
    case TcDhcpSock_ST_WAIT_OFFER:
        if(dnsh->op!=NyLPC_TDhcpHeader_BOOTREPLY){
            return NyLPC_TBool_FALSE;
            }
        if(!getUInt8Option(i_buf,i_info->size,DHCP_OPT_ID_MESSAGETYPE,&mt)){
            return NyLPC_TBool_FALSE;
        }
        if(mt!=DHCP_OPT_ID_MESSAGETYPE_OFFER){
            return NyLPC_TBool_FALSE;
        }
        if(!NyLPC_TDhcpHeader_parseDHCPOFFER(i_buf,i_info->size,inst->txid,inst)){
            return NyLPC_TBool_FALSE;
        }
        inst->_status=TcDhcpSock_ST_WAIT_OFFER_OK;
        break;
    }
    return NyLPC_TBool_FALSE;

}
