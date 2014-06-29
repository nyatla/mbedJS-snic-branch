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
#ifndef NyLPC_uip_h
#define NyLPC_uip_h
#include "../include/NyLPC_config.h"
#include "../include/NyLPC_stdlib.h"
#include "NyLPC_uip_ethernet.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define UIP_PROTO_ICMP  1
#define UIP_PROTO_TCP   6
#define UIP_PROTO_UDP   17
#define UIP_PROTO_ICMP6 58

#define ARP_REQUEST 1
#define ARP_REPLY   2
#define ARP_HWTYPE_ETH 1


#ifndef PACK_STRUCT_END
    #define PACK_STRUCT_END __attribute((packed))
#endif

/**********************************************************************
 *
 *
 *
 **********************************************************************/

/**
 * IPアドレスを格納します。
 * IPアドレスは、ネットワークオーダーで設定します。
 */
struct NyLPC_TIPv4Addr
{
    NyLPC_TUInt32 v;
}PACK_STRUCT_END;
extern const struct NyLPC_TIPv4Addr NyLPC_TIPv4Addr_ZERO;
extern const struct NyLPC_TIPv4Addr NyLPC_TIPv4Addr_ALL;
extern const struct NyLPC_TIPv4Addr NyLPC_TIPv4Addr_BROADCAST;
extern const struct NyLPC_TIPv4Addr NyLPC_TIPv4Addr_MULTICAST;
extern const struct NyLPC_TIPv4Addr NyLPC_TIPv4Addr_MULTICAST_MASK;
extern const struct NyLPC_TIPv4Addr NyLPC_TIPv4Addr_APIPA_MASK;

/**
 * addr1とaddr2が全く同じであるかをテストします。
 * \hideinitializer
 */
#define NyLPC_TIPv4Addr_isEqual(v1,v2) ((v1)->v==(v2)->v)
/**
 * addr1とaddr2をmaskでマスクした結果を比較します。
 * \hideinitializer
 */
#define NyLPC_TIPv4Addr_isEqualWithMask(addr1, addr2, mask) ((((addr1)->v) & ((mask)->v))==(((addr2)->v) & ((mask)->v)))

/**
 * 変数にIP v4アドレスをセットします。
 * 次のように使います。
 \code
 NyLPC_TIPv4Addr ip;
 NyLPC_TIpv4Addr_set(&ip,1,2,3,4);
 \endcode
 */
#define NyLPC_TIPv4Addr_set(s,a0,a1,a2,a3) (s)->v=NyLPC_htonl((0xff000000&(((NyLPC_TUInt32)(a0))<<24))|(0x00ff0000&(((NyLPC_TUInt32)(a1))<<16))|(0x0000ff00&(((NyLPC_TUInt32)(a2))<<8))|(0x000000ff&((NyLPC_TUInt32)(a3))))
#define NyLPC_TIPv4Addr_pack(a0,a1,a2,a3) {NyLPC_HTONL((0xff000000&(((NyLPC_TUInt32)(a0))<<24))|(0x00ff0000&(((NyLPC_TUInt32)(a1))<<16))|(0x0000ff00&(((NyLPC_TUInt32)(a2))<<8))|(0x000000ff&((NyLPC_TUInt32)(a3))))}

/**
 * IPアドレスを文字列に変換して返します。
 */
NyLPC_TInt16 NyLPC_TIPv4Addr_toString(const struct NyLPC_TIPv4Addr* i_ip,NyLPC_TChar* i_buf);





struct uip_ip6addr
{
    NyLPC_TUInt16 v[8];
}PACK_STRUCT_END;

typedef struct uip_ip6addr uip_ip6addr_t;

NyLPC_TUInt16 NyLPC_uip_chksum(NyLPC_TUInt16 sum, const NyLPC_TUInt8 *data, NyLPC_TUInt16 len);




/**
 * TTL
 */
#define UIP_DEFAULT_IP_TTL 64



/**
 * The maximum number of times a segment should be retransmitted
 * before the connection should be aborted.
 *
 * This should not be changed.
 */
#define UIP_MAXRTX      8





#   if UIP_BYTE_ORDER == NyLPC_ENDIAN_BIG
#       define NyLPC_HTONS(n) (n)
#       define NyLPC_htons(n) (n)
#       define NyLPC_ntohs(n) (n)
#       define NyLPC_htonl(n) (n)
#       define NyLPC_ntohl(n) (n)
#       define NyLPC_HTONS(n) (n)
#       define NyLPC_NTOHS(n) (n)
#   else
#       define NyLPC_htonl(n) NyLPC_TUInt32_bswap(n)
#       define NyLPC_ntohl(n) NyLPC_TUInt32_bswap(n)
#       define NyLPC_htons(n) NyLPC_TUInt16_bswap(n)
#       define NyLPC_ntohs(n) NyLPC_TUInt16_bswap(n)
#       define NyLPC_HTONS(n) NyLPC_TUInt16_BSWAP(n)
#       define NyLPC_NTOHS(n) NyLPC_TUInt16_BSWAP(n)
#       define NyLPC_NTOHL(n) NyLPC_TUInt32_BSWAP(n)
#       define NyLPC_HTONL(n) NyLPC_TUInt32_BSWAP(n)
#endif /* NyLPC_HTONS */




/**********************************************************************
 *
 * struct NyLPC_TEthernetIIHeader
 *
 **********************************************************************/

#define NyLPC_TEthernetIIHeader_TYPE_IP     0x0800
#define NyLPC_TEthernetIIHeader_TYPE_ARP    0x0806
#define NyLPC_TEthernetIIHeader_TYPE_IPV6   0x86DD
//#define UIP_ETHTYPE_IP  0x0800
//#define UIP_ETHTYPE_ARP 0x0806
//#define UIP_ETHTYPE_IP6 0x86dd

struct NyLPC_TEthernetIIHeader
{
    struct NyLPC_TEthAddr dest;
    struct NyLPC_TEthAddr src;
    NyLPC_TUInt16 type;
}PACK_STRUCT_END;


NyLPC_TUInt16 NyLPC_TEthernetIIHeader_setArpTx(
    struct NyLPC_TEthernetIIHeader* i_struct,
    const struct NyLPC_TEthAddr* i_my_eth_addr);

NyLPC_TUInt16 NyLPC_TEthernetIIHeader_setIPv4Tx(
    struct NyLPC_TEthernetIIHeader* i_eth,
    const struct NyLPC_TEthAddr* i_src_eth_addr,
    const struct NyLPC_TEthAddr* i_dest_eth_addr);
/**********************************************************************
 *
 * struct NyLPC_TIPv4Header
 *
 **********************************************************************/


/**
 * IPパケットヘッダのメモリマップ構造体
 * 値はすべてネットワークオーダーです。
 */
struct NyLPC_TIPv4Header
{
    NyLPC_TUInt8 vhl;
    NyLPC_TUInt8 tos;
    NyLPC_TUInt16 len16;
    NyLPC_TUInt16 ipid16;
    NyLPC_TUInt16 ipoffset;
    NyLPC_TUInt8 ttl;
    NyLPC_TUInt8 proto;
    NyLPC_TUInt16 ipchksum;
    struct NyLPC_TIPv4Addr srcipaddr;
    struct NyLPC_TIPv4Addr destipaddr;
}PACK_STRUCT_END;




NyLPC_TBool NyLPC_TIPv4Header_isCorrectIpCheckSum(
    const struct NyLPC_TIPv4Header* ip_header);

NyLPC_TBool NyLPC_cIPv4Packet_isCorrectTcpCheckSum(
    const struct NyLPC_TIPv4Header* ip_header);

NyLPC_TUInt16 NyLPC_TIPv4Header_makeIpChecksum(const struct NyLPC_TIPv4Header* ip_header);


NyLPC_TUInt16 NyLPC_TIPv4Header_makeTcpChecksum(
    const struct NyLPC_TIPv4Header* i_iph);

#define NyLPC_TIPv4Header_isDestAddrEqual(i_struct,i_addr) ((i_struct)->destipaddr==(i_addr))
#define NyLPC_TIPv4Header_isSrcAddrEqual(i_struct,i_addr) ((i_struct)->srcipaddr==(i_addr))

/**
 * IPヘッダの長さを返す。
 */
#define NyLPC_TIPv4Header_getHeaderLength(i_iph) (((i_iph)->vhl & 0x0f)*4)
/**
 * IPパケット全体の長さを返す。
 */
#define NyLPC_TIPv4Header_getPacketLength(i_iph) (NyLPC_ntohs((i_iph)->len16))

/**
 * IPヘッダを送信パケット用に設定する。
 * ipid16にはコールされるたびに新しい値を設定する。
 * ipcecksumには0を設定する。
 * この関数は、パケットサイズ,ローカルIP/リモートIPの設定はしない。
 */
void NyLPC_TIPv4Header_writeTxIpHeader(
    struct NyLPC_TIPv4Header* i_struct,
    NyLPC_TUInt8 i_proto);
/**********************************************************************
 *
 * struct NyLPC_TIPv6Header
 *
 **********************************************************************/


struct NyLPC_TIPv6Header
{
    /* IPv6 header. */
    NyLPC_TUInt8 vtc;
    NyLPC_TUInt8 tcflow;
    NyLPC_TUInt16 flow;
    NyLPC_TUInt8 len16;
    NyLPC_TUInt8 proto, ttl;
    uip_ip6addr_t srcipaddr;
    uip_ip6addr_t destipaddr;
}PACK_STRUCT_END;

void NyLPC_TIPv6Header_setSendHeader(
    struct NyLPC_TIPv6Header* i_iph,
    uip_ip6addr_t i_src,
    uip_ip6addr_t i_dest,
    NyLPC_TUInt8 i_proto,
    NyLPC_TUInt8 i_ttl,
    NyLPC_TUInt16 i_len);

NyLPC_TUInt16 NyLPC_TIPv6Header_makeTcpChecksum(
    struct NyLPC_TIPv6Header* i_iph,
    NyLPC_TUInt16 i_len);

#define NyLPC_TIPv6Header_isDestAddrEqual(i_struct,i_addr) (memcmp((i_struct)->destipaddr,(i_addr),sizeof(uip_ip6addr_t)))
#define NyLPC_TIPv6Header_isSrcAddrEqual(i_struct,i_addr) (memcmp(i_struct)->srcipaddr,(i_addr),sizeof(uip_ip6addr_t)))
/**********************************************************************
 *
 * struct NyLPC_TTcpHeader
 *
 **********************************************************************/
#define TCP_OPT_END     0   /* End of TCP options list */
#define TCP_OPT_NOOP    1   /* "No-operation" TCP option */
#define TCP_OPT_MSS     2   /* Maximum segment size TCP option */
#define TCP_OPT_MSS_LEN 4   /* Length of TCP MSS option. */

/**
 * TCP/IPヘッダのメモリマップ構造体
 * マルチバイトの値は、全てネットワークオーダーです。
 */
struct NyLPC_TTcpHeader
{
    //TCP header.
    NyLPC_TUInt16 srcport;
    NyLPC_TUInt16 destport;
    NyLPC_TUInt32 seqno32;
    NyLPC_TUInt32 ackno32;
    NyLPC_TUInt8 tcpoffset;
    NyLPC_TUInt8 flags;
    NyLPC_TUInt16 wnd16;
    NyLPC_TUInt16 tcpchksum;
    NyLPC_TUInt8 urgp[2];
//  NyLPC_TUInt8 optdata[4];
} PACK_STRUCT_END;


NyLPC_TUInt8* NyLPC_TTcpHeader_getTcpOptFragmentPtr(
    const struct NyLPC_TTcpHeader* i_struct,
    NyLPC_TUInt8 i_opt_id);

NyLPC_TBool NyLPC_TTcpHeader_getTcpMmsOpt(
    const struct NyLPC_TTcpHeader* i_struct,NyLPC_TUInt16* o_val);

/**
 * この関数は、TCPヘッダの長さを返します。ヘッダの長さは、i_structのフィールドから計算します。
 * @param i_struct
 * 構造体のアドレス。
 */
NyLPC_TUInt16 NyLPC_TTcpHeader_getHeaderLength(const struct NyLPC_TTcpHeader* i_struct);

/**
 * この関数は、指定したアドレスに、mmsオプション値を書き込みます。
 */
void NyLPC_TTcpHeader_setMmsOpt(NyLPC_TUInt8* i_opt_addr,NyLPC_TUInt16 i_mms);


/**********************************************************************
 *
 * struct NyLPC_TUdpHeader
 *
 **********************************************************************/

/**
 * UDP/IPヘッダのメモリマップ構造体
 */
struct NyLPC_TUdpHeader
{
    NyLPC_TUInt16 srcport;
    NyLPC_TUInt16 destport;
    NyLPC_TUInt16 udplen;
    NyLPC_TUInt16 udpchksum;
} PACK_STRUCT_END;

/**
 * UDPヘッダの長さを返す。
 */
#define NyLPC_TUdpHeader_getHeaderLength(i_struct) (8)

/**********************************************************************
 *
 * struct NyLPC_TIcmpipHeader
 *
 **********************************************************************/


struct NyLPC_TIcmpHeader
{
    /* ICMP (echo) header. */
    NyLPC_TUInt8 type, icode;
    NyLPC_TUInt16 icmpchksum;
    #if !UIP_CONF_IPV6
        NyLPC_TUInt16 id, seqno;
    #else /* !UIP_CONF_IPV6 */
        NyLPC_TUInt8 flags, reserved1, reserved2, reserved3;
        NyLPC_TUInt8 icmp6data[16];
        NyLPC_TUInt8 options[1];
    #endif /* !UIP_CONF_IPV6 */
} PACK_STRUCT_END;

/**********************************************************************
 *
 * struct NyLPC_TIcmpipHeader
 *
 **********************************************************************/

struct NyLPC_TArpHeader
{
  NyLPC_TUInt16 hwtype;
  NyLPC_TUInt16 protocol;
  NyLPC_TUInt8 hwlen;
  NyLPC_TUInt8 protolen;
  NyLPC_TUInt16 opcode;
  struct NyLPC_TEthAddr shwaddr;
  struct NyLPC_TIPv4Addr sipaddr;
  struct NyLPC_TEthAddr dhwaddr;
  struct NyLPC_TIPv4Addr dipaddr;
} PACK_STRUCT_END;


/**
 * i_req_addrを問い合わせるARP_REQUESTを生成します。
 */
void NyLPC_TArpHeader_setArpRequest(
    struct NyLPC_TArpHeader* i_struct,
    const struct NyLPC_TIPv4Addr i_saddr,
    const struct NyLPC_TEthAddr* i_srceth,
    const struct NyLPC_TIPv4Addr* i_req_addr);


typedef struct NyLPC_TcEthernetIIPayload NyLPC_TcEthernetIIPayload_t;




/**********************************************************************
 *
 * NyLPC_TIPv6Payload
 *
 **********************************************************************/


struct NyLPC_TIPv6Payload
{
    struct NyLPC_TIPv6Header* header;
    union{
        void* rawbuf;
        void* tcp;
        void* udp;
        void* icmp6;
    }payload;
};

struct NyLPC_TcEthernetIIPayload
{
    struct NyLPC_TEthernetIIHeader* header;
    union{
        void* rawbuf;
        struct NyLPC_TArpHeader* arp;
        struct NyLPC_TIPv4Payload* ipv4;
        struct NyLPC_TIPv6Payload* ipv6;
    }payload;
    NyLPC_TUInt16 len;
};





#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif
