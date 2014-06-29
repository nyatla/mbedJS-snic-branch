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
#include "NyLPC_cRingBuffer.h"


#if NyLPC_ARCH==NyLPC_ARCH_FREERTOS
#elif NyLPC_ARCH==NyLPC_ARCH_WIN32
void NyLPC_cRingBuffer_dump(NyLPC_TcRingBuffer_t* i_inst)
{
    NyLPC_TUInt8* s=((NyLPC_TUInt8*)(i_inst+1));//バッファ開始位置
    int i;
    if(i_inst->ro<=i_inst->wo){
        for(i=0;i<i_inst->ro;i++){
            printf("-- ");
        }
        for(i=i_inst->ro;i<i_inst->wo;i++){
            printf("%02X ",*(s+i));
        }
        for(i=i_inst->wo;i<i_inst->bl;i++){
            printf("-- ");
        }
    }else{
        for(i=0;i<i_inst->wo;i++){
            printf("%02X ",*(s+i));
        }
        for(i=i_inst->wo;i<i_inst->ro;i++){
            printf("-- ");
        }
        for(i=i_inst->ro;i<i_inst->bl;i++){
            printf("%02X ",*(s+i));
        }
    }
    printf("\n");
}
#else
#endif

void NyLPC_cRingBuffer_reset(NyLPC_TcRingBuffer_t* i_inst)
{
    i_inst->ro=i_inst->wo=0;
}

NyLPC_TInt16 NyLPC_cRingBuffer_getReadableSize(NyLPC_TcRingBuffer_t* i_inst)
{
    volatile NyLPC_TUInt16 wo=(i_inst)->wo;
    volatile NyLPC_TUInt16 ro=(i_inst)->ro;
    if(wo>=ro)
    {
        return wo-ro;
    }else{
        return (i_inst)->bl-ro;
    }
}

NyLPC_TInt16 NyLPC_cRingBuffer_getWritableSize(const NyLPC_TcRingBuffer_t* i_inst)
{
    volatile NyLPC_TUInt16 wo=(i_inst)->wo;
    volatile NyLPC_TUInt16 ro=(i_inst)->ro;
    if(wo>=ro){
        //書込み可能サイズの計算
        return i_inst->bl-(wo-ro)-1;
    }else{
        return ro-wo-1;
    }

}

void NyLPC_cRingBuffer_initialize(NyLPC_TcRingBuffer_t* i_inst,void* i_buf,NyLPC_TUInt16 sizeof_buf)
{
    //バッファの開始位置と終了位置の計算
    i_inst->bl=sizeof_buf;
    i_inst->ro=0;
    i_inst->wo=0;
    i_inst->buf=i_buf;
}

int NyLPC_cRingBuffer_write(NyLPC_TcRingBuffer_t* i_inst,NyLPC_TUInt8* i_data,const int i_len)
{
    NyLPC_TUInt8* s=(NyLPC_TUInt8*)i_inst->buf;
    NyLPC_TUInt8* p;
    NyLPC_TUInt16 wo=i_inst->wo;
    NyLPC_TUInt16 ro=i_inst->ro;
    NyLPC_TUInt16 wsize;//書込み可能サイズ
    int l,i;
    NyLPC_TUInt16 rw;

    if(wo>=ro){
        //書込み可能サイズの計算
        wsize=i_inst->bl-(wo-ro)-1;
        if(wsize<1){
            return 0;
        }
        //書込みサイズの調整
        if(wsize>i_len){
            wsize=i_len;
        }
        //右側の書込みサイズの計算
        rw=i_inst->bl-wo;
        l=(wsize<rw)?wsize:rw;
        //書込みポインタを設定
        p=(s+wo);
        for(i=l-1;i>=0;i--){
            *(p++)=*(i_data++);
        }
        //書込み位置の調整
        wo=(wo+l)%i_inst->bl;
        l=wsize-l;//lに左側の書込みサイズストア
    }else{
        wsize=ro-wo-1;
        if(wsize>i_len){
            wsize=i_len;
        }
        l=wsize;
    }
    if(l>0){
        //左側の書込み
        p=(s+wo);
        for(i=l-1;i>=0;i--){
            *(p++)=*(i_data++);
        }
        wo+=l;
    }
    i_inst->wo=wo;
    return wsize;
}
//読出しポインタを得る。
NyLPC_TUInt8* NyLPC_cRingBuffer_pread(NyLPC_TcRingBuffer_t* i_inst,NyLPC_TUInt16 *len)
{
    NyLPC_TUInt16 ro=i_inst->ro;
    *len=NyLPC_cRingBuffer_getReadableSize(i_inst);
    return ((NyLPC_TUInt8*)(i_inst->buf))+ro;
}

//前方シークする。
void NyLPC_cRingBuffer_preadSeek(NyLPC_TcRingBuffer_t* i_inst,NyLPC_TUInt16 i_seek)
{
    NyLPC_Assert(NyLPC_cRingBuffer_getReadableSize(i_inst)>=i_seek);
    i_inst->ro=(i_inst->ro+i_seek)%i_inst->bl;
}

#define TEST
#ifndef TEST
void main(void)
{
    NyLPC_TUInt16 l;
    NyLPC_TUInt8* b;
    int c;
    char buf[sizeof(NyLPC_TcRingBuffer_t)+5];
    NyLPC_TcRingBuffer_t* s;
    s=NyLPC_cRingBuffer_initialize(buf,sizeof(buf));
    for(;;){
        b=NyLPC_cRingBuffer_getReadPtr(s,&l);
        printf("readable:%d\n",l);
        c=NyLPC_cRingBuffer_write(s,"0123456789",3);
        NyLPC_cRingBuffer_dump(s);
        b=NyLPC_cRingBuffer_getReadPtr(s,&l);
        printf("readable:%d\n",l);
        NyLPC_cRingBuffer_seekReadPtr(s,(l>1)?l-1:1);
        printf("read:%d\n",(l>1)?l-1:1);
        NyLPC_cRingBuffer_dump(s);
    }

}
#endif
