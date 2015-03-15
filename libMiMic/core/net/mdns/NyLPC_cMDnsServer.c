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
#include "NyLPC_cMDnsServer.h"
#include "NyLPC_netif.h"
#include "NyLPC_http.h"
#include "NyLPC_utils.h"
#include <stdio.h>
#include <string.h>


/**
 * mDNSのポート番号
 */
#define MDNS_MCAST_PORT         5353
static const struct NyLPC_TIPv4Addr MDNS_MCAST_IPADDR = NyLPC_TIPv4Addr_pack(224, 0, 0, 251);
#define TIMEOUT_IN_MS       1000
#define NyLPC_TcMDns_RES_TTL (120)    //120
#define NyLPC_TcMDns_STD_TTL (30*60)  //(30min


struct NyLPC_TDnsHeader
{
    NyLPC_TUInt16 id;
    NyLPC_TUInt16 flag;
    NyLPC_TUInt16 qd;
    NyLPC_TUInt16 an;
    NyLPC_TUInt16 ns;
    NyLPC_TUInt16 ar;
}PACK_STRUCT_END;

#define NyLPC_TDnsHeader_FLAG_MASK_QR     0x8000
#define NyLPC_TDnsHeader_FLAG_MASK_OPCODE 0x7800
#define NyLPC_TDnsHeader_FLAG_MASK_AA     0x0400
#define NyLPC_TDnsHeader_FLAG_MASK_TC     0x0200
#define NyLPC_TDnsHeader_FLAG_MASK_RD     0x0100
#define NyLPC_TDnsHeader_FLAG_MASK_RA     0x0080
#define NyLPC_TDnsHeader_FLAG_MASK_Z      0x0070
#define NyLPC_TDnsHeader_FLAG_MASK_RECODE 0x000F

struct NyLPC_TDnsQuestion
{
    const char* buf;        //Questionパケットの先頭
    NyLPC_TUInt16 buf_len;  //パケット長さ
    NyLPC_TUInt16 qtype;
    NyLPC_TUInt16 qclass;
    NyLPC_TInt16 qname_pos;//Qnameの開始位置
};

#define NyLPC_TDnsQuestion_QTYPR_A      1
#define NyLPC_TDnsQuestion_QTYPR_NS     2
#define NyLPC_TDnsQuestion_QTYPR_CNAME  5
#define NyLPC_TDnsQuestion_QTYPR_SOA    6
#define NyLPC_TDnsQuestion_QTYPR_PTR    12
#define NyLPC_TDnsQuestion_QTYPR_MX     15
#define NyLPC_TDnsQuestion_QTYPR_TXT    16
#define NyLPC_TDnsQuestion_QTYPR_ANY    255
#define NyLPC_TDnsQuestion_QTYPR_AAAA   28
#define NyLPC_TDnsQuestion_QTYPR_NSEC   47
#define NyLPC_TDnsQuestion_QCLASS_IN    1
#define NyLPC_TDnsQuestion_QCLASS_CH    3
#define NyLPC_TDnsQuestion_QCLASS_HS    4
#define NyLPC_TDnsQuestion_QTYPR_SRV    33
#define NyLPC_TDnsQuestion_QCLASS_CACHE_FLUSH  0x8000


/**************************************************
* TLabelCache
**************************************************/
/**
 * nameフィールドの文字列圧縮を解除して圧縮後のテキストポインタを返します。
 */
static const char* getExtractNamePos(const char* i_packet_buf, const char* i_spos)
{
    NyLPC_TUInt8 limit = 0;
    const char* s = i_spos;// question->buf + pos;//クエリの解析位置
    for (;;){
        switch (*(const NyLPC_TUInt8*)s){
        case 0x00:
            //queryが先に終了に到達するのはおかしい。
            return NULL;
        case 0xc0:
            s = i_packet_buf + *((const NyLPC_TUInt8*)s + 1);//参照先にジャンプ
            if (i_spos <= s){
                //後方参照ならエラー
                return NULL;
            }
            limit++;
            if (limit > 32){
                return NULL;
            }
            continue;
        default:
            break;
        }
        break;
    }
    return s;
}

/**
 * 展開しない圧縮文字列の長さを返す
 */
static NyLPC_TInt16 sizeofCompressName(const char* i_str)
{
    NyLPC_TInt16 l = 0;
    NyLPC_TUInt8 c;
    for (;;){
        c = (NyLPC_TUInt8)*(i_str + l);
        switch (c){
        case 0xc0:
            return l + 2;
        case 0x00:
            return l + 1;
        }
        l += c + 1;
    }
}
/**
 * @return
 * 圧縮するとtrue i_nameは2文字に圧縮される。
 */
static NyLPC_TBool compressNameB(char* i_packet, NyLPC_TUInt16 i_spos, NyLPC_TUInt16 i_name_pos)
{
    NyLPC_TUInt16 ret;
    const char* p;  //プロトコル文字列の解析開始位置
    const char* s = i_packet + i_spos;  //クエリの解析位置
    const char* s2;

    //Protocol
    for (;;){
        //0xc0参照の解決
        s = getExtractNamePos(i_packet, s);
        if (s == NULL){
            //一致しない
            return 0;
        }
        for (;;){
            //検索位置のドメインを遡る
            if (*s == *(i_packet + i_name_pos)){
                //先頭一致
                p = i_packet + i_name_pos;
                s2 = getExtractNamePos(i_packet, s);
                ret = (NyLPC_TUInt16)(s2 - i_packet);
                for (;;){
                    //一致判定
                    if (s2 == NULL){
                        break;
                    }

                    if (memcmp(p, s2, (*(NyLPC_TUInt8*)s2 + 1)) != 0){
                        //不一致
                        break;
                    }
                    //検出位置の移動
                    p += (*(NyLPC_TUInt8*)s2) + 1;
                    s2 += (*(NyLPC_TUInt8*)s2) + 1;
                    if (*p == 0 && *s2 == 0){
                        //charで扱ってるクライアントのコケ防止
                        if (ret>0xff){
                            return 0;
                        }
                        *(i_packet + i_name_pos + 0) = 0xc0;
                        *(i_packet + i_name_pos + 1) = (NyLPC_TUInt8)ret;
                        return ret;
                    }
                    s2 = getExtractNamePos(i_packet, s2);
                }
            }
            //一致しない->検索パターンの次のフラグメントを調べる
            s += (*(NyLPC_TUInt8*)s) + 1;
            break;
        }
    }
}
/**
 * mDNSパケットからi_nameを検索する。
 * i_targetはi_packetに含まれ、NULL終端されていること
 * @return 0 圧縮失敗/その他:圧縮後のパケットサイズ
 *
 */
static NyLPC_TUInt16 compressNameA(char* i_packet, NyLPC_TUInt16 i_name_start, NyLPC_TUInt16 i_name_pos)
{
    NyLPC_TUInt16 s = 12;
    for (;;){
        if (i_name_start <= s){
            //圧縮対象文字列に到達
            return 0;
        }
        //i_targetとAnswer文字列が等しいか確認
        if (compressNameB(i_packet, s, i_name_pos)){
            return i_name_pos + 2;
        }
        //
        s += sizeofCompressName(i_packet + s);
        s += 2 + 2 + 4 + 2;
        if (i_name_start <= s){
            //圧縮対象文字列に到達
            return 0;
        }
        //          //データは圧縮ポインタにしない方がいいらしい。
        //          if (compressNameB(i_packet, s, i_name_pos)){
        //              return i_name_pos + 2;
        //          }
        s += NyLPC_ntohs(*((NyLPC_TUInt16*)(i_packet + s - 2)));
    }
}
/**
 * @return
 * 新しいi_packetの長さ
 */
static NyLPC_TUInt16 compressName(char* i_packet, NyLPC_TUInt16 i_name_pos, NyLPC_TUInt16 i_name_len)
{
    NyLPC_TUInt16 p;
    NyLPC_TUInt16 s = i_name_pos;
    for (;;){
        p = compressNameA(i_packet, i_name_pos, s);
        //i_targetとAnswer文字列が等しいか確認
        if (p != 0){
            return p;
        }
        s += (NyLPC_TUInt16)*(i_packet + s) + 1;
        if (*(i_packet + s) == 0){
            //圧縮対象文字列に到達
            return (i_name_len + i_name_pos);
        }
    }
}





/**
 * 受領可能なQuestionか確認する
 * @return
 * 受領可能なQuestionの数
 *
 */
static NyLPC_TUInt16 getNumberOfQuestion(const void* i_packet, NyLPC_TUInt16 i_len)
{
    struct NyLPC_TDnsHeader* ptr = (struct NyLPC_TDnsHeader*)i_packet;
    NyLPC_TUInt16 t;
    if (i_len<sizeof(struct NyLPC_TDnsHeader)){
        return NyLPC_TBool_FALSE;
    }
    //questrionの確認
    //QR==0 && op==0 && tc=0
    t = NyLPC_ntohs(ptr->flag);
    if (((t & NyLPC_TDnsHeader_FLAG_MASK_QR) != 0) &&
        ((t & NyLPC_TDnsHeader_FLAG_MASK_OPCODE) != 0) &&
        ((t & NyLPC_TDnsHeader_FLAG_MASK_TC) != 0))
    {
        //this is response
        return 0;
    }
    return NyLPC_ntohs(ptr->qd);
}

/**
 * [i_name].[i_protocol].localをquestionと比較します。i_nameは省略ができます。
 * i_nameはincasesensitiveです。
 * @return 等しい場合true
 */
static NyLPC_TBool NyLPC_TDnsQuestion_isEqualName(const struct NyLPC_TDnsQuestion* question, const char* i_name, const char* i_protocol)
{
    NyLPC_TUInt8 tmp;
    const char* p;                      //プロトコル文字列の解析開始位置
    const char* s = question->buf + (NyLPC_TUInt8)question->qname_pos; //クエリの解析位置

    //Domain
    if (i_name != NULL){
        //0xc0参照の解決
        s = getExtractNamePos(question->buf, s);
        if (s == NULL){
            return NyLPC_TBool_FALSE;
        }
        tmp = (NyLPC_TUInt8)strlen(i_name);
        if (tmp != *s || NyLPC_strnicmp(s + 1, i_name, tmp) != 0){
            return NyLPC_TBool_FALSE;
        }
        s += (*s) + 1;
    }
    else{
        s = question->buf + (NyLPC_TUInt8)question->qname_pos;//クエリの解析位置
    }
    p = i_protocol;
    //Protocol
    for (;;){
        //0xc0参照の解決
        s = getExtractNamePos(question->buf, s);
        if (s == NULL){
            return NyLPC_TBool_FALSE;
        }
        //SRVの末端到達
        if (*p == 0){
            if (question->buf + question->buf_len<s + 7 + 4){
                return NyLPC_TBool_FALSE;
            }
            return (memcmp("\5local\0", s, 7) == 0);
        }
        //有効サイズなら一致検出
        if (question->buf + question->buf_len<s + 1 + (NyLPC_TUInt8)*s + 4){
            return NyLPC_TBool_FALSE;
        }
        if (memcmp(p, s + 1, (NyLPC_TUInt8)*s) != 0){
            //不一致
            return NyLPC_TBool_FALSE;
        }
        //検出位置の移動
        p += (*s) + 1;
        s += (*s) + 1;
    }
}


static NyLPC_TUInt16 NyLPC_TDnsQuestion_parse(const char* i_packet, NyLPC_TUInt16 i_packet_len, NyLPC_TInt16 i_parse_start, struct NyLPC_TDnsQuestion* o_val)
{
    NyLPC_TUInt16 i;
    //解析開始位置を計算
    NyLPC_TUInt16 qlen = 0;
    for (i = i_parse_start; i<i_packet_len - 4; i++){
        switch ((NyLPC_TUInt8)(*(i_packet + i))){
        case 0x00:
            qlen++;
            break;
        case 0xc0:
            qlen += 2;
            break;
        default:
            qlen++;
            continue;
        }
        o_val->buf = i_packet;
        o_val->buf_len = i_packet_len;
        o_val->qname_pos = i_parse_start;
        o_val->qtype = NyLPC_ntohs(*(NyLPC_TUInt16*)(i_packet + i_parse_start + qlen));
        o_val->qclass = NyLPC_ntohs(*(NyLPC_TUInt16*)(i_packet + i_parse_start + qlen + sizeof(NyLPC_TUInt16)));
        return qlen + 4;
    }
    return 0;
}

/**
 * DNSレコードのPRTフィールドとDNSラベル文字列を比較する。
 */
static NyLPC_TInt16 NyLPC_TDnsRecord_getMatchPtrIdx(const struct NyLPC_TDnsRecord* i_struct, const struct NyLPC_TDnsQuestion* question)
{
    NyLPC_TInt16 i;
    for (i = 0; i<i_struct->num_of_srv; i++){
        if (NyLPC_TDnsQuestion_isEqualName(question, NULL, i_struct->srv[i].protocol)){
            return i;
        }
    }
    return -1;
}
static NyLPC_TInt16 NyLPC_TDnsRecord_getMatchSrvIdx(const struct NyLPC_TDnsRecord* i_struct, const struct NyLPC_TDnsQuestion* question)
{
    NyLPC_TInt16 i;
    for (i = 0; i<i_struct->num_of_srv; i++){
        if (NyLPC_TDnsQuestion_isEqualName(question, i_struct->name, i_struct->srv[i].protocol)){
            return i;
        }
    }
    return -1;
}



/**
 * '.'区切り文字列をDNS形式の[n]text[n]text\0へ変換する。
 * @return
 * 変換後のデータブロックの長さin byte
 * 終端の\0の長さを含みます。
 */
static NyLPC_TInt16 str2label(NyLPC_TChar* buf, const NyLPC_TChar* name)
{
    //proto文字列の変換
    NyLPC_TChar* lp;
    const NyLPC_TChar* n = name;
    NyLPC_TChar* b = buf;
    while (*n != '\0'){
        lp = b;
        b++;
        for (; strchr(".\0", *n) == NULL;){
            *b = *n;
            b++;
            n++;
        }
        *lp = (char)(b - lp - 1);
        if (*n != '\0'){
            n++;
        }
    }
    *b = '\0';
    b++;
    return (NyLPC_TInt16)((int)b - (int)buf);
}
/**
 * query文字列をパケットに追記します。
 * @return 出力したバイト数
 */
static NyLPC_TInt16 query2label(char* i_packet, NyLPC_TInt16 i_spos, NyLPC_TInt16 obuflen, const struct NyLPC_TDnsQuestion* i_query)
{
    const char* n;
    NyLPC_TInt16 s = i_query->qname_pos;
    NyLPC_TInt16 l = 0;//出力した文字数
    for (;;){
        n = getExtractNamePos(i_query->buf, i_query->buf + s);
        if (n == NULL){
            *(i_packet + i_spos + l) = 0;
            l++;
            break;
        }
        else{
            memcpy(i_packet + i_spos + l, n, ((NyLPC_TUInt8)*n) + 1);
            l += ((NyLPC_TUInt8)*n) + 1;
            s = (NyLPC_TInt16)((int)n - (int)i_query->buf + ((NyLPC_TUInt8)*n) + 1);
            if (obuflen<i_spos + l + 1){
                return 0;
            }
        }
    }
    return l;
}


static NyLPC_TInt16 writeSrvResourceHeader(char* i_packet, NyLPC_TInt16 i_spos, NyLPC_TInt16 buflen, const struct NyLPC_TDnsRecord* i_recode, NyLPC_TInt16 i_sid, NyLPC_TUInt16 i_type, NyLPC_TUInt16 i_class,NyLPC_TUInt16 i_ttl)
{
    NyLPC_TInt16 s;
    NyLPC_TInt16 l = (NyLPC_TInt16)(1 + strlen(i_recode->name) + 1 + strlen(i_recode->srv[i_sid].protocol) + 1 + 5 + 1);
    if (buflen<i_spos + l + 2 + 2 + 4){
        return 0;
    }
    s = str2label(i_packet + i_spos, i_recode->name) - 1;
    s += str2label(i_packet + i_spos + s, i_recode->srv[i_sid].protocol) - 1;
    s += str2label(i_packet + i_spos + s, "local");
    l = compressName(i_packet, i_spos, s);//圧縮
    (*(NyLPC_TUInt16*)(i_packet + l)) = NyLPC_HTONS(i_type);
    (*(NyLPC_TUInt16*)(i_packet + l + 2)) = NyLPC_HTONS(i_class);
    (*(NyLPC_TUInt32*)(i_packet + l + 4)) = NyLPC_HTONL(i_ttl);
    return l + 2 + 2 + 4;
}

/**
* パケットヘッダを書き込みます。
* @return パケットサイズ
*/
static NyLPC_TUInt16 setResponseHeader(char* i_packet, const struct NyLPC_TDnsHeader* i_in_dns_header, NyLPC_TUInt16 i_an_count, NyLPC_TUInt16 i_ns_count, NyLPC_TUInt16 i_ar_count)
{
    struct NyLPC_TDnsHeader* p = (struct NyLPC_TDnsHeader*)i_packet;
    if (i_in_dns_header != NULL){
        memcpy(p, i_in_dns_header, sizeof(struct NyLPC_TDnsHeader));
        p->flag = p->flag | NyLPC_HTONS(NyLPC_TDnsHeader_FLAG_MASK_QR | NyLPC_TDnsHeader_FLAG_MASK_AA);
        p->flag = p->flag & NyLPC_HTONS(~(NyLPC_TDnsHeader_FLAG_MASK_RECODE | NyLPC_TDnsHeader_FLAG_MASK_TC | NyLPC_TDnsHeader_FLAG_MASK_RA));
    }
    else{
        p->flag = 0;
        p->id = 0;
    }
    p->qd = 0;
    p->an = NyLPC_HTONS(i_an_count);
    p->ns = NyLPC_HTONS(i_ns_count);
    p->ar = NyLPC_HTONS(i_ar_count);
    return sizeof(struct NyLPC_TDnsHeader);
}
static NyLPC_TUInt16 setAnnounceHeader(char* i_packet,NyLPC_TUInt16 i_an_count, NyLPC_TUInt16 i_ns_count, NyLPC_TUInt16 i_ar_count)
{
    struct NyLPC_TDnsHeader* p = (struct NyLPC_TDnsHeader*)i_packet;
    p->id = 0;
    p->flag = NyLPC_HTONS(NyLPC_TDnsHeader_FLAG_MASK_QR | NyLPC_TDnsHeader_FLAG_MASK_AA);
    p->qd = 0;
    p->an = NyLPC_HTONS(i_an_count);
    p->ns = NyLPC_HTONS(i_ns_count);
    p->ar = NyLPC_HTONS(i_ar_count);
    return sizeof(struct NyLPC_TDnsHeader);
}

static NyLPC_TInt16 writeARecordData(char* i_packet, NyLPC_TInt16 i_spos,NyLPC_TInt16 obuflen,const struct NyLPC_TIPv4Addr* ip)
{
    NyLPC_TInt16 l=i_spos;
    (*(NyLPC_TUInt16*)(i_packet + l)) = NyLPC_HTONS(NyLPC_TDnsQuestion_QTYPR_A);
    (*(NyLPC_TUInt16*)(i_packet + l + 2)) = NyLPC_HTONS(NyLPC_TDnsQuestion_QCLASS_IN | NyLPC_TDnsQuestion_QCLASS_CACHE_FLUSH);
    (*(NyLPC_TUInt32*)(i_packet + l + 4)) = NyLPC_HTONL(NyLPC_TcMDns_RES_TTL);
    l += 2 + 2 + 4;
    //A record header
    if (obuflen<l + 6){
        return 0;
    }
    //Aレコードを書く
    //IPADDR
    (*(NyLPC_TUInt16*)(i_packet + l)) = NyLPC_HTONS(4);
    (*(NyLPC_TUInt32*)(i_packet + l + 2)) = ip->v;
    return l + 6;//NEXT_SPOS
}
/**
* ドメイン名からAレコードレスポンスを書きだす。
*/
static NyLPC_TInt16 writeARecord(char* i_packet, NyLPC_TInt16 i_spos, NyLPC_TInt16 obuflen, const NyLPC_TChar* i_name, const struct NyLPC_TIPv4Addr* ip)
{
    //AnswerはAレコードのみ
    NyLPC_TInt16 s = 1 + (NyLPC_TInt16)strlen(i_name) + 1 + 5 + 1;
    if (obuflen<i_spos + s + 4 + 4){
        return 0;
    }
    s = str2label(i_packet + i_spos, i_name) - 1;
    s += str2label(i_packet + i_spos + s, "local");
    //レコード圧縮
    s = compressName(i_packet, i_spos, s);
    return writeARecordData(i_packet,s,obuflen,ip);
}

/**
* AレコードクエリからAレコードレスポンスを書きだす。
*/
static NyLPC_TInt16 writeARecordByQuery(char* i_packet, NyLPC_TInt16 i_spos, NyLPC_TInt16 obuflen, const struct NyLPC_TDnsQuestion* i_query, const struct NyLPC_TIPv4Addr* ip)
{
    NyLPC_TInt16 s = query2label(i_packet, i_spos, obuflen, i_query);
    if (s == 0){
        return 0;
    }
    //レコード圧縮
    s = compressName(i_packet, i_spos, s);
    return writeARecordData(i_packet,s,obuflen,ip);
}

/*
static NyLPC_TInt16 writeAAAARecordByQueryData(char* i_packet, NyLPC_TInt16 i_spos,NyLPC_TInt16 obuflen,const struct NyLPC_TIPv4Addr* ip)
{
    NyLPC_TInt16 l=i_spos;
    (*(NyLPC_TUInt16*)(i_packet + l)) = NyLPC_HTONS(NyLPC_TDnsQuestion_QTYPR_AAAA);
    (*(NyLPC_TUInt16*)(i_packet + l + 2)) = NyLPC_HTONS(NyLPC_TDnsQuestion_QCLASS_IN | NyLPC_TDnsQuestion_QCLASS_CACHE_FLUSH);
    (*(NyLPC_TUInt32*)(i_packet + l + 4)) = NyLPC_HTONL(NyLPC_TcMDns_RES_TTL);
    l += 2 + 2 + 4;
    //A record header
    if (obuflen<l + 2 + 16){
        return 0;
    }
    //AAAAレコードを書く
    //IPADDR
    (*(NyLPC_TUInt16*)(i_packet + l)) = NyLPC_HTONS(16);
    memset(i_packet + l + 2, 0, 10);
    (*(NyLPC_TUInt16*)(i_packet + l + 2 + 10)) = 0xffff;
    (*(NyLPC_TUInt32*)(i_packet + l + 2 + 12)) = ip->v;
    return l + 2 + 16;
}*/


/**
* AレコードクエリからAレコードレスポンスを書きだす。
*//*
static NyLPC_TInt16 writeAAAARecordByQuery(char* i_packet, NyLPC_TInt16 i_spos, NyLPC_TInt16 obuflen, const struct NyLPC_TDnsQuestion* i_query, const struct NyLPC_TIPv4Addr* ip)
{
    NyLPC_TInt16 s = query2label(i_packet, i_spos, obuflen, i_query);
    if (s == 0){
        return 0;
    }
    //レコード圧縮
    s = compressName(i_packet, i_spos, s);
    return writeAAAARecordByQueryData(i_packet,s,obuflen,ip);
}*/
/**
* AレコードクエリからAレコードレスポンスを書きだす。
*//*
static NyLPC_TInt16 writeAAAARecord(char* i_packet, NyLPC_TInt16 i_spos, NyLPC_TInt16 obuflen, const NyLPC_TChar* i_name, const struct NyLPC_TIPv4Addr* ip)
{
    NyLPC_TInt16 s = 1 + (NyLPC_TInt16)strlen(i_name) + 1 + 5 + 1;
    if (obuflen<i_spos + s + 4 + 4){
        return 0;
    }
    s = str2label(i_packet + i_spos, i_name) - 1;
    s += str2label(i_packet + i_spos + s, "local");
    //レコード圧縮
    s = compressName(i_packet, i_spos, s);
    return writeAAAARecordByQueryData(i_packet,s,obuflen,ip);
}*/

static NyLPC_TInt16 writeNSECRecordData(char* i_packet, NyLPC_TInt16 i_spos, NyLPC_TInt16 obuflen,NyLPC_TInt16 i_next_domain)
{
    NyLPC_TInt16 l=i_spos;
    (*(NyLPC_TUInt16*)(i_packet + l)) = NyLPC_HTONS(NyLPC_TDnsQuestion_QTYPR_NSEC);
    (*(NyLPC_TUInt16*)(i_packet + l + 2)) = NyLPC_HTONS(NyLPC_TDnsQuestion_QCLASS_IN | NyLPC_TDnsQuestion_QCLASS_CACHE_FLUSH);
    (*(NyLPC_TUInt32*)(i_packet + l + 4)) = NyLPC_HTONL(NyLPC_TcMDns_RES_TTL);
    l += 2 + 2 + 4;
    //A record header
    if (obuflen<l + 2 + 2 + 6){
        return 0;
    }
    //NSECレコードを書く
    *((NyLPC_TUInt16*)(i_packet + l)) = NyLPC_HTONS(2 + 6);
    l += 2;
    *(i_packet + l) = 0xc0;
    *(i_packet + l + 1) = (NyLPC_TUInt8)i_next_domain;
    l += 2;
    memcpy(i_packet + l, "\x00\x04\x00\x00\x00\x08", 6);
    return l + 6;
}
/**
* NSECレコードレスポンスを書きだす。
* IPv6わからんし。
*/
static NyLPC_TInt16 writeNSECRecord(char* i_packet, NyLPC_TInt16 i_spos, NyLPC_TInt16 obuflen, const NyLPC_TChar* i_name)
{
    //AnswerはAレコードのみ
    NyLPC_TInt16 s = 1 + (NyLPC_TInt16)strlen(i_name) + 1 + 5 + 1;
    if (obuflen<i_spos + s + 4 + 4){
        return 0;
    }
    s = str2label(i_packet + i_spos, i_name) - 1;
    s += str2label(i_packet + i_spos + s, "local");
    //レコード圧縮
    s = i_spos + s;//compressName(i_packet,i_spos,s);
    return writeNSECRecordData(i_packet,s,obuflen,i_spos);
}
/**
* NSECレコードレスポンスを書きだす。
* IPv6わからんし。
*/
static NyLPC_TInt16 writeNSECRecordByQuery(char* i_packet, NyLPC_TInt16 i_spos, NyLPC_TInt16 obuflen, const struct NyLPC_TDnsQuestion* i_query)
{
    NyLPC_TInt16 s;
    //AnswerはAレコードのみ
    s = query2label(i_packet, i_spos, obuflen, i_query);
    if (s == 0){
        return 0;
    }

    //レコード圧縮
    s = i_spos + s;
    //    l=compressName(i_packet,i_spos,s);
    return writeNSECRecordData(i_packet,s,obuflen,i_spos);
}

static NyLPC_TInt16 writeSdPtrRecord(const struct NyLPC_TMDnsServiceRecord* i_srvlec, char* i_packet, NyLPC_TInt16 i_spos, NyLPC_TInt16 obuflen)
{
    NyLPC_TInt16 l, s;
    NyLPC_TUInt16* rlen;
    //Header
    //    s=(NyLPC_TInt16)*(i_question->buf+i_question->qname_pos);
    //Headerの長さチェック
    if (obuflen<i_spos + 30 + 2 + 2 + 4){
        return 0;
    }
    //Header書込み
    memcpy(i_packet + i_spos, "\x09_services\x07_dns-sd\x04_udp\x05local\x00", 30);
    s = compressName(i_packet, i_spos, 30);
    (*(NyLPC_TUInt16*)(i_packet + s)) = NyLPC_HTONS(NyLPC_TDnsQuestion_QTYPR_PTR);
    (*(NyLPC_TUInt16*)(i_packet + s + 2)) = NyLPC_HTONS(NyLPC_TDnsQuestion_QCLASS_IN);
    (*(NyLPC_TUInt32*)(i_packet + s + 4)) = NyLPC_HTONL(NyLPC_TcMDns_STD_TTL);
    l = s + 2 + 2 + 4;

    //Resourceの書込み
    s = (NyLPC_TInt16)(1 + strlen(i_srvlec->protocol) + 1 + 5 + 1);//逆引き文字列の長さ(デリミタ×3+1)
    if (obuflen<s + l + 2){
        return 0;
    }
    rlen=(NyLPC_TUInt16*)(i_packet + l);
    l += 2;
    s = str2label(i_packet + l, i_srvlec->protocol) - 1;
    s += str2label(i_packet + l + s, "local");
    s = compressName(i_packet, l, s);//圧縮
    *rlen = NyLPC_ntohs(s - l);
    return s;
}

static NyLPC_TInt16 writePtrRecord(const struct NyLPC_TDnsRecord* i_recode, NyLPC_TInt16 i_sid, char* i_packet, NyLPC_TInt16 i_spos, NyLPC_TInt16 obuflen)
{
    NyLPC_TInt16 l, s;
    NyLPC_TUInt16* rlen;
    //Header:開始文字数(1)+プレフィクス(n)+終端(1)+local(5)+1
    s = (NyLPC_TInt16)(1 + strlen(i_recode->srv[i_sid].protocol) + 1 + 5 + 1);
    //Headerの長さチェック
    if (obuflen<i_spos + s + 2 + 2 + 4){
        return 0;
    }
    //Header書込み
    s = str2label(i_packet + i_spos, i_recode->srv[i_sid].protocol) - 1;
    s += str2label(i_packet + i_spos + s, "local");
    s = compressName(i_packet, i_spos, s);
    (*(NyLPC_TUInt16*)(i_packet + s)) = NyLPC_HTONS(NyLPC_TDnsQuestion_QTYPR_PTR);
    (*(NyLPC_TUInt16*)(i_packet + s + 2)) = NyLPC_HTONS(NyLPC_TDnsQuestion_QCLASS_IN);
    (*(NyLPC_TUInt32*)(i_packet + s + 4)) = NyLPC_HTONL(NyLPC_TcMDns_STD_TTL);
    l = s + 2 + 2 + 4;

    //Resourceの書込み
    s = (NyLPC_TInt16)(1 + strlen(i_recode->name) + 1 + strlen(i_recode->srv[i_sid].protocol) + 1 + 5 + 1);//逆引き文字列の長さ(デリミタ×3+1)
    if (obuflen<s + l + 2){
        return 0;
    }
    rlen = (NyLPC_TUInt16*)(i_packet + l);
    l += 2;
    s = str2label(i_packet + l, i_recode->name) - 1;
    s += str2label(i_packet + l + s, i_recode->srv[i_sid].protocol) - 1;
    s += str2label(i_packet + l + s, "local");
    s = compressName(i_packet, l, s);//圧縮
    (*rlen) = NyLPC_ntohs(s - l);
    return s;
}

static NyLPC_TInt16 writeSRVRecord(NyLPC_TcMDnsServer_t* i_inst, NyLPC_TInt16 i_sid, char* i_packet, NyLPC_TUInt16 i_spos, NyLPC_TInt16 obuflen)
{
    NyLPC_TInt16 l, s;
    NyLPC_TUInt16* rlen;

    //SRV Record
    s = writeSrvResourceHeader(i_packet, i_spos, obuflen, i_inst->_ref_record, i_sid, NyLPC_TDnsQuestion_QTYPR_SRV, NyLPC_TDnsQuestion_QCLASS_IN | NyLPC_TDnsQuestion_QCLASS_CACHE_FLUSH,NyLPC_TcMDns_RES_TTL);
    if (s == 0){
        return 0;
    }

    l = (NyLPC_TInt16)(1 + strlen(i_inst->_ref_record->a) + 1 + 5 + 1);//逆引き文字列の長さ(デリミタ×3+1)
    if (obuflen<s + 8 + l){
        return 0;
    }
    //IPADDR
    rlen = (NyLPC_TUInt16*)(i_packet + s);
    (*(NyLPC_TUInt16*)(i_packet + s + 2)) = NyLPC_HTONS(0);//Priority
    (*(NyLPC_TUInt16*)(i_packet + s + 4)) = NyLPC_HTONS(0);//Weight
    (*(NyLPC_TUInt16*)(i_packet + s + 6)) = NyLPC_HTONS(i_inst->_ref_record->srv[i_sid].port);//PORT
    l = 4 * 2 + s;
    s = str2label(i_packet + l, i_inst->_ref_record->a) - 1;
    s += str2label(i_packet + l + s, "local");
    s = compressName(i_packet, l, s);//圧縮
    (*rlen) = NyLPC_HTONS(2 + 2 + 2 + (s - l));
    return s;
}
static NyLPC_TInt16 writeTXTRecord(NyLPC_TcMDnsServer_t* i_inst, NyLPC_TInt16 i_sid, char* i_packet, NyLPC_TInt16 i_spos, NyLPC_TInt16 obuflen)
{
    NyLPC_TInt16 ret;
    NyLPC_TInt16 l;
    //Answer
    ret = writeSrvResourceHeader(i_packet, i_spos, obuflen, i_inst->_ref_record, i_sid, NyLPC_TDnsQuestion_QTYPR_TXT, NyLPC_TDnsQuestion_QCLASS_IN | NyLPC_TDnsQuestion_QCLASS_CACHE_FLUSH,NyLPC_TcMDns_STD_TTL);
    if (ret == 0){
        return 0;
    }
    //name.proto.localを返す。
    if (obuflen<ret + 2){
        return 0;
    }
    (*(NyLPC_TUInt16*)(i_packet + ret)) = NyLPC_ntohs(0);
    //proto.name.local.
    l = ret + 2;
    return l;
}






static void sendAnnounse(NyLPC_TcMDnsServer_t* i_inst)
{
    char* obuf;
    NyLPC_TUInt16 obuflen;
    NyLPC_TUInt16 l;
    int i,i2;
    for(i2=0;i2<i_inst->_ref_record->num_of_srv;i2++){
        //Bufferの取得
        obuf=NyLPC_iUdpSocket_allocSendBuf(i_inst->_socket,512,&obuflen,TIMEOUT_IN_MS);
        if(obuf==NULL){
            return;
        }
        l=setAnnounceHeader(obuf,1+i_inst->_ref_record->num_of_srv+4,0,0);
        //<Answer />
        //PTR
        l=writePtrRecord(i_inst->_ref_record,i2,obuf,l,obuflen);
        //SD-PTR
        if(l<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
        for (i = 0; i<i_inst->_ref_record->num_of_srv; i++){
            l = writeSdPtrRecord(&(i_inst->_ref_record->srv[i]), obuf, l, obuflen);
            if (l <= 0){
                NyLPC_OnErrorGoto(ERROR);
            }
        }
        //<Additional/>
        //SRV
        l=writeSRVRecord(i_inst,i2,obuf,l,obuflen);
        if(l<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
        //TXT
        l=writeTXTRecord(i_inst,i2,obuf,l,obuflen);
        if(l<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
        //Aレコード
        l=writeARecord(obuf,l,obuflen,i_inst->_ref_record->a,NyLPC_iUdpSocket_getSockIP(i_inst->_socket));
        if(l<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
//        //AAAAレコード
//        l=writeAAAARecord(obuf,l,obuflen,i_inst->_ref_record->a,&(i_inst->_super.uip_udp_conn.lipaddr));
//        if(l<=0){
//            NyLPC_OnErrorGoto(ERROR);
//        }
        //NSEC
        l=writeNSECRecord(obuf,l,obuflen,i_inst->_ref_record->a);
        if(l<=0){
            NyLPC_OnErrorGoto(ERROR);
        }          
        if(!NyLPC_iUdpSocket_psend(i_inst->_socket,&MDNS_MCAST_IPADDR,MDNS_MCAST_PORT,obuf,l)){
            NyLPC_OnErrorGoto(ERROR);
        }
    }
    return;
ERROR:
    NyLPC_iUdpSocket_releaseSendBuf(i_inst->_socket,obuf);
    return;
}



static void sendReply2(NyLPC_TcMDnsServer_t* i_inst, const struct NyLPC_TDnsHeader* i_dns_header, const struct NyLPC_TDnsQuestion* q)
{
    NyLPC_TInt16 ptr_recode;
    NyLPC_TInt16 i2;
    char* obuf;
    NyLPC_TUInt16 obuflen;
    NyLPC_TUInt16 l;
    //パケットヘッダの生成
    switch (q->qtype){
    case NyLPC_TDnsQuestion_QTYPR_SRV:
        //
        ptr_recode = NyLPC_TDnsRecord_getMatchSrvIdx(i_inst->_ref_record, q);
        if (ptr_recode<0){
            goto DROP;
        }
        //Bufferの取得
        obuf = NyLPC_iUdpSocket_allocSendBuf(i_inst->_socket, 512, &obuflen, 0);
        if (obuf == NULL){
            goto DROP;
        }
        //SRV,(TXT,A,AAAA,NSEC)
        l = setResponseHeader(obuf, i_dns_header, 1, 0, 3);
        l = writeSRVRecord(i_inst, ptr_recode, obuf, l, obuflen);
        if (l <= 0){
            NyLPC_OnErrorGoto(ERROR);
        }
        l = writeTXTRecord(i_inst, ptr_recode, obuf, l, obuflen);
        if (l <= 0){
            NyLPC_OnErrorGoto(ERROR);
        }
        //Aレコード
        l = writeARecord(obuf, l, obuflen, i_inst->_ref_record->a, NyLPC_iUdpSocket_getSockIP(i_inst->_socket));
        if (l <= 0){
            NyLPC_OnErrorGoto(ERROR);
        }
//        //AAAAレコード
//        l = writeAAAARecord(obuf, l, obuflen, i_inst->_ref_record->a, &(i_inst->_super.uip_udp_conn.lipaddr));
//        if (l <= 0){
//            NyLPC_OnErrorGoto(ERROR);
//        }
        //NSEC
        l = writeNSECRecord(obuf, l, obuflen, i_inst->_ref_record->a);
        if (l <= 0){
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
/*    case NyLPC_TDnsQuestion_QTYPR_AAAA:
        //自分宛？(name.local)
        if (!NyLPC_TDnsQuestion_isEqualName(q, i_inst->_ref_record->a, "")){
            goto DROP;
        }
        //Bufferの取得
        obuf = NyLPC_cUdpSocket_allocSendBuf(&(i_inst->_super), 512, &obuflen, 0);
        if (obuf == NULL){
            goto DROP;
        }
        //Headerのコピー
        //AAAA,(A,NSEC)
        l = setResponseHeader(obuf, i_dns_header, 1, 0, 2);
        //AAAAレコード
        l = writeAAAARecordByQuery(obuf, l, obuflen, q, &(i_inst->_super.uip_udp_conn.lipaddr));
        if (l <= 0){
            NyLPC_OnErrorGoto(ERROR);
        }
        //Aレコードのみ
        l = writeARecordByQuery(obuf, l, obuflen, q, &(i_inst->_super.uip_udp_conn.lipaddr));
        if (l <= 0){
            NyLPC_OnErrorGoto(ERROR);
        }
        //NSEC
        l = writeNSECRecordByQuery(obuf, l, obuflen, q);
        if (l <= 0){
            NyLPC_OnErrorGoto(ERROR);
        }
        break;*/
    case NyLPC_TDnsQuestion_QTYPR_A:
        //自分宛？(name.local)
        if (!NyLPC_TDnsQuestion_isEqualName(q, i_inst->_ref_record->a, "")){
            goto DROP;
        }
        //Bufferの取得
        obuf = NyLPC_iUdpSocket_allocSendBuf(i_inst->_socket, 512, &obuflen, 0);
        if (obuf == NULL){
            goto DROP;
        }
        //Headerのコピー
        l = setResponseHeader(obuf, i_dns_header, 1, 0, 1);
        //A、(NSEC
        l = writeARecordByQuery(obuf, l, obuflen, q, NyLPC_iUdpSocket_getSockIP(i_inst->_socket));
        if (l <= 0){
            NyLPC_OnErrorGoto(ERROR);
        }
        l = writeNSECRecordByQuery(obuf, l, obuflen, q);
        if (l <= 0){
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    case NyLPC_TDnsQuestion_QTYPR_PTR:
        //_service._dns-sd._udpかどうか
        if (NyLPC_TDnsQuestion_isEqualName(q, NULL, "_services._dns-sd._udp")){
            //Bufferの取得
            obuf = NyLPC_iUdpSocket_allocSendBuf(i_inst->_socket, 512, &obuflen, 0);
            if (obuf == NULL){
                goto DROP;
            }
            l = setResponseHeader(obuf, i_dns_header, i_inst->_ref_record->num_of_srv, 0, 0);
            for (i2 = 0; i2<i_inst->_ref_record->num_of_srv; i2++){
                l = writeSdPtrRecord(&(i_inst->_ref_record->srv[i2]), obuf, l, obuflen);
                if (l <= 0){
                    NyLPC_OnErrorGoto(ERROR);
                }
            }
        }
        else{
            //自分宛？(proto.local)
            ptr_recode = NyLPC_TDnsRecord_getMatchPtrIdx(i_inst->_ref_record, q);
            if (ptr_recode<0){
                goto DROP;
            }
            //Bufferの取得
            obuf = NyLPC_iUdpSocket_allocSendBuf(i_inst->_socket, 512, &obuflen, 0);
            if (obuf == NULL){
                goto DROP;
            }
            l = setResponseHeader(obuf, i_dns_header, 1, 0, 4);
            l = writePtrRecord(i_inst->_ref_record, ptr_recode, obuf, l, obuflen);
            if (l <= 0){
                NyLPC_OnErrorGoto(ERROR);
            }
            //SRV
            l = writeSRVRecord(i_inst, ptr_recode, obuf, l, obuflen);
            if (l <= 0){
                NyLPC_OnErrorGoto(ERROR);
            }
            //TXT
            l = writeTXTRecord(i_inst, ptr_recode, obuf, l, obuflen);
            if (l <= 0){
                NyLPC_OnErrorGoto(ERROR);
            }
            //Aレコード
            l = writeARecord(obuf, l, obuflen, i_inst->_ref_record->a,NyLPC_iUdpSocket_getSockIP(i_inst->_socket));
            if (l <= 0){
                NyLPC_OnErrorGoto(ERROR);
            }
//            //AAAAレコード
//            l = writeAAAARecord(obuf, l, obuflen, i_inst->_ref_record->a, &(i_inst->_super.uip_udp_conn.lipaddr));
//            if (l <= 0){
//                NyLPC_OnErrorGoto(ERROR);
//            }
            //NSEC
            l = writeNSECRecord(obuf, l, obuflen, i_inst->_ref_record->a);
            if (l <= 0){
                NyLPC_OnErrorGoto(ERROR);
            }
        }
        break;
    case NyLPC_TDnsQuestion_QTYPR_TXT:
        //自分宛？(proto.local)
        ptr_recode = NyLPC_TDnsRecord_getMatchSrvIdx(i_inst->_ref_record, q);
        if (ptr_recode<0){
            goto DROP;
        }
        //Bufferの取得
        obuf = NyLPC_iUdpSocket_allocSendBuf(i_inst->_socket, 512, &obuflen, 0);
        l = setResponseHeader(obuf, i_dns_header, 1, 0, 2);
        l = writeTXTRecord(i_inst, ptr_recode, obuf, l, obuflen);
        if (l <= 0){
            NyLPC_OnErrorGoto(ERROR);
        }
        //A recoad
        l = writeARecord(obuf, l, obuflen, i_inst->_ref_record->a,NyLPC_iUdpSocket_getSockIP(i_inst->_socket));
        if (l <= 0){
            NyLPC_OnErrorGoto(ERROR);
        }
//        //AAAAレコード
//        l = writeAAAARecord(obuf, l, obuflen, i_inst->_ref_record->a, &(i_inst->_super.uip_udp_conn.lipaddr));
//        if (l <= 0){
//            NyLPC_OnErrorGoto(ERROR);
//        }
        //NSEC
        l = writeNSECRecord(obuf, l, obuflen, i_inst->_ref_record->a);
        if (l <= 0){
            NyLPC_OnErrorGoto(ERROR);
        }
        break;
    default:
        goto DROP;
    }
    if (!NyLPC_iUdpSocket_psend(i_inst->_socket, &MDNS_MCAST_IPADDR, MDNS_MCAST_PORT, obuf, l)){
        NyLPC_OnErrorGoto(ERROR);
    }
    return;
ERROR:
    NyLPC_iUdpSocket_releaseSendBuf(i_inst->_socket, obuf);
DROP:
    return;
}
#define ST_INIT 1       //初期QUERY送信(省略)
#define ST_ANNOUNCE 2   //アナウンス
#define ST_WAIT 3       //待機


static NyLPC_TBool onPacket(NyLPC_TiUdpSocket_t* i_inst,const void* i_buf,const struct NyLPC_TIPv4RxInfo* i_info)
{
    NyLPC_TUInt16 in_len;
    NyLPC_TUInt16 num_of_query;
    struct NyLPC_TDnsQuestion q;
    NyLPC_TUInt16 s;
    NyLPC_TInt16 i;

    if(i_info->peer_port!=MDNS_MCAST_PORT || !NyLPC_TIPv4Addr_isEqual(&MDNS_MCAST_IPADDR,&i_info->ip)){
        return NyLPC_TBool_FALSE;
    }

    num_of_query=getNumberOfQuestion(i_buf,i_info->size);
    if(num_of_query==0){
        goto DROP;
    }
    in_len=sizeof(struct NyLPC_TDnsHeader);
    for(i=0;i<num_of_query;i++){
        //Queryのパース

        s=NyLPC_TDnsQuestion_parse(i_buf,i_info->size,in_len,&q);
        if(s==0){
            goto DROP;
        }
        in_len+=s;
        sendReply2((NyLPC_TcMDnsServer_t*)i_inst->_tag,(const struct NyLPC_TDnsHeader*)i_buf,&q);
    }
    //パケット処理終了
    return NyLPC_TBool_FALSE;
DROP:
    return NyLPC_TBool_FALSE;
}

static void onPeriodic(NyLPC_TiUdpSocket_t* i_inst)
{
    NyLPC_TcMDnsServer_t* inst=(NyLPC_TcMDnsServer_t*)i_inst->_tag;
    //Announce Timeout
    if(NyLPC_cStopwatch_isExpired(&((NyLPC_TcMDnsServer_t*)inst)->_periodic_sw)){
        switch(inst->_state){
        case ST_WAIT:
            inst->_state_val=0;
            inst->_state=ST_ANNOUNCE;// set Announce status
        case ST_ANNOUNCE:
            //アナウンス
            inst->_state_val++;
            if(inst->_state_val<=3){
                sendAnnounse(((NyLPC_TcMDnsServer_t*)inst));
                NyLPC_cStopwatch_startExpire((&((NyLPC_TcMDnsServer_t*)inst)->_periodic_sw),1000);
            }else{
                inst->_state=ST_WAIT;
                //TTL(msec)*1000*80%
                NyLPC_cStopwatch_startExpire((&((NyLPC_TcMDnsServer_t*)inst)->_periodic_sw),NyLPC_TcMDns_STD_TTL*1000*4/5);
            }
        }
    }
}

NyLPC_TBool NyLPC_cMDnsServer_initialize(
    NyLPC_TcMDnsServer_t* i_inst,const struct NyLPC_TDnsRecord* i_ref_record)
{
    NyLPC_cStopwatch_initialize(&(i_inst->_periodic_sw));
    NyLPC_cStopwatch_startExpire(&(i_inst->_periodic_sw),1000);
    i_inst->_socket=NyLPC_cNet_createUdpSocketEx(MDNS_MCAST_PORT,NyLPC_TSocketType_UDP_NOBUF);
    i_inst->_socket->_tag=i_inst;
    NyLPC_iUdpSocket_setOnRxHandler(i_inst->_socket,onPacket);
    NyLPC_iUdpSocket_setOnPeriodicHandler(i_inst->_socket,onPeriodic);
    NyLPC_iUdpSocket_joinMulticast(i_inst->_socket,&MDNS_MCAST_IPADDR);
    i_inst->_state=ST_WAIT;
    i_inst->_state_val=0;
    i_inst->_ref_record=i_ref_record;
    return NyLPC_TBool_TRUE;
}
void NyLPC_cMDnsServer_finalize(
    NyLPC_TcMDnsServer_t* i_inst)
{
    NyLPC_iUdpSocket_finalize(i_inst->_socket);
    NyLPC_cStopwatch_finalize(&(i_inst->_periodic_sw));
}



