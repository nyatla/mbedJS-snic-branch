#ifndef LPC17xx_IAP_h
#define LPC17xx_IAP_h

#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LPC17xx_IAP_TRUE (0==0)
#define LPC17xx_IAP_FALSE (0!=0)
//Command is executed successfully.
#define LPC17xx_IAP_CMD_SUCCESS 0

int LPC17xx_IAP_addr2Sector(const void* addr,unsigned long* o_sector);
int LPC17xx_IAP_sector2Addr(unsigned int i_sector,void** o_addr);

unsigned long LPC17xx_IAP_getSectorSize(unsigned int i_sector);
unsigned long LPC17xx_IAP_prepare(unsigned long i_start,unsigned long i_end);
unsigned long LPC17xx_IAP_copyRam2Flash(const void* i_flash_addr,const void* i_src_addr,unsigned long i_size);
unsigned long LPC17xx_IAP_erase(unsigned long i_start,unsigned long i_end);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
