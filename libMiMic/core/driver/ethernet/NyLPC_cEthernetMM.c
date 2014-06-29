#include "NyLPC_cEthernetMM.h"
/**
 * メモリブロックの数
 */
#define NUM_OF_MAX_BUF  3
#define NUM_OF_512_BUF  3
#define NUM_OF_256_BUF  4
#define NUM_OF_128_BUF 16
#define NUM_OF_64_BUF   4

/**
 * FULLサイズのEthernetFrame送信メモリのサイズ。
 * ここで最大送信サイズを制限する。
 * 通常は1460+20+20+14=1514バイト
 */
#define MAX_TX_ETHERNET_FRAME_SIZE	1514

/**
 * TXメモリブロックの配置
 * 9518バイト
 */
struct TTxMemoryBlock
{
	struct{
		struct NyLPC_TTxBufferHeader h;
		NyLPC_TUInt8 b[MAX_TX_ETHERNET_FRAME_SIZE];
	}buf_max[NUM_OF_MAX_BUF];//(4+MAX_TX_ETHERNET_FRAME_SIZE(1514))*3=? default=4554
	struct{
		struct NyLPC_TTxBufferHeader h;
		NyLPC_TUInt8 b[512];
	}buf_512[NUM_OF_512_BUF];//(4+512)*3=1548
	struct{
		struct NyLPC_TTxBufferHeader h;
		NyLPC_TUInt8 b[256];
	}buf_256[NUM_OF_256_BUF];//(4+256)*4=1560
	struct{
		struct NyLPC_TTxBufferHeader h;
		NyLPC_TUInt8 b[128];
	}buf_128[NUM_OF_128_BUF];//(4+128)*16=1584
	struct{
		struct NyLPC_TTxBufferHeader h;
		NyLPC_TUInt8 b[64];
	}buf_64[NUM_OF_64_BUF];//(4+64)*4=272
};
/**
 * メモリブロックの配置
 */
static struct TTxMemoryBlock* _mem_addr;



/**
 * デバック用。使用中のTxブロックの数を返す。
 */
int NyLPC_cEthernetMM_dbg_getNumofUsedTx(void)
{
	int x;
	NyLPC_TUInt8 r1,r2,r3,r4,r5;
	r1=r2=r3=r4=r5=0;
	for(x=0;x<NUM_OF_MAX_BUF;x++){
		if(_mem_addr->buf_max[x].h.is_lock || _mem_addr->buf_max[x].h.ref>0){
			r1++;
			continue;
		}
	}
	for(x=0;x<NUM_OF_512_BUF;x++){
		if(_mem_addr->buf_512[x].h.is_lock || _mem_addr->buf_512[x].h.ref>0){
			r2++;
			continue;
		}
	}
	for(x=0;x<NUM_OF_256_BUF;x++){
		if(_mem_addr->buf_256[x].h.is_lock || _mem_addr->buf_256[x].h.ref>0){
			r3++;
			continue;
		}
	}
	for(x=0;x<NUM_OF_128_BUF;x++){
		if(_mem_addr->buf_128[x].h.is_lock || _mem_addr->buf_128[x].h.ref>0){
			r4++;
			continue;
		}
	}
	for(x=0;x<NUM_OF_64_BUF;x++){
		if(_mem_addr->buf_64[x].h.is_lock || _mem_addr->buf_64[x].h.ref>0){
			r5++;
			continue;
		}
	}
	return r1+r2+r3+r4+r5;
}


void NyLPC_cEthernetMM_initialize(void* i_memblock_addr)
{
	int x;
	_mem_addr=i_memblock_addr;
	//TXバッファを初期化
	for(x=0;x<NUM_OF_MAX_BUF;x++){
		_mem_addr->buf_max[x].h.is_lock=NyLPC_TUInt8_FALSE;
		_mem_addr->buf_max[x].h.ref=0;
	}
	for(x=0;x<NUM_OF_512_BUF;x++){
		_mem_addr->buf_512[x].h.is_lock=NyLPC_TUInt8_FALSE;
		_mem_addr->buf_512[x].h.ref=0;
	}
	for(x=0;x<NUM_OF_256_BUF;x++){
		_mem_addr->buf_256[x].h.is_lock=NyLPC_TUInt8_FALSE;
		_mem_addr->buf_256[x].h.ref=0;
	}
	for(x=0;x<NUM_OF_128_BUF;x++){
		_mem_addr->buf_128[x].h.is_lock=NyLPC_TUInt8_FALSE;
		_mem_addr->buf_128[x].h.ref=0;
	}
	for(x=0;x<NUM_OF_64_BUF;x++){
		_mem_addr->buf_64[x].h.is_lock=NyLPC_TUInt8_FALSE;
		_mem_addr->buf_64[x].h.ref=0;
	}
}

/**
 * 空のTxバッファのポインタを返します。
 */
struct NyLPC_TTxBufferHeader* NyLPC_cEthernetMM_alloc(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size)
{
	int i;
	//ヒントから、割り当てるメモリブロックを決定

	//特殊ブロック
	if(i_hint==NyLPC_TcEthernetMM_HINT_CTRL_PACKET){
		for(i=0;i<NUM_OF_64_BUF;i++){
			//未参照かつ送信中でないもの。
			if(_mem_addr->buf_64[i].h.ref>0 || _mem_addr->buf_64[i].h.is_lock){
				continue;
			}
			_mem_addr->buf_64[i].h.ref++;
			*o_size=64;
			return &(_mem_addr->buf_64[i].h);
		}
		return NULL;
	}

	//汎用ブロック
	if(i_hint<=128){
		goto ALLOC_128;
	}else if(i_hint<=256){
		goto ALLOC_256;
	}else if(i_hint<=512){
		goto ALLOC_512;
	}else{
		goto ALLOC_MAX;
	}

ALLOC_MAX:
	for(i=0;i<NUM_OF_MAX_BUF;i++){
		//未参照かつ送信中でないもの。
		if(_mem_addr->buf_max[i].h.ref>0 || _mem_addr->buf_max[i].h.is_lock){
			continue;
		}
		_mem_addr->buf_max[i].h.ref++;
		*o_size=MAX_TX_ETHERNET_FRAME_SIZE;
		return &(_mem_addr->buf_max[i].h);
	}
ALLOC_512:
	for(i=0;i<NUM_OF_512_BUF;i++){
		//未参照かつ送信中でないもの。
		if(_mem_addr->buf_512[i].h.ref>0 || _mem_addr->buf_512[i].h.is_lock){
			continue;
		}
		*o_size=512;
		_mem_addr->buf_512[i].h.ref++;
		return &(_mem_addr->buf_512[i].h);
	}
ALLOC_256:
	for(i=0;i<NUM_OF_256_BUF;i++){
		//未参照かつ送信中でないもの。
		if(_mem_addr->buf_256[i].h.ref>0 || (_mem_addr->buf_256[i].h.is_lock)){
			continue;
		}
		*o_size=256;
		_mem_addr->buf_256[i].h.ref++;
		return &(_mem_addr->buf_256[i].h);
	}
ALLOC_128:
	for(i=0;i<NUM_OF_128_BUF;i++){
		//未参照かつ送信中でないもの。
		if(_mem_addr->buf_128[i].h.ref>0 || (_mem_addr->buf_128[i].h.is_lock)){
			continue;
		}
		*o_size=128;
		_mem_addr->buf_128[i].h.ref++;
		return &(_mem_addr->buf_128[i].h);
	}
	return NULL;
}


void NyLPC_cEthernetMM_release(struct NyLPC_TTxBufferHeader* i_buf)
{
	//参照カウンタを1減算
	NyLPC_Assert(i_buf->ref>0);
	i_buf->ref--;
	return;
}

