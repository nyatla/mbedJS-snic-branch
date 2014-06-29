/*
 * EthDev_LPC1769.c
 *
 *  Created on: 2011/12/07
 *      Author: nyatla
 */
#include "NyLPC_config.h"
#if NyLPC_MCU==NyLPC_MCU_LPC17xx
#include "LPC17xx.h"
#include "EthDev_LPC17xx.h"
#include "NyLPC_os.h"
#include "LPC17xx.h"
/* If no buffers are available, then wait this long before looking again.... */
#define emacBUFFER_WAIT_DELAY_MS		 3
#define emacBUFFER_WAIT_EMPTY_DELAY_MS	10
#define emacSHORT_DELAY_MS				10
//--------------------------------------------------
// common function.
//--------------------------------------------------



/**
 * 送信デスクリプタを準備します。
 */
void EthDev_LPC17xx_prevTxDescriptor(void)
{
	long x;
	//デスクリプタの設定
	for( x = 0; x < NUM_TX_FRAG; x++ )
	{
		TX_DESC_PACKET( x ) = ( unsigned long ) NULL;
		TX_DESC_CTRL( x ) = 0;
		TX_STAT_INFO( x ) = 0;
	}
	/* Set LPC_EMAC Transmit Descriptor Registers. */
	LPC_EMAC->TxDescriptor = TX_DESC_BASE;
	LPC_EMAC->TxStatus = TX_STAT_BASE;
	LPC_EMAC->TxDescriptorNumber = NUM_TX_FRAG - 1;
}


/**
 * 送信中のイーサフレームを処理する機会を与えて、送信キューが空くまで待ちます。
 * LPC1769の場合は、非同期に更新したディスクリプタの内容から、送信メモリのフラグを更新します。
 * @return
 * 次に書き込むことが出来る送信キュー。
 */
static NyLPC_TUInt32 waitForTxEthFrameEmpty(void)
{
	NyLPC_TUInt32	IndexNext;
	struct NyLPC_TTxBufferHeader *b;
	void* p;
	NyLPC_TUInt32 i;

	//送信キューの決定
	IndexNext = (LPC_EMAC->TxProduceIndex + 1)%NUM_TX_FRAG;

	//送信キューフルが解除されるまで待ち
	while(IndexNext == LPC_EMAC->TxConsumeIndex)
	{
		//
		NyLPC_cThread_sleep(emacBUFFER_WAIT_EMPTY_DELAY_MS);
	}

	//(TxProduceIndex+1)→TxConsumeIndexにあるデータのsentフラグを消去
	for(i=IndexNext;i!=LPC_EMAC->TxConsumeIndex;i=(i+1)%NUM_TX_FRAG)
	{
		p=(void*)TX_DESC_PACKET(i);
		if(p!=NULL){
			b=((struct NyLPC_TTxBufferHeader*)p)-1;
			b->is_lock=NyLPC_TUInt8_FALSE;
			TX_DESC_PACKET(i)=0;
		}
	}
	p=(void*)TX_DESC_PACKET(i);
	if(p!=NULL){
		b=((struct NyLPC_TTxBufferHeader*)p)-1;
		b->is_lock=NyLPC_TUInt8_FALSE;
		TX_DESC_PACKET(i)=0;
	}
	return IndexNext;
}

void EthDev_LPC17xx_processTx(void)
{
	waitForTxEthFrameEmpty();
}

/**
 * Ethernetパケットを送信します。
 * allocTxBufで得たバッファか、NyLPC_TTxBufferHeaderのペイロード部分を指定すること。
 * <p>関数仕様</p>
 * この関数は、i_bufが
 * </div>
 */
void EthDev_LPC17xx_sendTxEthFrame(struct NyLPC_TTxBufferHeader* i_buf,unsigned short i_size)
{
	NyLPC_TUInt32	IndexNext,Index;

	//サイズ0なら送信の必要なし
	if(i_size == 0)
	{
		return;
	}
	//送信デスクリプタの反映
	IndexNext =waitForTxEthFrameEmpty();

	//送信対象のメモリブロックを送信中に設定。
//	b=(i_buf+1);
	//送信中のメモリブロックなら無視
	if(i_buf->is_lock){
		return;
	}
	//送信中にセット
	i_buf->is_lock=NyLPC_TUInt8_TRUE;

	//送信データのセット
	Index = LPC_EMAC->TxProduceIndex;
	if (i_size > ETH_FRAG_SIZE){
		i_size = ETH_FRAG_SIZE;
	}

	//送信処理
	TX_DESC_PACKET( Index ) = ( unsigned long )(i_buf+1);
	//See UM10360.pdf Table 181. Transmit descriptor control word
	TX_DESC_CTRL( Index ) = ((i_size-1) | TCTRL_LAST | TCTRL_INT );
	LPC_EMAC->TxProduceIndex = IndexNext;
	return;
}

/***********************************************************************
 * RXバッファ
 ***********************************************************************/

void EthDev_LPC17xx_prevRxDescriptor(void)
{
	int x;
	//デスクリプタの設定
	for( x = 0; x < NUM_RX_FRAG; x++ )
	{
		/* Allocate the next Ethernet buffer to this descriptor. */
		RX_DESC_PACKET(x) = ETH_BUF(x);
		RX_DESC_CTRL(x) = RCTRL_INT | ( ETH_FRAG_SIZE - 1 );
		RX_STAT_INFO(x) = 0;
		RX_STAT_HASHCRC(x) = 0;
	}

	/* Set LPC_EMAC Receive Descriptor Registers. */
	LPC_EMAC->RxDescriptor = RX_DESC_BASE;
	LPC_EMAC->RxStatus = RX_STAT_BASE;
	LPC_EMAC->RxDescriptorNumber = NUM_RX_FRAG - 1;

}


/**
 * 受信キューの先頭にあるRXフレームのポインタを返します。
 * 関数は、受信キューのポインタを操作しません。続けて読み出したとしても、同じポインターを返します。
 * 制限として、返却したポインタの内容は、一時的に書き換え可としてください。（この制限は将来削除します。）
 * @return
 * 成功した場合、受信データを格納したバッファポインターです。
 * 次回nextRxEthFrameを呼び出すまで有効です。
 */
void* EthDev_LPC17xx_getRxEthFrame(unsigned short* o_len_of_data)
{

	if( LPC_EMAC->RxProduceIndex != LPC_EMAC->RxConsumeIndex )
	{
		//受信データを返却する。
		*o_len_of_data = (unsigned short)(( RX_STAT_INFO( LPC_EMAC->RxConsumeIndex ) & RINFO_SIZE ) - 3);
		return ( unsigned char * ) RX_DESC_PACKET( LPC_EMAC->RxConsumeIndex );
	}
	return NULL;
}


/**
 * 受信キューを進行します。
 */
void EthDev_LPC17xx_nextRxEthFrame(void)
{
	long lIndex;
	if( LPC_EMAC->RxProduceIndex != LPC_EMAC->RxConsumeIndex )
	{
		//キューすすめる。
		lIndex = LPC_EMAC->RxConsumeIndex;
		lIndex++;
		if( lIndex >= NUM_RX_FRAG )
		{
			lIndex = 0;
		}
		LPC_EMAC->RxConsumeIndex = lIndex;
	}
}


/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/
#define ETHDEV_PHY_DEF_ADR		0x0100  /* Default PHY device address        */

NyLPC_TBool EthDev_LPC17xx_prvWritePHY( long lPhyReg, long lValue )
{
    const long lMaxTime = 10;
    long x;

    LPC_EMAC->MCMD = 0;
    LPC_EMAC->MADR = ETHDEV_PHY_DEF_ADR | lPhyReg;
    LPC_EMAC->MWTD = lValue;

    for( x = 0; x < lMaxTime; x++ )
    {
        if( ( LPC_EMAC->MIND & MIND_BUSY ) == 0 )
        {
            /* Operation has finished. */
            break;
        }

        NyLPC_cThread_sleep( emacSHORT_DELAY_MS );
    }

    if( x < lMaxTime )
    {
        return NyLPC_TBool_TRUE;
    }
    else
    {
        return NyLPC_TBool_FALSE;
    }
}
/*-----------------------------------------------------------*/

unsigned short EthDev_LPC17xx_prvReadPHY( unsigned int ucPhyReg, NyLPC_TBool* plStatus )
{
    long x;
    const long lMaxTime = 10;

    LPC_EMAC->MCMD = 1;
    LPC_EMAC->MADR = ETHDEV_PHY_DEF_ADR | ucPhyReg;
    LPC_EMAC->MCMD = MCMD_READ;

    for( x = 0; x < lMaxTime; x++ )
    {
        /* Operation has finished. */
        if( ( LPC_EMAC->MIND & MIND_BUSY ) == 0 )
        {
            break;
        }
        NyLPC_cThread_sleep( emacSHORT_DELAY_MS );
    }

    LPC_EMAC->MCMD = 0;

    if( x >= lMaxTime )
    {
        *plStatus = NyLPC_TBool_FALSE;
    }

    return( LPC_EMAC->MRDD );
}








#endif

