#include "NyLPC_config.h"
#if NyLPC_MCU==NyLPC_MCU_LPC4088 || NyLPC_MCU==NyLPC_MCU_LPC17xx
#include "LPC17xx_IAP.h"
#include "NyLPC_cOnchipFlashWriter.h"


/**
 * 指定したアドレスが、オンチップフラッシュかどうか
 */
NyLPC_TBool NyLPC_cOnchipFlashWriter_isOnchipFlash(const void* i_addr)
{
    unsigned long snum;
    return LPC17xx_IAP_addr2Sector(i_addr,&snum)==LPC17xx_IAP_CMD_SUCCESS;
}

NyLPC_TUInt8 _work[256];


NyLPC_TBool NyLPC_cOnchipFlashWriter_write(const void* i_dest,const void* i_src,NyLPC_TUInt32 i_size)
{
    NyLPC_TUInt32 size;
    const char* src;
    NyLPC_TUInt32 snum;
    const char* fblock_addr;
    const char* dest_addr;
    NyLPC_TUInt32 wsize;
    NyLPC_TUInt16 s_padding;
    NyLPC_TUInt16 free_size;
    if(i_size%4!=0){
        NyLPC_OnErrorGoto(Error);
    }
    if(((NyLPC_TUInt32)i_dest)%4!=0){
        NyLPC_OnErrorGoto(Error);
    }
    size=i_size;
    src=(const char*)i_src;
    dest_addr=(const char*)i_dest;
    for(;size>0;){
        //開始位置の端数を調べる
        s_padding=((NyLPC_TUInt32)dest_addr)%256;
        //書き込みアドレス取得
        fblock_addr=dest_addr-s_padding;
        //書込み可能サイズを計算
        free_size=256-s_padding;
        //書込みサイズを決定
        wsize=(free_size>size)?size:free_size;
        //Flashから一時RAMへ前方パディングを読む
        if(s_padding>0){
            memcpy(_work,fblock_addr,s_padding);
        }
        //書き込むデータを一時RAMへ書き込む
        memcpy(_work+s_padding,src,wsize);
        //後半
        if(256-(wsize+s_padding)>0){
            memcpy(_work+s_padding+wsize,fblock_addr+(wsize+s_padding),256-(wsize+s_padding));
        }

        //Flashへ書込み
        //開始セクタ
        if(!LPC17xx_IAP_addr2Sector(fblock_addr,&snum)){
            NyLPC_OnErrorGoto(Error);
        }
        //IAPのprepareコマンド

        if(LPC17xx_IAP_CMD_SUCCESS!=LPC17xx_IAP_prepare(snum,snum)){
            NyLPC_OnErrorGoto(Error);
        }
        //IAPのwriteコマンド
        if(LPC17xx_IAP_CMD_SUCCESS!=LPC17xx_IAP_copyRam2Flash(fblock_addr,_work,256)){
            NyLPC_OnErrorGoto(Error);
        }
        dest_addr+=wsize;
        src+=wsize;
        size-=wsize;
    }
    return NyLPC_TBool_TRUE;
Error:
    return NyLPC_TBool_FALSE;
}

/**
 * セクタ+オフセット形式で、データを書き込みます。
 */
NyLPC_TBool NyLPC_cOnchipFlashWriter_writeSector(NyLPC_TUInt16 i_sector,NyLPC_TUInt32 i_offset,const void* i_src,NyLPC_TUInt32 i_size)
{
    void* addr;
    if(!LPC17xx_IAP_sector2Addr(i_sector,&addr)){
        return NyLPC_TBool_FALSE;
    }
    addr=(void*)((NyLPC_TUInt32)addr+i_offset);
    return NyLPC_cOnchipFlashWriter_write(addr,i_src,i_size);

}
/**
 * FlashRomのセクタ番号Nにイレースを実行します。
 */
NyLPC_TBool NyLPC_cOnchipFlashWriter_elase(NyLPC_TUInt16 i_sector_s,NyLPC_TUInt16 i_sector_e)
{
    if(LPC17xx_IAP_CMD_SUCCESS!=LPC17xx_IAP_prepare(i_sector_s,i_sector_e)){
        return NyLPC_TBool_FALSE;
    }
    if(LPC17xx_IAP_CMD_SUCCESS!=LPC17xx_IAP_erase(i_sector_s,i_sector_e)){
        return NyLPC_TBool_FALSE;
    }
    return NyLPC_TBool_TRUE;
}



#ifdef TEST
#include "stdio.h"
unsigned long buf[128]={0x1,0x2,0x03,0x04,0x05};
#define MIMIC_CONFIG_ADDR ((long*)(0x00018000+1280))
void setup(void)
{
    NyLPC_TcFlashWriter_t writer;
    NyLPC_cFlashWriter_initialize(&writer);
    unsigned long p;
    NyLPC_cFlashWriter_elase(&writer,29);
//  NyLPC_cFlashWriter_write(&writer,MIMIC_CONFIG_ADDR-8,buf,5*4);
    NyLPC_cFlashWriter_finalize(&writer);
    return;
}
void loop(void)
{
    //Implementation
    //ここにメインタスクを書きます。
    for(;;){}
}
#endif
#endif
