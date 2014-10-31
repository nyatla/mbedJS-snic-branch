#include "NyLPC_config.h"
#if NyLPC_MCU==NyLPC_MCU_LPC4088
#include "NyLPC_os.h"
#include "copy_of_ethernet_api.h"
#include "NyLPC_IEthernetDevice.h"
#include "NyLPC_cEthernetMM.h"



#define emacSHORT_DELAY_MS                 10
#ifndef configEMAC_INTERRUPT_PRIORITY
    #define configEMAC_INTERRUPT_PRIORITY       5
#endif
////////////////////////////////////////////////////////////////////////////////
// Ethernet Memory
////////////////////////////////////////////////////////////////////////////////

#define AHB_SRAM_BANK1_BASE  0x20004000UL
#define RX_DESC_BASE        (AHB_SRAM_BANK1_BASE         )
#define RX_STAT_BASE        (RX_DESC_BASE + NUM_RX_FRAG*(2*4))     /* 2 * uint32_t, see RX_DESC_TypeDef */
#define TX_DESC_BASE        (RX_STAT_BASE + NUM_RX_FRAG*(2*4))     /* 2 * uint32_t, see RX_STAT_TypeDef */
#define TX_STAT_BASE        (TX_DESC_BASE + NUM_TX_FRAG*(2*4))     /* 2 * uint32_t, see TX_DESC_TypeDef */
#define ETH_BUF_BASE		(TX_STAT_BASE + NUM_TX_FRAG*(1*4))     /* 1 * uint32_t, see TX_STAT_TypeDef */

/**
 * 消費メモリ量は、
 * descriptor = NUM_RX_FRAG*16+NUM_TX_FRAG*12.
 * EthnetBuf=ETH_FRAG_SIZE*NUM_RX_FRAG
 */

/* RX and TX descriptor and status definitions. */
#define RX_DESC_PACKET(i)   (*(unsigned int *)(RX_DESC_BASE   + 8*i))
#define RX_DESC_CTRL(i)     (*(unsigned int *)(RX_DESC_BASE+4 + 8*i))
#define RX_STAT_INFO(i)     (*(unsigned int *)(RX_STAT_BASE   + 8*i))
#define RX_STAT_HASHCRC(i)  (*(unsigned int *)(RX_STAT_BASE+4 + 8*i))
#define TX_DESC_PACKET(i)   (*(unsigned int *)(TX_DESC_BASE   + 8*i))
#define TX_DESC_CTRL(i)     (*(unsigned int *)(TX_DESC_BASE+4 + 8*i))
#define TX_STAT_INFO(i)     (*(unsigned int *)(TX_STAT_BASE   + 4*i))
#define ETH_BUF(i)          ( ETH_BUF_BASE + ETH_FRAG_SIZE*i )
#define ETH_TX_BUF_BASE ((void*)(ETH_BUF_BASE+ETH_FRAG_SIZE*NUM_RX_FRAG))


#define emacWAIT_FOR_LINK_TO_ESTABLISH_MS 500

////////////////////////////////////////////////////////////////////////////////
// Ethernet interdface functions
////////////////////////////////////////////////////////////////////////////////
static NyLPC_TBool start(const struct NyLPC_TEthAddr* i_eth_addr,NyLPC_TiEthernetDevice_onEvent i_handler,void* i_param);
static void stop(void);
static void* getRxEthFrame(unsigned short* o_len_of_data);
static void nextRxEthFrame(void);
static void* allocTxBuf(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size);
static void releaseTxBuf(void* i_buf);
static void sendTxEthFrame(void* i_buf,unsigned short i_size);
static void processTx(void);

////////////////////////////////////////////////////////////////////////////////
// Private
////////////////////////////////////////////////////////////////////////////////
static void emacIsrHandler(unsigned long i_status);
static unsigned int clockselect(void);
static int ethernet_link(void);
static int phy_write(unsigned int PhyReg, unsigned short Data);
static int phy_read(unsigned int PhyReg);
static void prevTxDescriptor(void);
static void prevRxDescriptor(void);
static NyLPC_TUInt32 waitForTxEthFrameEmpty(void);

/*-----------------------------------------------------------*/


const static struct TiEthernetDevice _interface_LAN8720=
{
	"LAN8720",
	start,
	stop,
	getRxEthFrame,
	nextRxEthFrame,
	allocTxBuf,
	releaseTxBuf,
	sendTxEthFrame,
	processTx
};
const static struct TiEthernetDevice _interface_DP83848C=
{
	"DP83848C",
	start,
	stop,
	getRxEthFrame,
	nextRxEthFrame,
	allocTxBuf,
	releaseTxBuf,
	sendTxEthFrame,
	processTx
};

static void* _event_param;
static NyLPC_TiEthernetDevice_onEvent _event_handler;
static unsigned int phy_id;

/*
 * EthernetDeviceのファクトリー関数。インターフェイスを生成できればtrue
 *
 */
NyLPC_TBool EthDev_LPC4088_getInterface(
	const struct TiEthernetDevice** o_dev)
{
	int regv, tout;
	unsigned int clock = clockselect();
	
	LPC_SC->PCONP |= 0x40000000; /* Power Up the EMAC controller. */
	LPC_IOCON->P1_0 &= ~0x07; /* ENET I/O config */
	LPC_IOCON->P1_0 |= 0x01; /* ENET_TXD0 */
	LPC_IOCON->P1_1 &= ~0x07;
	LPC_IOCON->P1_1 |= 0x01; /* ENET_TXD1 */
	LPC_IOCON->P1_4 &= ~0x07;
	LPC_IOCON->P1_4 |= 0x01; /* ENET_TXEN */
	LPC_IOCON->P1_8 &= ~0x07;
	LPC_IOCON->P1_8 |= 0x01; /* ENET_CRS */
	LPC_IOCON->P1_9 &= ~0x07;
	LPC_IOCON->P1_9 |= 0x01; /* ENET_RXD0 */
	LPC_IOCON->P1_10 &= ~0x07;
	LPC_IOCON->P1_10 |= 0x01; /* ENET_RXD1 */
	LPC_IOCON->P1_14 &= ~0x07;
	LPC_IOCON->P1_14 |= 0x01; /* ENET_RX_ER */
	LPC_IOCON->P1_15 &= ~0x07;
	LPC_IOCON->P1_15 |= 0x01; /* ENET_REF_CLK */
	LPC_IOCON->P1_16 &= ~0x07; /* ENET/PHY I/O config */
	LPC_IOCON->P1_16 |= 0x01; /* ENET_MDC */
	LPC_IOCON->P1_17 &= ~0x07;
	LPC_IOCON->P1_17 |= 0x01; /* ENET_MDIO */
  
	/* Reset all EMAC internal modules. */
	LPC_EMAC->MAC1 = MAC1_RES_TX | MAC1_RES_MCS_TX | MAC1_RES_RX | MAC1_RES_MCS_RX | MAC1_SIM_RES | MAC1_SOFT_RES;
	LPC_EMAC->Command = CR_REG_RES | CR_TX_RES | CR_RX_RES | CR_PASS_RUNT_FRM;		
for (tout = 100; tout; tout--) { __NOP(); } /* A short delay */
	
	/* Initialize MAC control registers. */
	LPC_EMAC->MAC1 = MAC1_PASS_ALL; 
	LPC_EMAC->MAC2 = MAC2_CRC_EN | MAC2_PAD_EN;
	LPC_EMAC->MAXF = ETH_MAX_FLEN;
	LPC_EMAC->CLRT = CLRT_DEF;
	LPC_EMAC->IPGR = IPGR_DEF;
	
	/* Enable Reduced MII interface. */
	LPC_EMAC->MCFG = (clock << 0x2) & MCFG_CLK_SEL; /* Set clock */
	LPC_EMAC->MCFG |= MCFG_RES_MII; /* and reset */
	LPC_EMAC->Command = CR_RMII | CR_PASS_RUNT_FRM |CR_PASS_RX_FILT; /* Enable Reduced MII interface. */
	
for (tout = 100; tout; tout--) { __NOP(); } /* A short delay */

	LPC_EMAC->MCFG = (clock << 0x2) & MCFG_CLK_SEL;
	LPC_EMAC->MCMD = 0;
	LPC_EMAC->SUPP = SUPP_RES_RMII; /* Reset Reduced MII Logic. */
for (tout = 100; tout; tout--) { __NOP(); } /* A short delay */
	LPC_EMAC->SUPP = SUPP_SPEED;
	
	phy_write(PHY_REG_BMCR, PHY_BMCR_RESET); /* perform PHY reset */
	for(tout = 0x20000; ; tout--) { /* Wait for hardware reset to end. */
		regv = phy_read(PHY_REG_BMCR);
		if(regv < 0 || tout == 0) {
    		return NyLPC_TBool_FALSE; /* Error */
    	}
		if(!(regv & PHY_BMCR_RESET)) {
    		break; /* Reset complete. */
    	}
  	}

	phy_id = (phy_read(PHY_REG_IDR1) << 16);
	phy_id |= (phy_read(PHY_REG_IDR2) & 0XFFF0);

	switch(phy_id){
	case DP83848C_ID:
		*o_dev=&_interface_DP83848C;
		break;
	case LAN8720_ID:
		*o_dev=&_interface_LAN8720;
		break;
	default:
		return NyLPC_TBool_FALSE; /* Error */
  	}
	LPC_EMAC->TxProduceIndex = 0;
	LPC_EMAC->RxConsumeIndex = 0;  	
	return NyLPC_TBool_TRUE;
}



static NyLPC_TBool start(const struct NyLPC_TEthAddr* i_eth_addr,NyLPC_TiEthernetDevice_onEvent i_handler,void* i_param)
{
	int i;
	//ISRw割り込み設定
	NyLPC_cIsr_setEnetISR(emacIsrHandler);
	_event_handler=i_handler;
	_event_param=i_param;
	/* Set the Ethernet MAC Address registers */
	LPC_EMAC->SA0 = (((uint32_t)(i_eth_addr->addr[0])) << 8 ) | i_eth_addr->addr[1];
	LPC_EMAC->SA1 = (((uint32_t)(i_eth_addr->addr[2])) << 8 ) | i_eth_addr->addr[3];
	LPC_EMAC->SA2 = (((uint32_t)(i_eth_addr->addr[4])) << 8 ) | i_eth_addr->addr[5];

	//TXメモリマネージャの準備
	NyLPC_cEthernetMM_initialize(ETH_TX_BUF_BASE);
	/* Initialize Tx and Rx DMA Descriptors */
	prevRxDescriptor();
	prevTxDescriptor();
	//wait for link up 
	for(i=0;i<5;i++){
		if(ethernet_link()!=0){
			break;
		}
		NyLPC_cThread_sleep(emacWAIT_FOR_LINK_TO_ESTABLISH_MS);
	}

	//setup Link
	ethernet_set_link(-1, 0);

	LPC_EMAC->RxFilterCtrl = RFC_UCAST_EN | RFC_MCAST_EN | RFC_BCAST_EN | RFC_PERFECT_EN;
	/* Receive Broadcast, Perfect Match Packets */

	//Ethernetの割込み開始設定
	NyLPC_cIsr_enterCritical();
	{
		LPC_EMAC->IntEnable = INT_RX_DONE | INT_TX_DONE; /* Enable EMAC interrupts. */
		LPC_EMAC->IntClear = 0xFFFF; /* Reset all interrupts */
	  
		LPC_EMAC->Command |= (CR_RX_EN | CR_TX_EN); /* Enable receive and transmit mode of MAC Ethernet core */
		LPC_EMAC->MAC1 |= MAC1_REC_EN;

        NVIC_SetPriority( ENET_IRQn, configEMAC_INTERRUPT_PRIORITY );
        NVIC_EnableIRQ( ENET_IRQn );
	}
	NyLPC_cIsr_exitCritical();

	return NyLPC_TBool_TRUE;
}


static void stop(void)
{
	NyLPC_cIsr_enterCritical();
	{
	    LPC_EMAC->IntEnable &= ~(INT_RX_DONE | INT_TX_DONE);
	    LPC_EMAC->IntClear = 0xFFFF;
	    
        NVIC_DisableIRQ( ENET_IRQn );
	}
	NyLPC_cIsr_exitCritical();
	LPC_EMAC->Command &= ~( CR_RX_EN | CR_TX_EN );
	LPC_EMAC->MAC1 &= ~MAC1_REC_EN;
	//ISR割り込み解除
	NyLPC_cIsr_setEnetISR(NULL);
	//TXメモリマネージャの終了
	NyLPC_cEthernetMM_finalize();
}

static void* allocTxBuf(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size)
{
	return NyLPC_cEthernetMM_alloc(i_hint,o_size);
}
static void releaseTxBuf(void* i_buf)
{
	NyLPC_cEthernetMM_release(i_buf);
}


/**
*/
static void processTx(void)
{
	waitForTxEthFrameEmpty();
}



/**
 * Ethernetパケットを送信します。
 * allocTxBufで得たバッファを指定すること。
 * <p>関数仕様</p>
 * この関数は、i_bufが
 * </div>
 */
static void sendTxEthFrame(void* i_buf,unsigned short i_size)
{
	NyLPC_TUInt32	IndexNext,Index;
	struct NyLPC_TTxBufferHeader* bh=NyLPC_TTxBufferHeader_getBufferHeaderAddr(i_buf);

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
	if(bh->is_lock){
		return;
	}
	//送信中にセット
	bh->is_lock=NyLPC_TUInt8_TRUE;

	//送信データのセット
	Index = LPC_EMAC->TxProduceIndex;
	if (i_size > ETH_FRAG_SIZE){
		i_size = ETH_FRAG_SIZE;
	}
	//送信処理
	TX_DESC_PACKET( Index ) = ( unsigned long )i_buf;
	//See UM10360.pdf Table 181. Transmit descriptor control word
	TX_DESC_CTRL( Index ) = ((i_size-1) | TCTRL_LAST | TCTRL_INT );
	LPC_EMAC->TxProduceIndex = IndexNext;
	return;
}
/**
 * 送信デスクリプタを準備します。
 */
static void prevTxDescriptor(void)
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
	LPC_EMAC->TxDescriptor =TX_DESC_BASE;
	LPC_EMAC->TxStatus = TX_STAT_BASE;
	LPC_EMAC->TxDescriptorNumber = NUM_TX_FRAG - 1;
}
static void prevRxDescriptor(void)
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
static void* getRxEthFrame(unsigned short* o_len_of_data)
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
static void nextRxEthFrame(void)
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
/********************************************************************************
 * Private functions
 *******************************************************************************/



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
		NyLPC_cThread_sleep(emacSHORT_DELAY_MS);
	}

	//(TxProduceIndex+1)→TxConsumeIndexにあるデータのsentフラグを消去
	for(i=IndexNext;i!=LPC_EMAC->TxConsumeIndex;i=(i+1)%NUM_TX_FRAG)
	{
		p=(void*)TX_DESC_PACKET(i);
		if(p!=NULL){
			b=NyLPC_TTxBufferHeader_getBufferHeaderAddr(p);
			b->is_lock=NyLPC_TUInt8_FALSE;
			TX_DESC_PACKET(i)=0;
		}
	}
	p=(void*)TX_DESC_PACKET(i);
	if(p!=NULL){
		b=NyLPC_TTxBufferHeader_getBufferHeaderAddr(p);
		b->is_lock=NyLPC_TUInt8_FALSE;
		TX_DESC_PACKET(i)=0;
	}
	return IndexNext;
}

//--------------------------------------------------------------------------------
// ISR
//--------------------------------------------------------------------------------

static void ethernet_set_link(int speed, int duplex) {
    unsigned short phy_data;
    int tout;
    
    if((speed < 0) || (speed > 1)) {
        phy_data = PHY_AUTO_NEG;
    } else {
        phy_data = (((unsigned short) speed << 13) |
                    ((unsigned short) duplex << 8));
    }
    
    phy_write(PHY_REG_BMCR, phy_data);
    
    for (tout = 100; tout; tout--) { __NOP(); } /* A short delay */
    
    switch(phy_id) {
        case DP83848C_ID:
            phy_data = phy_read(PHY_REG_STS);
            
            if(phy_data & PHY_STS_DUPLEX) {
                LPC_EMAC->MAC2 |= MAC2_FULL_DUP;
                LPC_EMAC->Command |= CR_FULL_DUP;
                LPC_EMAC->IPGT = IPGT_FULL_DUP;
            } else {
            LPC_EMAC->MAC2 &= ~MAC2_FULL_DUP;
                LPC_EMAC->Command &= ~CR_FULL_DUP;
                LPC_EMAC->IPGT = IPGT_HALF_DUP;
            }
            
            if(phy_data & PHY_STS_SPEED) {
                LPC_EMAC->SUPP &= ~SUPP_SPEED;
            } else {
                LPC_EMAC->SUPP |= SUPP_SPEED;
            }
            break;
        
        case LAN8720_ID:
            phy_data = phy_read(PHY_REG_SCSR);
            
            if (phy_data & PHY_SCSR_DUPLEX) {
                LPC_EMAC->MAC2 |= MAC2_FULL_DUP;
                LPC_EMAC->Command |= CR_FULL_DUP;
                LPC_EMAC->IPGT = IPGT_FULL_DUP;
            } else {
                LPC_EMAC->Command &= ~CR_FULL_DUP;
                LPC_EMAC->IPGT = IPGT_HALF_DUP;
            }
            
            if(phy_data & PHY_SCSR_100MBIT) {
                LPC_EMAC->SUPP |= SUPP_SPEED;
            } else {
                LPC_EMAC->SUPP &= ~SUPP_SPEED;
            }
            
            break;
    }
}

static int phy_write(unsigned int PhyReg, unsigned short Data) {
    unsigned int timeOut;

    LPC_EMAC->MADR = DP83848C_DEF_ADR | PhyReg;
    LPC_EMAC->MWTD = Data;

    for(timeOut = 0; timeOut < MII_WR_TOUT; timeOut++) { /* Wait until operation completed */
        if((LPC_EMAC->MIND & MIND_BUSY) == 0) {
            return 0;
        }
    }

    return -1;
}


static int phy_read(unsigned int PhyReg) {
    unsigned int timeOut;

    LPC_EMAC->MADR = DP83848C_DEF_ADR | PhyReg;
    LPC_EMAC->MCMD = MCMD_READ;

    for(timeOut = 0; timeOut < MII_RD_TOUT; timeOut++) { /* Wait until operation completed */
        if((LPC_EMAC->MIND & MIND_BUSY) == 0) {
            LPC_EMAC->MCMD = 0;
            return LPC_EMAC->MRDD; /* Return a 16-bit value. */
        }
    }

    return -1;
}


//extern unsigned int SystemFrequency;
static unsigned int clockselect(void)
{
  if(SystemCoreClock < 10000000) {
    return 1;
  } else if(SystemCoreClock < 15000000) {
    return 2;
  } else if(SystemCoreClock < 20000000) {
    return 3;
  } else if(SystemCoreClock < 25000000) {
    return 4;
  } else if(SystemCoreClock < 35000000) {
    return 5;
  } else if(SystemCoreClock < 50000000) {
    return 6;
  } else if(SystemCoreClock < 70000000) {
    return 7;
  } else if(SystemCoreClock < 80000000) {
    return 8;
  } else if(SystemCoreClock < 90000000) {
    return 9;
  } else if(SystemCoreClock < 100000000) {
    return 10;
  } else if(SystemCoreClock < 120000000) {
    return 11;
  } else if(SystemCoreClock < 130000000) {
    return 12;
  } else if(SystemCoreClock < 140000000) {
    return 13;
  } else if(SystemCoreClock < 150000000) {
    return 15;
  } else if(SystemCoreClock < 160000000) {
    return 16;
  } else {
    return 0;
  }
}

static int ethernet_link(void)
{

    if (phy_id == DP83848C_ID) {
      return (phy_read(PHY_REG_STS) & PHY_STS_LINK);
    }
    else { // LAN8720_ID
      return (phy_read(PHY_REG_BMSR) & PHY_BMSR_LINK);
    }
}
//--------------------------------------------------------------------------------
// ISR
//--------------------------------------------------------------------------------


/**
 * EMACからのハンドラ
 */
static void emacIsrHandler(unsigned long i_status)
{
	if( i_status & INT_RX_DONE )
	{
		_event_handler(_event_param,NyLPC_TiEthernetDevice_EVENT_ON_RX);
	}
	if( i_status & INT_TX_DONE )
	{
		_event_handler(_event_param,NyLPC_TiEthernetDevice_EVENT_ON_TX);
	}
}

#endif



