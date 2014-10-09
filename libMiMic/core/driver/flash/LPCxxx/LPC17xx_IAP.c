/*
 * LPC17xx_IAP.c
 *
 *  Created on: 2011/10/17
 *      Author: nyatla
 */
#include "NyLPC_config.h"
#if NyLPC_MCU==NyLPC_MCU_LPC4088 || NyLPC_MCU==NyLPC_MCU_LPC17xx

#include "LPC17xx_IAP.h"
/**
 * IPAに通知するCPUクロック。
 * ここで指定したクロック以上で動作させないでください。
 */
#define IPA_CPU_FREQ_IN_HZ ( ( unsigned long ) 100000000 )
#define LPC17xx_FLASH_SECTOR_ADDR_16 ((void*)0x00010000)

//Define data structure or pointers to pass IAP command table and result table to the IAP
#define IAP_LOCATION 0x1FFF1FF1;
/**
 * @param command
 *
 * @param result
 */
typedef void (*_LPCXpresso_IAP_FUNC)(unsigned long command[],unsigned long result[]);
/**
 *
 */
_LPCXpresso_IAP_FUNC LPCXpresso_iap_entry=(_LPCXpresso_IAP_FUNC) IAP_LOCATION;


/**
 * アドレスをフラッシュメモリのセクタ番号へ変換する。
 */
int LPC17xx_IAP_addr2Sector(const void* addr,unsigned long* o_sector)
{
    unsigned long t;
    if(addr>=LPC17xx_FLASH_SECTOR_ADDR_16){
        t=((((unsigned long)addr)-((unsigned long)LPC17xx_FLASH_SECTOR_ADDR_16))/0x8000)+16;
        if(t>29){
            return LPC17xx_IAP_FALSE;//Error
        }
        *o_sector=t;
    }else{
        *o_sector=((unsigned long)addr)/0x1000;
    }
    return LPC17xx_IAP_TRUE;//Error
}
/**
 * セクタ番号をフラッシュメモリのアドレスへ変換する。
 */
int LPC17xx_IAP_sector2Addr(unsigned int i_sector,void** o_addr)
{
    if(i_sector<16){
        *o_addr=(void*)(0x1000*i_sector);
    }else if(i_sector>29){
        return LPC17xx_IAP_FALSE;
    }else{
        *o_addr=(void*)(((unsigned int)LPC17xx_FLASH_SECTOR_ADDR_16)+(i_sector-16)*0x8000);
    }
    return LPC17xx_IAP_TRUE;
}


unsigned long LPC17xx_IAP_getSectorSize(unsigned int i_sector)
{
    if(i_sector<16){
        return 0x1000;
    }else if(i_sector>29){
        return 0;
    }else{
        return 0x8000;
    }
}
/**
 * IAPのprepareコマンドを実行
 */
unsigned long LPC17xx_IAP_prepare(unsigned long i_start,unsigned long i_end)
{
    unsigned long b[8];
    unsigned long c[5];
    unsigned long r[5];
    c[0]=50;
    c[1]=i_start;
    c[2]=i_end;
    memcpy(b,(void*)0x10000000,32);
    LPCXpresso_iap_entry(c,r);
    memcpy((void*)0x10000000,b,32);
    return r[0];
}
/**
 * IAPのcopy ram to flashコマンドを実行。
 */
unsigned long LPC17xx_IAP_copyRam2Flash(const void* i_flash_addr,const void* i_src_addr,unsigned long i_size)
{
    unsigned long b[8];
    unsigned long c[5];
    unsigned long r[5];
    c[0]=51;
    c[1]=(unsigned long)i_flash_addr;
    c[2]=(unsigned long)i_src_addr;
    c[3]=i_size;
    c[4]=IPA_CPU_FREQ_IN_HZ/1000;
    memcpy(b,(void*)0x10000000,32);
    LPCXpresso_iap_entry(c,r);
    memcpy((void*)0x10000000,b,32);
    return r[0];
}
/**
 * IAPのeraseコマンドを実行
 */
unsigned long LPC17xx_IAP_erase(unsigned long i_start,unsigned long i_end)
{
    unsigned long b[8];
    unsigned long c[5];
    unsigned long r[5];
    c[0]=52;
    c[1]=i_start;
    c[2]=i_end;
    c[3]=IPA_CPU_FREQ_IN_HZ/1000;
    memcpy(b,(void*)0x10000000,32);
    LPCXpresso_iap_entry(c,r);
    memcpy((void*)0x10000000,b,32);
    return r[0];
}
#endif
