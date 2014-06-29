/*
 * NyLPC_cBase64.c
 *
 *  Created on: 2013/09/04
 *      Author: nyatla
 */

#ifndef NYLPC_CBASE64_C_
#define NYLPC_CBASE64_C_

#include "NyLPC_cBase64.h"

/**
 * @param i_src
 * @param length
 * @param i_dest
 * Base64文字列の出力領域. length/3*4+1の長さが必要。
 */
void NyLPC_cBase64_encode(const NyLPC_TChar* i_src,NyLPC_TUInt16 length,char* i_dest)
{
	const static char* B64CODE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	NyLPC_TUInt16 s,d;
	d=0;
    for(s=0; s<(length-2); s+=3){
    	i_dest[d++]=B64CODE[((0xfc&i_src[s+0])>>2)];
		i_dest[d++]=B64CODE[((0x03&i_src[s+0])<<4)|((0xf0&i_src[s+1])>>4)];
		i_dest[d++]=B64CODE[((0x0f&i_src[s+1])<<2)|((0xc0&i_src[s+2])>>6)];
		i_dest[d++]=B64CODE[((0x3f&i_src[s+2])>>0)];
    }
	s=length-length%3;
    switch(length%3){
	case 1:
    	i_dest[d++]=B64CODE[((0xfc&i_src[s+0])>>2)];
		i_dest[d++]=B64CODE[((0x03&i_src[s+0])<<4)];
		break;
	case 2:
    	i_dest[d++]=B64CODE[((0xfc&i_src[s+0])>>2)];
		i_dest[d++]=B64CODE[((0x03&i_src[s+0])<<4)|((0xf0&i_src[s+1])>>4)];
		i_dest[d++]=B64CODE[((0x0f&i_src[s+1])<<2)];
		break;
    }
    //文字数
    while(d%4!=0){
    	i_dest[d++]='=';
    }
	i_dest[d]='\0';
}



#endif /* NYLPC_CBASE64_C_ */

