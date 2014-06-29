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
 *
 * Parts of this file were leveraged from uIP:
 *
 * Copyright (c) 2001-2003, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "NyLPC_cIPv4Arp.h"
#include "NyLPC_uip.h"
#include "NyLPC_cUipService_protected.h"
#include <string.h>


/**
 * The maxium age of ARP table entries measured in 10ths of seconds.
 *
 * An UIP_ARP_MAXAGE of 120 corresponds to 20 minutes (BSD
 * default).
 */
#define UIP_ARP_MAXAGE 120


//static const struct NyLPC_TEthAddr broadcast_ethaddr = { { 0xff, 0xff, 0xff,0xff, 0xff, 0xff } };
//static const struct NyLPC_TIPv4Addr broadcast_ipaddr = { 0xfffffff };




static void uip_arp_update(NyLPC_TcIPv4Arp_t* i_inst,const struct NyLPC_TIPv4Addr* ipaddr,const struct NyLPC_TEthAddr *ethaddr);
/*-----------------------------------------------------------------------------------*/
/**
 * Initialize the ARP module.
 *
 */
/*-----------------------------------------------------------------------------------*/
void NyLPC_cIPv4Arp_initialize(NyLPC_TcIPv4Arp_t* i_inst,const NyLPC_TcIPv4Config_t* i_ref_config)
{
    int i;
    struct NyLPC_TArpTableItem* tbl=i_inst->arp_table;
    i_inst->_cfg = i_ref_config;
    i_inst->arptime = 0;
    i_inst->tmpage = 0;
    for (i = 0; i < NyLPC_TcIPv4Arp_ARPTAB_SIZE; ++i) {
        memset(&(tbl[i].ipaddr), 0, sizeof(struct NyLPC_TIPv4Addr));
    }
}
/*-----------------------------------------------------------------------------------*/
/**
 * Periodic ARP processing function.
 *
 * This function performs periodic timer processing in the ARP module
 * and should be called at regular intervals. The recommended interval
 * is 10 seconds between the calls.
 *
 */
/*-----------------------------------------------------------------------------------*/
void NyLPC_cIPv4Arp_periodic(NyLPC_TcIPv4Arp_t* i_inst)
{
    struct NyLPC_TArpTableItem* tbl=i_inst->arp_table;
    struct NyLPC_TArpTableItem* tabptr;
    int i;
    i_inst->arptime++;
    for (i = 0; i < NyLPC_TcIPv4Arp_ARPTAB_SIZE; ++i) {
        tabptr = &tbl[i];
        if (tabptr->ipaddr.v != 0 && i_inst->arptime - tabptr->time >= UIP_ARP_MAXAGE)
        {
            tabptr->ipaddr.v = 0;
        }
    }

}
/*-----------------------------------------------------------------------------------*/
/**
 * ARP processing for incoming IP packets
 *
 * This function should be called by the device driver when an IP
 * packet has been received. The function will check if the address is
 * in the ARP cache, and if so the ARP cache entry will be
 * refreshed. If no ARP cache entry was found, a new one is created.
 *
 * This function expects an IP packet with a prepended Ethernet header
 * in the uip_buf[] buffer, and the length of the packet in the global
 * variable uip_len.
 */
/*-----------------------------------------------------------------------------------*/
void NyLPC_cIPv4Arp_incomingIp(NyLPC_TcIPv4Arp_t* i_inst,const struct NyLPC_TEthernetIIHeader* i_eth,struct NyLPC_TIPv4Addr i_ip_src)
{
    //EtherとIPv4の値を読みだす。
    /* Only insert/update an entry if the source IP address of the
     incoming IP packet comes from a host on the local network. */
    if ((i_ip_src.v & i_inst->_cfg->netmask.v) != (i_inst->_cfg->ip_addr.v & i_inst->_cfg->netmask.v)) {
        return;
    }
    uip_arp_update(i_inst,&(i_ip_src), &(i_eth->src));
    return;
}
/**
 * ARP processing for incoming ARP packets.
 *
 * This function should be called by the device driver when an ARP
 * packet has been received. The function will act differently
 * depending on the ARP packet type: if it is a reply for a request
 * that we previously sent out, the ARP cache will be filled in with
 * the values from the ARP reply. If the incoming ARP packet is an ARP
 * request for our IP address, an ARP reply packet is created and put
 * into the uip_buf[] buffer.
 *
 * When the function returns, the value of the global variable uip_len
 * indicates whether the device driver should send out a packet or
 * not. If uip_len is zero, no packet should be sent. If uip_len is
 * non-zero, it contains the length of the outbound packet that is
 * present in the uip_buf[] buffer.
 *
 * This function expects an ARP packet with a prepended Ethernet
 * header in the uip_buf[] buffer, and the length of the packet in the
 * global variable uip_len.
 */


/**
 * ARPパケットの読出し用構造体
 */
struct TArpPacketPtr
{
    struct NyLPC_TEthernetIIHeader header;
    struct NyLPC_TArpHeader arp;
}PACK_STRUCT_END;

/**
 * arpパケットを処理します。
 */
void* NyLPC_cIPv4Arp_rx(NyLPC_TcIPv4Arp_t* i_inst,const struct NyLPC_TArpHeader* i_arp, NyLPC_TUInt16 i_len, NyLPC_TUInt16* o_tx_len)
{
    struct NyLPC_TArpHeader* arp_tx;
    if (i_len < sizeof(struct NyLPC_TArpHeader)) {
        return NULL;
    }
    const NyLPC_TcIPv4Config_t* cfg=i_inst->_cfg;
    switch (i_arp->opcode) {
    case NyLPC_HTONS(ARP_REQUEST):
        /* ARP request. If it asked for our address, we send out a reply. */
        if (NyLPC_TIPv4Addr_isEqual(&(i_arp->dipaddr), &(cfg->ip_addr))) {
            /* First, we register the one who made the request in our ARP
             table, since it is likely that we will do more communication
             with this host in the future. */
            uip_arp_update(i_inst,&(i_arp->sipaddr), &i_arp->shwaddr);
            //イーサネットヘッダもいじくるから
            arp_tx=(struct NyLPC_TArpHeader*)NyLPC_cUipService_allocSysTxBuf();

            /* The reply opcode is 2. */
            arp_tx->hwtype      =i_arp->hwtype;
            arp_tx->protocol    =i_arp->protocol;
            arp_tx->hwlen       =i_arp->hwlen;
            arp_tx->protolen    =i_arp->protolen;
            arp_tx->opcode = NyLPC_HTONS(2);
            memcpy(arp_tx->dhwaddr.addr, i_arp->shwaddr.addr, 6);
            memcpy(arp_tx->shwaddr.addr, cfg->eth_mac.addr, 6);
            arp_tx->dipaddr = i_arp->sipaddr;
            arp_tx->sipaddr = cfg->ip_addr;
            *o_tx_len=NyLPC_TEthernetIIHeader_setArpTx((((struct NyLPC_TEthernetIIHeader*)arp_tx)-1),&(i_inst->_cfg->eth_mac));

//          /* The reply opcode is 2. */
//          i_arp->opcode = NyLPC_HTONS(2);
//
//          memcpy(i_arp->dhwaddr.addr, i_arp->shwaddr.addr, 6);
//          memcpy(i_arp->shwaddr.addr, cfg->eth_mac.addr, 6);
//
//          i_arp->dipaddr = i_arp->sipaddr;
//          i_arp->sipaddr = cfg->ip_addr;
            return arp_tx;
        }
        break;
    case NyLPC_HTONS(ARP_REPLY):
        // ARP reply. We insert or update the ARP table if it was meant for us.
        if (NyLPC_TIPv4Addr_isEqual(&(i_arp->dipaddr),&(cfg->ip_addr))) {
            uip_arp_update(i_inst,&(i_arp->sipaddr), &i_arp->shwaddr);
        }
        break;
    }
    return NULL;
}
/**
 * Prepend Ethernet header to an outbound IP packet and see if we need
 * to send out an ARP request.
 *
 * This function should be called before sending out an IP packet. The
 * function checks the destination IP address of the IP packet to see
 * what Ethernet MAC address that should be used as a destination MAC
 * address on the Ethernet.
 *
 * If the destination IP address is in the local network (determined
 * by logical ANDing of netmask and our IP address), the function
 * checks the ARP cache to see if an entry for the destination IP
 * address is found. If so, an Ethernet header is prepended and the
 * function returns. If no ARP cache entry is found for the
 * destination IP address, the packet in the uip_buf[] is replaced by
 * an ARP request packet for the IP address. The IP packet is dropped
 * and it is assumed that they higher level protocols (e.g., TCP)
 * eventually will retransmit the dropped packet.
 *
 * If the destination IP address is not on the local network, the IP
 * address of the default router is used instead.
 *
 * When the function returns, a packet is present in the uip_buf[]
 * buffer, and the length of the packet is in the global variable
 * uip_len.
 */

/**
 * IPアドレス-MACアドレス交換
 */
const struct NyLPC_TEthAddr* NyLPC_cIPv4Arp_IPv4toEthAddr(NyLPC_TcIPv4Arp_t* i_inst,const struct NyLPC_TIPv4Addr i_ip_addr)
{
    int i;
    struct NyLPC_TArpTableItem *tabptr;
    //ARPテーブルから検索
    for (i = NyLPC_TcIPv4Arp_ARPTAB_SIZE - 1; i >= 0; i--) {
        tabptr = &i_inst->arp_table[i];
        if (NyLPC_TIPv4Addr_isEqual(&i_ip_addr,&(tabptr->ipaddr))) {
            return &tabptr->ethaddr;
        }
    }
    return NULL;
}







static void uip_arp_update(NyLPC_TcIPv4Arp_t* i_inst,const struct NyLPC_TIPv4Addr* ipaddr,const struct NyLPC_TEthAddr *ethaddr)
{
    register struct NyLPC_TArpTableItem *tabptr;
    int i,c;
    /* Walk through the ARP mapping table and try to find an entry to
     update. If none is found, the IP -> MAC address mapping is
     inserted in the ARP table. */
    for (i = 0; i < NyLPC_TcIPv4Arp_ARPTAB_SIZE; ++i) {
        tabptr = &i_inst->arp_table[i];
        /* Only check those entries that are actually in use. */
        if (tabptr->ipaddr.v != 0) {
            /* Check if the source IP address of the incoming packet matches
             the IP address in this ARP table entry. */
            if (ipaddr->v == tabptr->ipaddr.v) {
                /* An old entry found, update this and return. */
                memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
                tabptr->time = i_inst->arptime;

                return;
            }
        }
    }

    /* If we get here, no existing ARP table entry was found, so we
     create one. */
    /* First, we try to find an unused entry in the ARP table. */
    for (i = 0; i < NyLPC_TcIPv4Arp_ARPTAB_SIZE; ++i) {
        tabptr = &i_inst->arp_table[i];
        if (tabptr->ipaddr.v == 0) {
            break;
        }
    }

    /* If no unused entry is found, we try to find the oldest entry and
     throw it away. */
    if (i == NyLPC_TcIPv4Arp_ARPTAB_SIZE) {
        i_inst->tmpage = 0;
        c = 0;
        for (i = 0; i < NyLPC_TcIPv4Arp_ARPTAB_SIZE; ++i) {
            tabptr = &i_inst->arp_table[i];
            if (i_inst->arptime - tabptr->time > i_inst->tmpage) {
                i_inst->tmpage = i_inst->arptime - tabptr->time;
                c = i;
            }
        }
        i = c;
        tabptr = &i_inst->arp_table[i];
    }

    /* Now, i is the ARP table entry which we will fill with the new information. */
    tabptr->ipaddr = *ipaddr;
    memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
    tabptr->time = i_inst->arptime;
}


