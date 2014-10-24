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
#include "NyLPC_uipService.h"
#include "NyLPC_http.h"
#include "NyLPC_utils.h"
#include <stdio.h>
#include <string.h>


/**
 * mDNSのポート番号
 */
#define MDNS_MCAST_PORT         5353
static const struct NyLPC_TIPv4Addr MDNS_MCAST_IPADDR=NyLPC_TIPv4Addr_pack(224,0,0,251);
#define TIMEOUT_IN_MS       1000
#define NyLPC_TcMDns_TTL (30*60)	//30min


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
	const char* buf;		//Questionパケットの先頭
	NyLPC_TUInt16 buf_len;	//パケット長さ
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
#define NyLPC_TDnsQuestion_QCLASS_IN    1
#define NyLPC_TDnsQuestion_QCLASS_CH    3
#define NyLPC_TDnsQuestion_QCLASS_HS    4
#define NyLPC_TDnsQuestion_QTYPR_SRV    33

/**************************************************
 * TLabelCache
 **************************************************/

struct TLabelCache
{
    NyLPC_TUInt16 idx;
    const char* str;
};
static void TLabelCache_reset(struct TLabelCache* i_struct)
{
    i_struct->idx=0;
    i_struct->str=NULL;
}
static NyLPC_TInt16 TLabelCache_compress(struct TLabelCache* i_struct,NyLPC_TChar* i_src)
{
    NyLPC_TInt16 s;
    NyLPC_TInt16 k;
    const NyLPC_TChar* q;
    const NyLPC_TChar* d;
    //初めてのインデクスは保存
    if(i_struct->idx==0){
        i_struct->idx=12;
        i_struct->str=i_src;
    }else{
        d=i_struct->str;
        do{
            q=i_src;
            do{
                if(strcmp(q,d)==0){
                    //一致,インデクスを計算
                    s=(NyLPC_TInt16)(q-i_src);
                    k=(NyLPC_TInt16)(d-i_struct->str);
                    //新しい長さを返す
                    *((NyLPC_TInt16*)(i_src+s))=NyLPC_HTONS(0xC000|(i_struct->idx+k));
                    return s+2;
                }
                q+=*q+1;
            }while(*q!=0);
            d+=*d+1;
        }while(*d!=0);
    }
    return (NyLPC_TInt16)(strlen(i_src)+1);
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
 * nameフィールドの文字列圧縮を解除して圧縮後のテキストポインタを返します。
 */
static const char* getExtractNamePos(const char* i_packet_buf, const char* i_spos)
{
	NyLPC_TUInt8 limit=0;
	const char* s = i_spos;// question->buf + pos;//クエリの解析位置
	for (;;){
		switch (*(const NyLPC_TUInt8*)s){
		case 0x00:
			//queryが先に終了に到達するのはおかしい。
			return NULL;
		case 0xc0:
			s = i_packet_buf + *((const NyLPC_TUInt8*)s + 1);//参照先にジャンプ
			if (i_spos<=s){
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
 * [i_name].[i_protocol].localをquestionと比較します。i_nameは省略ができます。
 * @return 等しい場合true
 */
static NyLPC_TBool NyLPC_TDnsQuestion_isEqualName(const struct NyLPC_TDnsQuestion* question, const char* i_name, const char* i_protocol)
{
	NyLPC_TUInt8 tmp;
	const char* p;						//プロトコル文字列の解析開始位置
	const char* s = question->buf + (NyLPC_TUInt8)question->qname_pos; //クエリの解析位置

	//Domain
	if (i_name != NULL){
		//0xc0参照の解決
		s = getExtractNamePos(question->buf, s);
		if (s == NULL){
			return NyLPC_TBool_FALSE;
		}
		tmp = (NyLPC_TUInt8)strlen(i_name);
		if (tmp != *s || memcmp(s + 1, i_name, tmp) != 0){
			return NyLPC_TBool_FALSE;
		}
		s += (*s) + 1;
	}else{
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
			qlen+=2;
			break;
		default:
			qlen++;
			continue;
		}
		o_val->buf = i_packet;
		o_val->buf_len = i_packet_len;
		o_val->qname_pos = i_parse_start;
		o_val->qtype = NyLPC_ntohs(*(NyLPC_TUInt16*)(i_packet+i_parse_start + qlen));
		o_val->qclass = NyLPC_ntohs(*(NyLPC_TUInt16*)(i_packet+i_parse_start + qlen + sizeof(NyLPC_TUInt16)));
		return qlen + 4;
	}
	return 0;
}

/**
 * DNSレコードのPRTフィールドとDNSラベル文字列を比較する。
 */
static NyLPC_TInt16 NyLPC_TDnsRecord_getMatchPtrIdx(const struct NyLPC_TDnsRecord* i_struct,const struct NyLPC_TDnsQuestion* question)
{
    NyLPC_TInt16 i;
    for(i=0;i<i_struct->num_of_srv;i++){
        if(NyLPC_TDnsQuestion_isEqualName(question,NULL,i_struct->srv[i].protocol)){
            return i;
        }
    }
    return -1;
}
static NyLPC_TInt16 NyLPC_TDnsRecord_getMatchSrvIdx(const struct NyLPC_TDnsRecord* i_struct,const struct NyLPC_TDnsQuestion* question)
{
    NyLPC_TInt16 i;
    for(i=0;i<i_struct->num_of_srv;i++){
    	if(NyLPC_TDnsQuestion_isEqualName(question,i_struct->name,i_struct->srv[i].protocol)){
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
static NyLPC_TInt16 str2label(NyLPC_TChar* buf,const NyLPC_TChar* name)
{
    //proto文字列の変換
    NyLPC_TChar* lp;
    const NyLPC_TChar* n=name;
    NyLPC_TChar* b=buf;
    while(*n!='\0'){
        lp=b;
        b++;
        for(;strchr(".\0",*n)==NULL;){
            *b=*n;
            b++;
            n++;
        }
        *lp=(char)(b-lp-1);
        if(*n!='\0'){
            n++;
        }
    }
    *b='\0';
    b++;
    return b-buf;
}

/**
 * ResourceHeaderのライタ
 */
static NyLPC_TInt16 writeResourceHeader(char* buf,NyLPC_TInt16 len,const char* i_name,NyLPC_TUInt16 i_type,NyLPC_TUInt16 i_class)
{
    NyLPC_TInt16 s;
    NyLPC_TInt16 l=1+(NyLPC_TInt16)strlen(i_name)+1+5+1;
    if(len<l+4+4){
        return 0;
    }
    s=str2label(buf,i_name);
    str2label(buf+s-1,"local");
    (*(NyLPC_TUInt16*)(buf+l))=NyLPC_HTONS(i_type);
    (*(NyLPC_TUInt16*)(buf+l+2))=NyLPC_HTONS(i_class);
    (*(NyLPC_TUInt32*)(buf+l+4))=NyLPC_HTONL(NyLPC_TcMDns_TTL);
    return l+2+2+4;
}

inline static NyLPC_TInt16 writeSrvResourceHeader(char* buf,NyLPC_TInt16 len,const struct NyLPC_TDnsRecord* i_recode,int i_sid,NyLPC_TUInt16 i_type,NyLPC_TUInt16 i_class,struct TLabelCache* i_ca)
{
    NyLPC_TInt16 l=(NyLPC_TInt16)(1+strlen(i_recode->name)+1+strlen(i_recode->srv[i_sid].protocol)+1+5+1);
    if(len<l+2+2+4){
        return 0;
    }
    l=str2label(buf,i_recode->name)-1;
    l+=str2label(buf+l,i_recode->srv[i_sid].protocol)-1;
    l+=str2label(buf+l,"local");
    l=TLabelCache_compress(i_ca,buf);//圧縮
    (*(NyLPC_TUInt16*)(buf+l))=NyLPC_HTONS(i_type);
    (*(NyLPC_TUInt16*)(buf+l+2))=NyLPC_HTONS(i_class);
    (*(NyLPC_TUInt32*)(buf+l+4))=NyLPC_HTONL(NyLPC_TcMDns_TTL);
    return l+2+2+4;
}

inline static NyLPC_TUInt16 setResponseHeader(char* obuf,const struct NyLPC_TDnsHeader* i_in_dns_header,NyLPC_TUInt16 i_an_count,NyLPC_TUInt16 i_ns_count,NyLPC_TUInt16 i_ar_count)
{
    struct NyLPC_TDnsHeader* p=(struct NyLPC_TDnsHeader*)obuf;
    if(i_in_dns_header!=NULL){
        memcpy(p,i_in_dns_header,sizeof(struct NyLPC_TDnsHeader));
        p->flag=p->flag | NyLPC_HTONS(NyLPC_TDnsHeader_FLAG_MASK_QR|NyLPC_TDnsHeader_FLAG_MASK_AA);
        p->flag=p->flag & NyLPC_HTONS(~(NyLPC_TDnsHeader_FLAG_MASK_RECODE|NyLPC_TDnsHeader_FLAG_MASK_TC|NyLPC_TDnsHeader_FLAG_MASK_RA));
    }else{
        p->flag=0;
        p->id=0;
    }
    p->qd=0;
    p->an=NyLPC_HTONS(i_an_count);
    p->ns=NyLPC_HTONS(i_ns_count);
    p->ar=NyLPC_HTONS(i_ar_count);
    return sizeof(struct NyLPC_TDnsHeader);
}
inline static NyLPC_TInt16 writeARecord(char* obuf,NyLPC_TInt16 obuflen,const NyLPC_TChar* a_rec,const struct NyLPC_TIPv4Addr* ip)
{
    NyLPC_TInt16 ret;
    //AnswerはAレコードのみ
    //A record header
    ret=writeResourceHeader(obuf,obuflen,a_rec,NyLPC_TDnsQuestion_QTYPR_A,NyLPC_TDnsQuestion_QCLASS_IN);
    if(ret==0 || obuflen-ret<8){
        return 0;
    }
    //Aレコードを書く
    //IPADDR
    (*(NyLPC_TUInt16*)(obuf+ret))=NyLPC_HTONS(4);
    (*(NyLPC_TUInt32*)(obuf+ret+2))=ip->v;
    return ret+6;
}


static NyLPC_TInt16 writeSdPtrRecord(const struct NyLPC_TDnsQuestion* i_question,const struct NyLPC_TMDnsServiceRecord* i_srvlec,char* obuf,NyLPC_TInt16 obuflen,struct TLabelCache* i_ca)
{
    NyLPC_TInt16 l,s;
    //Header
    s=(NyLPC_TInt16)*(i_question->buf+i_question->qname_pos);
    //Headerの長さチェック
    if(obuflen<30+2+2+4){
        return 0;
    }
    //Header書込み
    memcpy(obuf,"\x09_services\x07_dns-sd\x04_udp\x05local\x00",30);
    s=TLabelCache_compress(i_ca,obuf);
    (*(NyLPC_TUInt16*)(obuf+s))=NyLPC_HTONS(NyLPC_TDnsQuestion_QTYPR_PTR);
    (*(NyLPC_TUInt16*)(obuf+s+2))=NyLPC_HTONS(NyLPC_TDnsQuestion_QCLASS_IN);
    (*(NyLPC_TUInt32*)(obuf+s+4))=NyLPC_HTONL(NyLPC_TcMDns_TTL);
    l=s+2+2+4;

    //Resourceの書込み
    s=(NyLPC_TInt16)(1+strlen(i_srvlec->protocol)+1+5+1);//逆引き文字列の長さ(デリミタ×3+1)
    if(obuflen<s+l+2){
        return 0;
    }
    (*(NyLPC_TUInt16*)(obuf+l))=NyLPC_ntohs(s);
    l+=2;
    s=str2label(obuf+l,i_srvlec->protocol)-1;
    s+=str2label(obuf+s+l,"local");
    s=TLabelCache_compress(i_ca,obuf+l);//圧縮
    return l+s;
}
static NyLPC_TInt16 writePtrRecord(const struct NyLPC_TDnsRecord* i_recode,NyLPC_TInt16 i_sid,char* obuf,NyLPC_TInt16 obuflen,struct TLabelCache* i_ca)
{
    NyLPC_TInt16 l,s;
    NyLPC_TUInt16* rlen;
    //Header:開始文字数(1)+プレフィクス(n)+終端(1)+local(5)+1
    s=(NyLPC_TInt16)(1+strlen(i_recode->srv[i_sid].protocol)+1+5+1);
    //Headerの長さチェック
    if(obuflen<s+2+2+4){
        return 0;
    }
    //Header書込み
    s=str2label(obuf,i_recode->srv[i_sid].protocol)-1;
    s+=str2label(obuf+s,"local");
    s=TLabelCache_compress(i_ca,obuf);
    (*(NyLPC_TUInt16*)(obuf+s))=NyLPC_HTONS(NyLPC_TDnsQuestion_QTYPR_PTR);
    (*(NyLPC_TUInt16*)(obuf+s+2))=NyLPC_HTONS(NyLPC_TDnsQuestion_QCLASS_IN);
    (*(NyLPC_TUInt32*)(obuf+s+4))=NyLPC_HTONS(NyLPC_TcMDns_TTL);
    l=s+2+2+4;

    //Resourceの書込み
    s=1+strlen(i_recode->name)+1+strlen(i_recode->srv[i_sid].protocol)+1+5+1;//逆引き文字列の長さ(デリミタ×3+1)
    if(obuflen<s+l+2){
        return 0;
    }
    rlen=(NyLPC_TUInt16*)(obuf+l);
    l+=2;
    s=str2label(obuf+l,i_recode->name)-1;
    s+=str2label(obuf+l+s,i_recode->srv[i_sid].protocol)-1;
    s+=str2label(obuf+l+s,"local");
    s=TLabelCache_compress(i_ca,obuf+l);//圧縮
    (*rlen)=NyLPC_ntohs(s);
    return l+s;
}


static NyLPC_TInt16 writeSRVRecord(NyLPC_TcMDnsServer_t* i_inst,NyLPC_TInt16 i_sid,char* obuf,NyLPC_TInt16 obuflen,struct TLabelCache* i_ca)
{
    NyLPC_TInt16 l,s;
    NyLPC_TUInt16* rlen;

    //SRV Record
    s=writeSrvResourceHeader(obuf,obuflen,i_inst->_ref_record,i_sid,NyLPC_TDnsQuestion_QTYPR_SRV,NyLPC_TDnsQuestion_QCLASS_IN,i_ca);
    if(s==0){
        return 0;
    }

    l=1+strlen(i_inst->_ref_record->a)+1+5+1;//逆引き文字列の長さ(デリミタ×3+1)
    if(obuflen-s-l<8){
        return 0;
    }
    //IPADDR
    rlen=(NyLPC_TUInt16*)(obuf+s);
    (*(NyLPC_TUInt16*)(obuf+s+2))=NyLPC_HTONS(0);//Priority
    (*(NyLPC_TUInt16*)(obuf+s+4))=NyLPC_HTONS(0);//Weight
    (*(NyLPC_TUInt16*)(obuf+s+6))=NyLPC_HTONS(i_inst->_ref_record->srv[i_sid].port);//PORT
    l=4*2+s;
    s=str2label(obuf+l,i_inst->_ref_record->a)-1;
    s+=str2label(obuf+l+s,"local");
    s=TLabelCache_compress(i_ca,obuf+l);//圧縮
    (*rlen)=NyLPC_HTONS(2+2+2+s);
    return l+s;
}
static NyLPC_TInt16 writeTXTRecord(NyLPC_TcMDnsServer_t* i_inst,NyLPC_TInt16 i_sid,char* obuf,NyLPC_TInt16 obuflen,struct TLabelCache* i_ca)
{
    NyLPC_TInt16 ret;
    NyLPC_TInt16 l;
    //Answer
    ret=writeSrvResourceHeader(obuf,obuflen,i_inst->_ref_record,i_sid,NyLPC_TDnsQuestion_QTYPR_TXT,NyLPC_TDnsQuestion_QCLASS_IN,i_ca);
    if(ret==0){
        return 0;
    }
    //name.proto.localを返す。
    if(obuflen-ret<2){
        return 0;
    }
    (*(NyLPC_TUInt16*)(obuf+ret))=NyLPC_ntohs(0);
    //proto.name.local.
    l=ret+2;
    return l;
}






static void sendAnnounse(NyLPC_TcMDnsServer_t* i_inst)
{
    char* obuf;
    NyLPC_TUInt16 obuflen;
    NyLPC_TUInt16 l,s;
    int i2;
    struct TLabelCache cache;
    for(i2=0;i2<i_inst->_ref_record->num_of_srv;i2++){
        TLabelCache_reset(&cache);
        //Bufferの取得
        obuf=NyLPC_cUdpSocket_allocSendBuf(&(i_inst->_super),512,&obuflen,TIMEOUT_IN_MS);
        if(obuf==NULL){
            return;
        }
        l=setResponseHeader(obuf,NULL,1,0,3);
        s=writePtrRecord(i_inst->_ref_record,i2,obuf+l,obuflen-l,&cache);
        if(s<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
        l+=s;
        s=writeSRVRecord(i_inst,i2,obuf+l,obuflen-l,&cache);
        if(s<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
        l+=s;
        s=writeTXTRecord(i_inst,i2,obuf+l,obuflen-l,&cache);
        if(s<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
        l+=s;
        //Aレコード
        s=writeARecord(obuf+l,obuflen-l,i_inst->_ref_record->a,&(i_inst->_super.uip_udp_conn.lipaddr));
        if(s<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
        l+=s;
        if(!NyLPC_cUdpSocket_psend(&(i_inst->_super),&MDNS_MCAST_IPADDR,MDNS_MCAST_PORT,obuf,l)){
            NyLPC_OnErrorGoto(ERROR);
        }
    }
    return;
ERROR:
    NyLPC_cUdpSocket_releaseSendBuf(&(i_inst->_super),obuf);
    return;
}



static void sendReply2(NyLPC_TcMDnsServer_t* i_inst,const struct NyLPC_TDnsHeader* i_dns_header,const struct NyLPC_TDnsQuestion* q)
{
    NyLPC_TInt16 ptr_recode;
    NyLPC_TInt16 i2;
    char* obuf;
    NyLPC_TUInt16 obuflen;
    NyLPC_TUInt16 l,s;
    struct TLabelCache cache;
    TLabelCache_reset(&cache);
    //パケットヘッダの生成
    switch(q->qtype){
    case NyLPC_TDnsQuestion_QTYPR_SRV:
        //SRV,A record
        ptr_recode=NyLPC_TDnsRecord_getMatchSrvIdx(i_inst->_ref_record,q);
        if(ptr_recode<0){
            goto DROP;
        }
        //Bufferの取得
        obuf=NyLPC_cUdpSocket_allocSendBuf(&(i_inst->_super),512,&obuflen,0);
        if(obuf==NULL){
            goto DROP;
        }
        l=setResponseHeader(obuf,i_dns_header,1,0,2);
        s=writeSRVRecord(i_inst,ptr_recode,obuf+l,obuflen-l,&cache);
        if(s<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
        l+=s;
        s=writeTXTRecord(i_inst,ptr_recode,obuf+l,obuflen-l,&cache);
        if(s<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
        l+=s;
        //Aレコード
        s=writeARecord(obuf+l,obuflen-l,i_inst->_ref_record->a,&(i_inst->_super.uip_udp_conn.lipaddr));
        if(s<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
        l+=s;
        break;
    case NyLPC_TDnsQuestion_QTYPR_A:
        //自分宛？(name.local)
        if(!NyLPC_TDnsQuestion_isEqualName(q,i_inst->_ref_record->a,"")){
            goto DROP;
        }
        //Bufferの取得
        obuf=NyLPC_cUdpSocket_allocSendBuf(&(i_inst->_super),512,&obuflen,0);
        if(obuf==NULL){
            goto DROP;
        }
        //Headerのコピー
        l=setResponseHeader(obuf,i_dns_header,1,0,0);
        //Aレコードのみ
        s=writeARecord(obuf+l,obuflen-l,i_inst->_ref_record->a,&(i_inst->_super.uip_udp_conn.lipaddr));
        if(s<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
        l+=s;
        break;
    case NyLPC_TDnsQuestion_QTYPR_PTR:
    	//_service._dns-sd._udpかどうか
        if(NyLPC_TDnsQuestion_isEqualName(q,NULL,"_services._dns-sd._udp")){
            //Bufferの取得
            obuf=NyLPC_cUdpSocket_allocSendBuf(&(i_inst->_super),512,&obuflen,0);
            if(obuf==NULL){
                goto DROP;
            }
            l=setResponseHeader(obuf,i_dns_header,i_inst->_ref_record->num_of_srv,0,0);
            for(i2=0;i2<i_inst->_ref_record->num_of_srv;i2++){
                s=writeSdPtrRecord(q,&(i_inst->_ref_record->srv[i2]),obuf+l,obuflen-l,&cache);
                if(s<=0){
                    NyLPC_OnErrorGoto(ERROR);
                }
                l+=s;
            }
        }else{
            //自分宛？(proto.local)
            ptr_recode=NyLPC_TDnsRecord_getMatchPtrIdx(i_inst->_ref_record,q);
            if(ptr_recode<0){
                goto DROP;
            }
            //Bufferの取得
            obuf=NyLPC_cUdpSocket_allocSendBuf(&(i_inst->_super),512,&obuflen,0);
            if(obuf==NULL){
                goto DROP;
            }
            l=setResponseHeader(obuf,i_dns_header,1,0,3);
            s=writePtrRecord(i_inst->_ref_record,ptr_recode,obuf+l,obuflen-l,&cache);
            if(s<=0){
                NyLPC_OnErrorGoto(ERROR);
            }
            l+=s;
            s=writeSRVRecord(i_inst,ptr_recode,obuf+l,obuflen-l,&cache);
            if(s<=0){
                NyLPC_OnErrorGoto(ERROR);
            }
            l+=s;
            s=writeTXTRecord(i_inst,ptr_recode,obuf+l,obuflen-l,&cache);
            if(s<=0){
                NyLPC_OnErrorGoto(ERROR);
            }
            l+=s;
            //Aレコード
            s=writeARecord(obuf+l,obuflen-l,i_inst->_ref_record->a,&(i_inst->_super.uip_udp_conn.lipaddr));
            if(s<=0){
                NyLPC_OnErrorGoto(ERROR);
            }
            l+=s;
        }
        break;
    case NyLPC_TDnsQuestion_QTYPR_TXT:
        //自分宛？(proto.local)
        ptr_recode=NyLPC_TDnsRecord_getMatchSrvIdx(i_inst->_ref_record,q);
        if(ptr_recode<0){
            goto DROP;
        }
        //Bufferの取得
        obuf=NyLPC_cUdpSocket_allocSendBuf(&(i_inst->_super),512,&obuflen,0);
        l=setResponseHeader(obuf,i_dns_header,1,0,1);
        s=writeTXTRecord(i_inst,ptr_recode,obuf+l,obuflen-l,&cache);
        if(s<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
        l+=s;
        s=writeARecord(obuf+l,obuflen-l,i_inst->_ref_record->a,&(i_inst->_super.uip_udp_conn.lipaddr));
        if(s<=0){
            NyLPC_OnErrorGoto(ERROR);
        }
        l+=s;
        break;
    default:
        goto DROP;
    }
    if(!NyLPC_cUdpSocket_psend(&(i_inst->_super),&MDNS_MCAST_IPADDR,MDNS_MCAST_PORT,obuf,l)){
        NyLPC_OnErrorGoto(ERROR);
    }
    return;
ERROR:
    NyLPC_cUdpSocket_releaseSendBuf(&(i_inst->_super),obuf);
DROP:
    return;
}
#define CTRL_FLAG_INIT           0x00
#define CTRL_FLAG_STARTED        0x80
#define CTRL_FLAG_STOP_REQUESTED 0x40
#define CTRL_FLAG_PROCESS_PACKET 0x20   //パケット処理中の間1

static NyLPC_TBool onPacket(NyLPC_TcUdpSocket_t* i_inst,const void* i_buf,const struct NyLPC_TIPv4RxInfo* i_info)
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
        sendReply2((NyLPC_TcMDnsServer_t*)i_inst,(const struct NyLPC_TDnsHeader*)i_buf,&q);
    }
    //パケット処理終了
    return NyLPC_TBool_FALSE;
DROP:
    return NyLPC_TBool_FALSE;
}

static void onPeriodic(NyLPC_TcUdpSocket_t* i_inst)
{

    if(NyLPC_cStopwatch_isExpired(&((NyLPC_TcMDnsServer_t*)i_inst)->_periodic_sw)){
        //アナウンス
        sendAnnounse(((NyLPC_TcMDnsServer_t*)i_inst));
        //TTL(msec)*1000*80%
        NyLPC_cStopwatch_startExpire((&((NyLPC_TcMDnsServer_t*)i_inst)->_periodic_sw),NyLPC_TcMDns_TTL*1000/2);
    }
}

NyLPC_TBool NyLPC_cMDnsServer_initialize(
    NyLPC_TcMDnsServer_t* i_inst,const struct NyLPC_TDnsRecord* i_ref_record)
{
    NyLPC_cStopwatch_initialize(&(i_inst->_periodic_sw));
    NyLPC_cStopwatch_startExpire(&(i_inst->_periodic_sw),1000);
    NyLPC_cUdpSocket_initialize(&(i_inst->_super),MDNS_MCAST_PORT,NULL,0);
    NyLPC_cUdpSocket_setOnRxHandler(&(i_inst->_super),onPacket);
    NyLPC_cUdpSocket_setOnPeriodicHandler(&(i_inst->_super),onPeriodic);
    NyLPC_cUdpSocket_joinMulticast(&(i_inst->_super),&MDNS_MCAST_IPADDR);
    i_inst->_ref_record=i_ref_record;
    return NyLPC_TBool_TRUE;
}
void NyLPC_cMDnsServer_finalize(
    NyLPC_TcMDnsServer_t* i_inst)
{
    NyLPC_cUdpSocket_finalize(&(i_inst->_super));
    NyLPC_cStopwatch_finalize(&(i_inst->_periodic_sw));
}



