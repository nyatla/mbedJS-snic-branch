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
#include "NyLPC_uip.h"



const struct NyLPC_TIPv4Addr NyLPC_TIPv4Addr_ZERO={0x00000000};
const struct NyLPC_TIPv4Addr NyLPC_TIPv4Addr_ALL ={0xffffffff};
const struct NyLPC_TIPv4Addr NyLPC_TIPv4Addr_BROADCAST = { 0xffffffff };
const struct NyLPC_TIPv4Addr NyLPC_TIPv4Addr_MULTICAST = NyLPC_TIPv4Addr_pack(224,0,0,0);
const struct NyLPC_TIPv4Addr NyLPC_TIPv4Addr_MULTICAST_MASK = NyLPC_TIPv4Addr_pack(224,0,0,0);
const struct NyLPC_TIPv4Addr NyLPC_TIPv4Addr_APIPA_MASK = NyLPC_TIPv4Addr_pack(255,255,0,0);

NyLPC_TInt16 NyLPC_TIPv4Addr_toString(const struct NyLPC_TIPv4Addr* i_ip,NyLPC_TChar* i_buf)
{
    NyLPC_TUInt32 ip;
    NyLPC_TChar* p=i_buf;
    NyLPC_TUInt8 v;
    NyLPC_TInt8 l;
    //IPをホストオーダーにする。
    ip=NyLPC_NTOHL(i_ip->v);
    for(l=3;l>=0;l--){
        v=(ip>>(8*l))&0xff;
        if(v>=100){
            *p=(v/100)+'0';
            p++;
        }
        if(v>=10){
            *p=((v%100)/10)+'0';
            p++;
        }
        *p=(v%10)+'0';
        *(p+1)='.';
        p+=2;
    }
    *(p-1)='\0';
    return p-i_buf-1;
}


NyLPC_TUInt16 NyLPC_uip_chksum(NyLPC_TUInt16 sum, const NyLPC_TUInt8 *data, NyLPC_TUInt16 len)
{
    NyLPC_TUInt16 t;
    const NyLPC_TUInt8 *dataptr;
    const NyLPC_TUInt8 *last_byte;

    dataptr = data;
    last_byte = data + len - 1;

    while (dataptr < last_byte) { /* At least two more bytes */
        t = (dataptr[0] << 8) + dataptr[1];
        sum += t;
        if (sum < t) {
            sum++; /* carry */
        }
        dataptr += 2;
    }

    if (dataptr == last_byte) {
        t = (dataptr[0] << 8) + 0;
        sum += t;
        if (sum < t) {
            sum++; /* carry */
        }
    }

    /* Return sum in host byte order. */
    return sum;
}

/*--------------------------------------------------------------------------------
 *
 * struct NyLPC_TEthernetIIHeader
 *
 *------------------------------------------------------------------------------*/

/**
 * Ethernetヘッダの内容を、ARPパケットの内容に一致するように書き換えます。
 * i_structの後方にあるものと仮定します。
 * 戻り値は、フレームの長さです。
 */
NyLPC_TUInt16 NyLPC_TEthernetIIHeader_setArpTx(
    struct NyLPC_TEthernetIIHeader* i_struct,
    const struct NyLPC_TEthAddr* i_my_eth_addr)
{
    struct NyLPC_TArpHeader* arph=(struct NyLPC_TArpHeader*)(((NyLPC_TUInt8*)i_struct)+sizeof(struct NyLPC_TEthernetIIHeader));

    i_struct->type = NyLPC_HTONS(NyLPC_TEthernetIIHeader_TYPE_ARP);
    switch(arph->opcode){
    case NyLPC_HTONS(ARP_REPLY):
        memcpy(i_struct->src.addr, i_my_eth_addr->addr, 6);
        memcpy(i_struct->dest.addr, arph->dhwaddr.addr, 6);
        break;
    case NyLPC_HTONS(ARP_REQUEST):
        memset(i_struct->dest.addr, 0xff, 6);
        memcpy(i_struct->src.addr, i_my_eth_addr->addr, 6);
        break;
    }
    return sizeof(struct NyLPC_TEthernetIIHeader)+sizeof(struct NyLPC_TArpHeader);
}

/**
 * イーサネットヘッダをIPv4向けにセットアップする。
 * 関数は、ペイロードをIPv4ヘッダとして、フレームサイズを計算する。
 */
NyLPC_TUInt16 NyLPC_TEthernetIIHeader_setIPv4Tx(
    struct NyLPC_TEthernetIIHeader* i_eth,
    const struct NyLPC_TEthAddr* i_src_eth_addr,
    const struct NyLPC_TEthAddr* i_dest_eth_addr)
{
    struct NyLPC_TIPv4Header* iph=(struct NyLPC_TIPv4Header*)(((NyLPC_TUInt8*)i_eth)+sizeof(struct NyLPC_TEthernetIIHeader));

    i_eth->type = NyLPC_HTONS(NyLPC_TEthernetIIHeader_TYPE_IP);
    /* Build an ethernet header. */
    memcpy(i_eth->dest.addr,i_dest_eth_addr, 6);
    memcpy(i_eth->src.addr, i_src_eth_addr->addr, 6);


    //IPフラグメントに応じたサイズ計算
    switch(iph->proto){
    case UIP_PROTO_TCP:
        return sizeof(struct NyLPC_TEthernetIIHeader)+NyLPC_htons(iph->len16);
    case UIP_PROTO_UDP:
        return sizeof(struct NyLPC_TEthernetIIHeader)+NyLPC_htons(iph->len16);
    case UIP_PROTO_ICMP:
        return sizeof(struct NyLPC_TEthernetIIHeader)+NyLPC_htons(iph->len16);
    }
    return 0;
}
/*--------------------------------------------------------------------------------
 *
 * struct NyLPC_TIPv4Header
 *
 *------------------------------------------------------------------------------*/

/**
 * based on uip_ipchksum
 */
NyLPC_TUInt16 NyLPC_TIPv4Header_makeIpChecksum(const struct NyLPC_TIPv4Header* ip_header)
{
    NyLPC_TUInt16 sum;
    sum = NyLPC_uip_chksum(0, (const NyLPC_TUInt8 *)ip_header,NyLPC_TIPv4Header_getHeaderLength(ip_header));
    return (sum == 0) ? 0xffff : NyLPC_htons(sum);
}



NyLPC_TBool NyLPC_TIPv4Header_isCorrectIpCheckSum(const struct NyLPC_TIPv4Header* ip_header)
{
    return (NyLPC_TIPv4Header_makeIpChecksum(ip_header)==0xffff);
}

NyLPC_TBool NyLPC_cIPv4Packet_isCorrectTcpCheckSum(const struct NyLPC_TIPv4Header* ip_header)
{
    return (NyLPC_TIPv4Header_makeTcpChecksum(ip_header) == 0xffff);
}




/**
 * TCPチェックサムを計算します。
 * ペイロードはIPヘッダの後方に連続して存在する物と仮定します。
 * i_lenは、ペイロード長さ
 */
NyLPC_TUInt16 NyLPC_TIPv4Header_makeTcpChecksum(
    const struct NyLPC_TIPv4Header* i_iph)
{
    NyLPC_TUInt16 sum;
    NyLPC_TUInt16 iph_len=NyLPC_TIPv4Header_getHeaderLength(i_iph);
    NyLPC_TUInt16 len = NyLPC_ntohs((i_iph)->len16)- iph_len;
    NyLPC_ArgAssert(i_iph!=NULL);
    /*TCP疑似ヘッダ部分*/
    /* IP protocol and length fields. This addition cannot carry. */
    sum = len + i_iph->proto;
    /* Sum IP source and destination addresses. */
    sum = NyLPC_uip_chksum(sum, (NyLPC_TUInt8 *) &(i_iph->srcipaddr), 2 * sizeof(struct NyLPC_TIPv4Addr));
    /* Sum TCP header and data. */
    sum = NyLPC_uip_chksum(sum, (((NyLPC_TUInt8 *)(i_iph))+iph_len),len);
    //  sum = chksum(sum, &uip_buf[UIP_IPH_LEN + UIP_LLH_LEN], i_len_of_data);
    return (sum == 0) ? 0xffff : NyLPC_htons(sum);
}

static  NyLPC_TUInt16 pid=0x3939;
/**
 * IPヘッダを送信パケット用に設定する。
 * ipid16にはコールされるたびに新しい値を設定する。
 * ipcecksumには0を設定する。
 * この関数は、パケットサイズ,ローカルIP/リモートIPの設定はしない。
 */
void NyLPC_TIPv4Header_writeTxIpHeader(
    struct NyLPC_TIPv4Header* i_struct,
    NyLPC_TUInt8 i_proto)
{
    //IPパケットのセット
    i_struct->proto=i_proto;
    i_struct->ttl = UIP_DEFAULT_IP_TTL;
    i_struct->tos = 0;
    i_struct->ipid16=(pid++);
    i_struct->ipoffset=0;//NyLPC_HTONS(0|0x4000);
    i_struct->ipchksum = 0;
}
/*--------------------------------------------------------------------------------
 *
 * struct NyLPC_TIPv6Header
 *
 *------------------------------------------------------------------------------*/
#define IPV6_HEADER_SIZE 40
/**
 * IPヘッダーを作って埋める関数
 */
void NyLPC_TIPv6Header_setSendHeader(
    struct NyLPC_TIPv6Header* i_iph,
    uip_ip6addr_t i_src,
    uip_ip6addr_t i_dest,
    NyLPC_TUInt8 i_proto,
    NyLPC_TUInt8 i_ttl,
    NyLPC_TUInt16 i_len)
{
    i_iph->srcipaddr=i_src;
    i_iph->destipaddr=i_dest;
    i_iph->proto=i_proto;
    i_iph->ttl = i_ttl;
    i_iph->vtc = 0x60;
    i_iph->tcflow = 0x00;
    i_iph->flow = 0x00;
    i_iph->len16= NyLPC_htons(i_len - IPV6_HEADER_SIZE);
    return;
}


/**
 * チェックサムは、TCP疑似ヘッダから計算。
 * i_tcpiphの送信/受信アドレス、ProtocolID,DATAフィールドは有効であること。
 */
NyLPC_TUInt16 NyLPC_TIPv6Header_makeTcpChecksum(
    struct NyLPC_TIPv6Header* i_iph,
    NyLPC_TUInt16 i_len)
{
    NyLPC_TUInt16 sum;
    NyLPC_TUInt16 len;
    len = i_len;
    /*TCP疑似ヘッダ部分*/
    /* IP protocol and length fields. This addition cannot carry. */
    sum = len + i_iph->proto;
    /* Sum IP source and destination addresses. */
    sum = NyLPC_uip_chksum(sum, (NyLPC_TUInt8 *) &(i_iph->srcipaddr), 2 * sizeof(uip_ip6addr_t));
    /* Sum TCP header and data. */
    sum = NyLPC_uip_chksum(sum, (((NyLPC_TUInt8 *)(i_iph))+IPV6_HEADER_SIZE),len);
    //  sum = chksum(sum, &uip_buf[UIP_IPH_LEN + UIP_LLH_LEN], i_len_of_data);
    return (sum == 0) ? 0xffff : NyLPC_htons(sum);
}



/*--------------------------------------------------------------------------------
 *
 * struct NyLPC_TTcpHeader
 *
 *------------------------------------------------------------------------------*/


/**
 * MMSオプションの値を返す。
 */
NyLPC_TBool NyLPC_TTcpHeader_getTcpMmsOpt(
    const struct NyLPC_TTcpHeader* i_struct,NyLPC_TUInt16* o_val)
{
    NyLPC_TUInt8* opt;
    opt=NyLPC_TTcpHeader_getTcpOptFragmentPtr(i_struct,TCP_OPT_MSS);
    if(opt!=NULL){
        if (*(opt+1) == TCP_OPT_MSS_LEN)
        {
            // An MSS option with the right option length.
            *o_val = ((NyLPC_TUInt16) (*(opt+2)) << 8) | (NyLPC_TUInt16) (*(opt + 3));
            //And we are done processing options.
            return NyLPC_TBool_TRUE;
        }
    }
    return NyLPC_TBool_FALSE;
}

#define DEFAULT_TCP_HEADER_LEN 20
/**
 * TCPフラグメントのポインタを返す。
 */
NyLPC_TUInt8* NyLPC_TTcpHeader_getTcpOptFragmentPtr(
    const struct NyLPC_TTcpHeader* i_struct,
    NyLPC_TUInt8 i_opt_id)
{
    NyLPC_TUInt8 opt;
    int c;
    NyLPC_TUInt8* opt_buf=((NyLPC_TUInt8*)(i_struct+1));

    /* Parse the TCP MSS option, if present. */
    if ((i_struct->tcpoffset & 0xf0) > 0x50){
        for (c = 0; c < ((i_struct->tcpoffset >> 4) - 5) << 2;)
        {
            opt=opt_buf[c];
            if(opt==i_opt_id){
                return opt_buf+c;//found!
            }
            switch(opt)
            {
            case TCP_OPT_NOOP:
                continue;//NOP option.
            case TCP_OPT_END:
                return NULL;//End of options.
            default:
                // All other options have a length field, so that we easily can skip past them.
                if (opt_buf[1 + c] == 0) {
                    // If the length field is zero, the options are malformed and we don't process them further.
                    NyLPC_OnErrorGoto(ERROR_INVALID_OPTION);
                }
                c += opt_buf[1 + c];
            }
        }
    }
ERROR_INVALID_OPTION:
    return NULL;
}
/*
 * Optionパラメタを書きだす。
 */
void NyLPC_TTcpHeader_setMmsOpt(NyLPC_TUInt8* i_opt_addr,NyLPC_TUInt16 i_mms)
{
    i_opt_addr[0] = TCP_OPT_MSS;
    i_opt_addr[1] = TCP_OPT_MSS_LEN;
    i_opt_addr[2] = (i_mms) / 256;
    i_opt_addr[3] = (i_mms) & 255;
    return;
}


NyLPC_TUInt16 NyLPC_TTcpHeader_getHeaderLength(const struct NyLPC_TTcpHeader* ip_header)
{
    return (ip_header->tcpoffset>>4)*4;
}
/*--------------------------------------------------------------------------------
 *
 * struct NyLPC_TUdpHeader
 *
 *------------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------------
 *
 * struct NyLPC_TArpHeader
 *
 *------------------------------------------------------------------------------*/
/**
 * i_req_addrを問い合わせるARP_REQUESTを生成します。
 */
void NyLPC_TArpHeader_setArpRequest(
    struct NyLPC_TArpHeader* i_struct,
    const struct NyLPC_TIPv4Addr i_saddr,
    const struct NyLPC_TEthAddr* i_srceth,
    const struct NyLPC_TIPv4Addr* i_req_addr)
{
    memset(i_struct->dhwaddr.addr, 0x00, 6);
    memcpy(i_struct->shwaddr.addr, i_srceth, 6);
    i_struct->dipaddr=*i_req_addr;
    i_struct->sipaddr=i_saddr;
    i_struct->opcode = NyLPC_HTONS(ARP_REQUEST); /* ARP request. */
    i_struct->hwtype = NyLPC_HTONS(ARP_HWTYPE_ETH);
    i_struct->protocol = NyLPC_HTONS(NyLPC_TEthernetIIHeader_TYPE_IP);
    i_struct->hwlen = 6;
    i_struct->protolen = 4;
    return;
}

/*--------------------------------------------------------------------------------
 *
 * class IPv4Route
 *
 *------------------------------------------------------------------------------*/


