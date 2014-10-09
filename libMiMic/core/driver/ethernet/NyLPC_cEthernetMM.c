#include "NyLPC_cEthernetMM.h"

/**
 * メモリブロックの配置
 */
static struct NyLPC_TcEthernetMM_TxMemoryBlock* _mem_addr;



/**
 * デバック用。使用中のTxブロックの数を返す。
 */
int NyLPC_cEthernetMM_dbg_getNumofUsedTx(void)
{
	int x;
	NyLPC_TUInt8 r1,r2,r3,r4,r5;
	r1=r2=r3=r4=r5=0;
	for(x=0;x<NyLPC_TcEthernetMM_NUM_OF_MAX_BUF;x++){
		if(_mem_addr->buf_max[x].h.is_lock || _mem_addr->buf_max[x].h.ref>0){
			r1++;
			continue;
		}
	}
	for(x=0;x<NyLPC_TcEthernetMM_NUM_OF_512_BUF;x++){
		if(_mem_addr->buf_512[x].h.is_lock || _mem_addr->buf_512[x].h.ref>0){
			r2++;
			continue;
		}
	}
	for(x=0;x<NyLPC_TcEthernetMM_NUM_OF_256_BUF;x++){
		if(_mem_addr->buf_256[x].h.is_lock || _mem_addr->buf_256[x].h.ref>0){
			r3++;
			continue;
		}
	}
	for(x=0;x<NyLPC_TcEthernetMM_NUM_OF_128_BUF;x++){
		if(_mem_addr->buf_128[x].h.is_lock || _mem_addr->buf_128[x].h.ref>0){
			r4++;
			continue;
		}
	}
	for(x=0;x<NyLPC_TcEthernetMM_NUM_OF_64_BUF;x++){
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
	for(x=0;x<NyLPC_TcEthernetMM_NUM_OF_MAX_BUF;x++){
		_mem_addr->buf_max[x].h.is_lock=NyLPC_TUInt8_FALSE;
		_mem_addr->buf_max[x].h.ref=0;
	}
	for(x=0;x<NyLPC_TcEthernetMM_NUM_OF_512_BUF;x++){
		_mem_addr->buf_512[x].h.is_lock=NyLPC_TUInt8_FALSE;
		_mem_addr->buf_512[x].h.ref=0;
	}
	for(x=0;x<NyLPC_TcEthernetMM_NUM_OF_256_BUF;x++){
		_mem_addr->buf_256[x].h.is_lock=NyLPC_TUInt8_FALSE;
		_mem_addr->buf_256[x].h.ref=0;
	}
	for(x=0;x<NyLPC_TcEthernetMM_NUM_OF_128_BUF;x++){
		_mem_addr->buf_128[x].h.is_lock=NyLPC_TUInt8_FALSE;
		_mem_addr->buf_128[x].h.ref=0;
	}
	for(x=0;x<NyLPC_TcEthernetMM_NUM_OF_64_BUF;x++){
		_mem_addr->buf_64[x].h.is_lock=NyLPC_TUInt8_FALSE;
		_mem_addr->buf_64[x].h.ref=0;
	}
}

/**
 * 空のTxバッファのポインタを返します。
 */
void* NyLPC_cEthernetMM_alloc(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size)
{
	int i;
	//ヒントから、割り当てるメモリブロックを決定

	//特殊ブロック
	if(i_hint==NyLPC_TcEthernetMM_HINT_CTRL_PACKET){
		for(i=0;i<NyLPC_TcEthernetMM_NUM_OF_64_BUF;i++){
			//未参照かつ送信中でないもの。
			if(_mem_addr->buf_64[i].h.ref>0 || _mem_addr->buf_64[i].h.is_lock){
				continue;
			}
			_mem_addr->buf_64[i].h.ref++;
			*o_size=64;
			return _mem_addr->buf_64[i].b;
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
	for(i=0;i<NyLPC_TcEthernetMM_NUM_OF_MAX_BUF;i++){
		//未参照かつ送信中でないもの。
		if(_mem_addr->buf_max[i].h.ref>0 || _mem_addr->buf_max[i].h.is_lock){
			continue;
		}
		_mem_addr->buf_max[i].h.ref++;
		*o_size=NyLPC_TcEthernetMM_MAX_TX_ETHERNET_FRAME_SIZE;
		return _mem_addr->buf_max[i].b;
	}
ALLOC_512:
	for(i=0;i<NyLPC_TcEthernetMM_NUM_OF_512_BUF;i++){
		//未参照かつ送信中でないもの。
		if(_mem_addr->buf_512[i].h.ref>0 || _mem_addr->buf_512[i].h.is_lock){
			continue;
		}
		*o_size=512;
		_mem_addr->buf_512[i].h.ref++;
		return _mem_addr->buf_512[i].b;
	}
ALLOC_256:
	for(i=0;i<NyLPC_TcEthernetMM_NUM_OF_256_BUF;i++){
		//未参照かつ送信中でないもの。
		if(_mem_addr->buf_256[i].h.ref>0 || (_mem_addr->buf_256[i].h.is_lock)){
			continue;
		}
		*o_size=256;
		_mem_addr->buf_256[i].h.ref++;
		return _mem_addr->buf_256[i].b;
	}
ALLOC_128:
	for(i=0;i<NyLPC_TcEthernetMM_NUM_OF_128_BUF;i++){
		//未参照かつ送信中でないもの。
		if(_mem_addr->buf_128[i].h.ref>0 || (_mem_addr->buf_128[i].h.is_lock)){
			continue;
		}
		*o_size=128;
		_mem_addr->buf_128[i].h.ref++;
		return _mem_addr->buf_128[i].b;
	}
	return NULL;
}


void NyLPC_cEthernetMM_release(void* i_buf)
{
	struct NyLPC_TTxBufferHeader* h=NyLPC_TTxBufferHeader_getBufferHeaderAddr(i_buf);
	//参照カウンタを1減算
	NyLPC_Assert(h->ref>0);
	h->ref--;
	return;
}

