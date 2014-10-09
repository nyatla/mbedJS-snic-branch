#include "NyLPC_config.h"
#if NyLPC_MCU==NyLPC_MCU_K64F
#include "NyLPC_stdlib.h"
#include "NyLPC_os.h"
#include "copy_of_ethernet_api.h"
#include "NyLPC_IEthernetDevice.h"
#include "NyLPC_cEthernetMM.h"
//////////
#include "fsl_enet_driver.h"
#include "fsl_enet_hal.h"
#include "fsl_device_registers.h"
#include "fsl_phy_driver.h"
#include "fsl_interrupt_manager.h"
#include "k64f_emac_config.h"
#include <string.h>
#include <stdlib.h>

/**
 */
static NyLPC_TBool start(const struct NyLPC_TEthAddr* i_eth_addr,NyLPC_TiEthernetDevice_onEvent i_handler,void* i_param);
static void stop(void);
static void* getRxEthFrame(unsigned short* o_len_of_data);
static void nextRxEthFrame(void);
static void* allocTxBuf(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size);
static void releaseTxBuf(void* i_buf);
static void sendTxEthFrame(void* i_buf,unsigned short i_size);

const static struct TiEthernetDevice _interface_KSZ8081RNACA=
{
	"KSZ8081RNACA",
	start,
	stop,
	getRxEthFrame,
	nextRxEthFrame,
	allocTxBuf,
	releaseTxBuf,
	sendTxEthFrame,
	NULL	//deleted API
};



struct TEtherDriver{
    int rx_idx;
    int tx_idx;
	uint8_t *tx_desc_start_addr; /**< TX descriptor start address */
	uint8_t *rx_desc_start_addr; /**< RX descriptor start address */
};

static struct TEtherDriver _driver;
static void* _event_param;
static NyLPC_TiEthernetDevice_onEvent _event_handler;

////////////////////////////////////////////////////////////////////////////////
// LANパケットバッファ
////////////////////////////////////////////////////////////////////////////////
#define NUM_OF_RX_BUF 4
#define SIZE_OF_ETH_PACKET (1536)   //16バイト単位であること
static void* RX_BUF_BASE;       	//[NUM_OF_RX_BUF][SIZE_OF_ETH_PACKET]
static unsigned char* RX_BUF;   	//[NUM_OF_RX_BUF][SIZE_OF_ETH_PACKET]
static void* TX_BUF_BASE;       	//
static unsigned char* TX_BUF;   	//sizeof(struct NyLPC_TcEthernetMM_TxMemoryBlock)

#define NUM_OF_RX_RING NUM_OF_RX_BUF
#define NUM_OF_TX_RING 4

////////////////////////////////////////////////////////////////////////////////
//private function
////////////////////////////////////////////////////////////////////////////////


static NyLPC_TBool low_level_init(const unsigned char* i_ethaddr,int i_addr_len);
static void setRxDesc(void* rx_buf, int idx);
static void updateRxDesc(int idx);
static void setTxDesc(int idx);
static void updateTxDesc(int idx, uint8_t *buffer, uint16_t length, bool isLast);
static void eth_arch_enable_interrupts(void);
static void eth_arch_disable_interrupts(void);
static NyLPC_TUInt32 waitForTxEthFrameEmpty(void);

////////////////////////////////////////////////////////////////////////////////
//LAN API
////////////////////////////////////////////////////////////////////////////////
NyLPC_TBool EthDev_K64F_getInterface(
	const struct TiEthernetDevice** o_dev)
{
	*o_dev=&_interface_KSZ8081RNACA;
    RX_BUF_BASE=(unsigned char*)malloc(SIZE_OF_ETH_PACKET*NUM_OF_RX_BUF+RX_BUF_ALIGNMENT);
    RX_BUF=(unsigned char*)ENET_ALIGN((NyLPC_TUInt32)RX_BUF_BASE,RX_BUF_ALIGNMENT);
	TX_BUF_BASE=malloc(sizeof(struct NyLPC_TcEthernetMM_TxMemoryBlock)+TX_BUF_ALIGNMENT);
	TX_BUF=(unsigned char*)ENET_ALIGN((NyLPC_TUInt32)TX_BUF_BASE,TX_BUF_ALIGNMENT);
	
	return NyLPC_TBool_TRUE;
}

static NyLPC_TBool start(const struct NyLPC_TEthAddr* i_eth_addr,NyLPC_TiEthernetDevice_onEvent i_handler,void* i_param)
{
    _driver.rx_idx=0;
    _driver.tx_idx=0;
    //ISRw割り込み設定
    _event_handler=i_handler;
    _event_param=i_param;
    
    if(!low_level_init((const NyLPC_TUInt8*)(i_eth_addr->addr),6)){
        return NyLPC_TBool_FALSE;
    }
    //TXメモリマネージャの準備(バッファのアライメントは16,パディングも16にしてね。謎バイトが２個いるから。)
	NyLPC_cEthernetMM_initialize(TX_BUF);

    //Ethernetの割込み開始設定
    NyLPC_cIsr_enterCritical();
    //Ethernetの初期化シーケンス。割込みONとか
    {
    	eth_arch_enable_interrupts();
    }
    NyLPC_cIsr_exitCritical();

    return NyLPC_TBool_TRUE;
}

static void stop(void)
{
    NyLPC_cIsr_enterCritical();
    {
    	eth_arch_disable_interrupts();
    }
    NyLPC_cIsr_exitCritical();
   return;
}


#define err_mask (kEnetRxBdTrunc | kEnetRxBdCrc | kEnetRxBdNoOctet | kEnetRxBdLengthViolation)

static void* getRxEthFrame(unsigned short* o_len_of_data)
{
    int idx=_driver.rx_idx;
    enet_bd_struct_t * bdPtr = (enet_bd_struct_t*)_driver.rx_desc_start_addr;
    if((bdPtr[idx].control & kEnetRxBdEmpty)!=0){
        //パケット未着
        return NULL;
    }
    if((bdPtr[idx].control & err_mask) != 0){
        //エラー:パケットバッファを再設定して返却
        setRxDesc(RX_BUF+(idx*SIZE_OF_ETH_PACKET),idx);        
        _driver.rx_idx=(idx+1)%NUM_OF_RX_BUF;
        return NULL;
    }
    *o_len_of_data =(unsigned short)enet_hal_get_bd_length(&(bdPtr[idx]))-2;
    return &RX_BUF[idx*SIZE_OF_ETH_PACKET+2];
}

static void nextRxEthFrame(void)
{
    int idx=_driver.rx_idx;
    enet_bd_struct_t * bdPtr = (enet_bd_struct_t*)_driver.rx_desc_start_addr;
    //現在のRXメモリが有効かを確認
    if((bdPtr[idx].control & kEnetRxBdEmpty)==0){
        //パケットバッファを復活させる
        updateRxDesc(idx);
        //キューのエントリを進行
        _driver.rx_idx=(idx+1)%NUM_OF_RX_BUF;
        return;
    }
}

static void* allocTxBuf(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size)
{
	return ((NyLPC_TUInt8*)NyLPC_cEthernetMM_alloc(i_hint,o_size))+2;
}
static void releaseTxBuf(void* i_buf)
{
	NyLPC_cEthernetMM_release((NyLPC_TUInt8*)i_buf-2);
}


static void sendTxEthFrame(void* i_buf,unsigned short i_size)
{
	int Index;
	struct NyLPC_TTxBufferHeader* bh=NyLPC_TTxBufferHeader_getBufferHeaderAddr(i_buf-2);
	//サイズ0なら送信の必要なし
	if(i_size == 0)
	{
		return;
	}
	//送信デスクリプタが使えるようになるのを待つよ！
	waitForTxEthFrameEmpty();

	//送信対象のメモリブロックを送信中に設定。
	if(bh->is_lock){
		//送信中のメモリブロックなら無視
		return;
	}

	//送信中にセット
	bh->is_lock=NyLPC_TUInt8_TRUE;

	//最大送信サイズの制限
	if (i_size > ETH_FRAG_SIZE){
		i_size = ETH_FRAG_SIZE;
	}
	Index=_driver.tx_idx;

	updateTxDesc(Index,(uint8_t*)((unsigned char*)i_buf-2),i_size+2,Index==(NUM_OF_TX_RING-1));
	_driver.tx_idx=(_driver.tx_idx+1)%NUM_OF_TX_RING;
	return;
}

/**
 * 送信キューが空くまで待ちます。
 * @return
 * 次に書き込むことが出来る送信キューのIDだけど使わないで
 */
static NyLPC_TUInt32 waitForTxEthFrameEmpty(void)
{
	int idx;
	int i;
	struct NyLPC_TTxBufferHeader *b;
	volatile void* p;
	volatile enet_bd_struct_t * bdPtr = (enet_bd_struct_t*)_driver.tx_desc_start_addr;


	//送信キューフルが解除されるまで待ち.現在のTQDescがReady状態の間ループ
	
	while((bdPtr[_driver.tx_idx].control & kEnetTxBdReady)!=0)
	{
		NyLPC_cThread_sleep(10);
	}
	//インデクスを起点にReady状態が0のデスクリプタを全て解放(無駄があるけど無視)
	for(i=0;i<NUM_OF_TX_RING;i++){
		idx=(_driver.tx_idx+i)%NUM_OF_TX_RING;
		if((bdPtr[idx].control & kEnetTxBdReady)!=0){
			//if Ready state then break!
			break;
		}
		bdPtr[idx].control=(bdPtr[idx].control)&(~kEnetTxBdReady);
		//バッファを参照してたらそいつのロックビットも解放
		p=(void*)NTOHL(bdPtr[idx].buffer);
		if(p!=NULL){
			b=((struct NyLPC_TTxBufferHeader*)p)-1;
			b->is_lock=NyLPC_TUInt8_FALSE;
			bdPtr[idx].buffer=0;
		}
	}
	return _driver.tx_idx;
}

////////////////////////////////////////////////////////////////////////////////
// Ethernet interdface functions
////////////////////////////////////////////////////////////////////////////////


// K64F-specific macros
#define RX_PBUF_AUTO_INDEX    (-1)
extern void k64f_init_eth_hardware(void);


//static struct k64f_enetdata k64f_enetdata;

static enet_dev_if_t enetDevIf[HW_ENET_INSTANCE_COUNT];
static enet_mac_config_t g_enetMacCfg[HW_ENET_INSTANCE_COUNT] = 
{
  {
    ENET_ETH_MAX_FLEN ,  /*!< enet receive buffer size*/
    ENET_RX_LARGE_BUFFER_NUM, /*!< enet large receive buffer number*/
    NUM_OF_RX_RING,        /*!< enet receive bd number*/
    NUM_OF_TX_RING,        /*!< enet transmit bd number*/
    {0},                /*!< enet mac address*/
    kEnetCfgRmii,       /*!< enet rmii interface*/
    kEnetCfgSpeed100M,  /*!< enet rmii 100M*/
    kEnetCfgFullDuplex, /*!< enet rmii Full- duplex*/
     /*!< enet mac control flag recommended to use enet_mac_control_flag_t
      we send frame with crc so receive crc forward for data length check test*/
    kEnetRxCrcFwdEnable | kEnetRxFlowControlEnable,
    true,         /*!< enet txaccelerator enabled*/
    true,        /*!< enet rxaccelerator enabled*/
    false,        /*!< enet store and forward*/
    {false, false, true, false, true},  /*!< enet rxaccelerator config*/
    {false, false, true},          /*!< enet txaccelerator config*/
    true,               /*!< vlan frame support*/
    true,               /*!< phy auto discover*/
    ENET_MII_CLOCK,     /*!< enet MDC clock*/
  },
};
static enet_phy_config_t g_enetPhyCfg[HW_ENET_INSTANCE_COUNT] =
{
	{0, false}
};

static NyLPC_TBool k64f_rx_setup(enet_rxbd_config_t *rxbdCfg);
static NyLPC_TBool k64f_tx_setup(enet_txbd_config_t *txbdCfg);


/**
 * i_idx番目のデスクリプタにバッファをセット
 */
static void setRxDesc(void* rx_buf, int idx)
{
    enet_bd_struct_t *start = (enet_bd_struct_t *)_driver.rx_desc_start_addr;
    /* Setup descriptor and clear statuses */
    enet_hal_init_rxbds(start + idx, (uint8_t*)rx_buf,idx ==(NUM_OF_RX_RING - 1));
    enet_hal_active_rxbd(BOARD_DEBUG_ENET_INSTANCE);
}
static void updateRxDesc(int idx)
{
    enet_bd_struct_t *start = (enet_bd_struct_t *)_driver.rx_desc_start_addr;
    /* Setup descriptor and clear statuses */
    enet_hal_update_rxbds(start + idx,NULL,false);
    enet_hal_active_rxbd(BOARD_DEBUG_ENET_INSTANCE);
}
static void setTxDesc(int idx)
{
    enet_bd_struct_t *start = (enet_bd_struct_t *)_driver.tx_desc_start_addr;
    /* Setup descriptor and clear statuses */
    enet_hal_init_txbds(start + idx,idx ==(NUM_OF_RX_RING - 1));
    enet_hal_active_txbd(BOARD_DEBUG_ENET_INSTANCE);
}

static void updateTxDesc(int idx, uint8_t *buffer, uint16_t length, bool isLast)
{
	volatile enet_bd_struct_t * bdPtr = (enet_bd_struct_t *)(_driver.tx_desc_start_addr + idx * enet_hal_get_bd_size());
	
	bdPtr->length = HTONS(length); /* Set data length*/
	bdPtr->buffer = (uint8_t *)HTONL((uint32_t)buffer); /* Set data buffer*/
	bdPtr->control |= kEnetTxBdLast;//最終フラグメントのフラグね
	bdPtr->controlExtend1 |= kEnetTxBdTxInterrupt;
	bdPtr->controlExtend2 &= ~TX_DESC_UPDATED_MASK; // descriptor not updated by DMA
	bdPtr->control |= kEnetTxBdTransmitCrc | kEnetTxBdReady;
	if(isLast){
		//これはデスクリプタの終了位置のフラグ
		bdPtr->control |=kEnetTxBdWrap;
	}
	enet_hal_active_txbd(BOARD_DEBUG_ENET_INSTANCE);	
}

static void* ENET_MAC_CONTEXT_BUF=NULL;

/** \brief  Low level init of the MAC and PHY.
 *
 *  \param[in]      netif  Pointer to LWIP netif structure
 */
NyLPC_TBool low_level_init(const unsigned char* i_ethaddr,int i_addr_len)
{
    enet_dev_if_t * enetIfPtr;
    uint32_t device = BOARD_DEBUG_ENET_INSTANCE;
    enet_rxbd_config_t rxbdCfg;
    enet_txbd_config_t txbdCfg;
    enet_phy_speed_t phy_speed;
    enet_phy_duplex_t phy_duplex;
    
    //RX/TXメモリはデバイス選択時に確保
    k64f_init_eth_hardware();
  
    /* Initialize device*/
    enetIfPtr = (enet_dev_if_t *)&enetDevIf[device];
    enetIfPtr->deviceNumber = device;
    enetIfPtr->macCfgPtr = &g_enetMacCfg[device];
    enetIfPtr->phyCfgPtr = &g_enetPhyCfg[device];
    enetIfPtr->macApiPtr = &g_enetMacApi;
    enetIfPtr->phyApiPtr = (void *)&g_enetPhyApi;
    //macアドレスのコピー
    memcpy(enetIfPtr->macCfgPtr->macAddr,(char*)i_ethaddr,i_addr_len);
	//enetIfPtr->macContextPtrはgetInterface
	if(ENET_MAC_CONTEXT_BUF!=NULL){
		free(ENET_MAC_CONTEXT_BUF);
		ENET_MAC_CONTEXT_BUF=NULL;
	}
	ENET_MAC_CONTEXT_BUF=calloc(1, sizeof(enet_mac_context_t));
	if(ENET_MAC_CONTEXT_BUF==NULL){
		return NyLPC_TBool_FALSE;//ERR_BUF;
	}
	enetIfPtr->macContextPtr = (enet_mac_context_t *)ENET_MAC_CONTEXT_BUF;

	/* Initialize enet buffers*/
	if(!k64f_rx_setup(&rxbdCfg)) {
		return NyLPC_TBool_FALSE;//ERR_BUF;
	}
	/* Initialize enet buffers*/
	if(!k64f_tx_setup(&txbdCfg)) {
		return NyLPC_TBool_FALSE;//ERR_BUF;
	}
	/* Initialize enet module*/
	if (enet_mac_init(enetIfPtr, &rxbdCfg, &txbdCfg) == kStatus_ENET_Success)
	{
		/* Initialize PHY*/
		if (enetIfPtr->macCfgPtr->isPhyAutoDiscover) {
			if (((enet_phy_api_t *)(enetIfPtr->phyApiPtr))->phy_auto_discover(enetIfPtr) != kStatus_PHY_Success)
				return NyLPC_TBool_FALSE;//ERR_IF;
		}
		if (((enet_phy_api_t *)(enetIfPtr->phyApiPtr))->phy_init(enetIfPtr) != kStatus_PHY_Success)
			return NyLPC_TBool_FALSE;//ERR_IF;		
		enetIfPtr->isInitialized = true;
	}else{
		// TODOETH: cleanup memory
		return NyLPC_TBool_FALSE;//ERR_IF;
	}
	
	/* Get link information from PHY */
	phy_get_link_speed(enetIfPtr, &phy_speed);
	phy_get_link_duplex(enetIfPtr, &phy_duplex);
	BW_ENET_RCR_RMII_10T(enetIfPtr->deviceNumber, phy_speed == kEnetSpeed10M ? kEnetCfgSpeed10M : kEnetCfgSpeed100M);
	BW_ENET_TCR_FDEN(enetIfPtr->deviceNumber, phy_duplex == kEnetFullDuplex ? kEnetCfgFullDuplex : kEnetCfgHalfDuplex);
	
	/* Enable Ethernet module*/
	enet_hal_config_ethernet(device, true, true);
	
	/* Active Receive buffer descriptor must be done after module enable*/
	enet_hal_active_rxbd(enetIfPtr->deviceNumber);
	enet_hal_active_txbd(enetIfPtr->deviceNumber);
	
	return NyLPC_TBool_TRUE;//ERR_OK;
}


static void* RX_DESC_BUF_BASE=NULL;


/** \brief  Sets up the RX descriptor ring buffers.
 *
 *  This function sets up the descriptor list used for receive packets.
 *
 *  \param[in]  netif  Pointer to driver data structure
 *  \returns    true/false
 */
static NyLPC_TBool k64f_rx_setup(enet_rxbd_config_t *rxbdCfg)
{   
//    struct k64f_enetdata *k64f_enet = &(netif->state);
    enet_dev_if_t *enetIfPtr = (enet_dev_if_t *)&enetDevIf[BOARD_DEBUG_ENET_INSTANCE];
    uint32_t rxBufferSizeAligned;
    int i;

    // Allocate RX descriptors
    if(RX_DESC_BUF_BASE!=NULL){
    	free(RX_DESC_BUF_BASE);
    	RX_DESC_BUF_BASE=NULL;
    }
    RX_DESC_BUF_BASE = (void*)calloc(1, enet_hal_get_bd_size() * enetIfPtr->macCfgPtr->rxBdNumber + ENET_BD_ALIGNMENT);
    if(RX_DESC_BUF_BASE==NULL){
        return NyLPC_TBool_FALSE;
    }
    //16byteアライメントに修正
    _driver.rx_desc_start_addr = (uint8_t *)ENET_ALIGN((NyLPC_TUInt32)RX_DESC_BUF_BASE, ENET_BD_ALIGNMENT);
    rxBufferSizeAligned = ENET_ALIGN(enetIfPtr->macCfgPtr->rxBufferSize, ENET_RX_BUFFER_ALIGNMENT);
    enetIfPtr->macContextPtr->rxBufferSizeAligned = rxBufferSizeAligned;
    rxbdCfg->rxBdPtrAlign = _driver.rx_desc_start_addr;
    rxbdCfg->rxBdNum = enetIfPtr->macCfgPtr->rxBdNumber;
    rxbdCfg->rxBufferNum = enetIfPtr->macCfgPtr->rxBdNumber;
    
    //初期化
    enet_hal_active_rxbd(BOARD_DEBUG_ENET_INSTANCE);
    for(i=0;i<NUM_OF_RX_RING;i++){
        setRxDesc(RX_BUF+(i*SIZE_OF_ETH_PACKET),i);
    }
    //  k64f_rx_queue(netif, RX_PBUF_AUTO_INDEX);
    return NyLPC_TBool_TRUE;
}




static void* TX_DESC_BUF_BASE=NULL;
/** \brief  Sets up the TX descriptor ring buffers.
 *
 *  This function sets up the descriptor list used for transmit packets.
 *
 *  \param[in]      netif  Pointer to driver data structure
 *  \returns        true/false
 */
static NyLPC_TBool k64f_tx_setup(enet_txbd_config_t *txbdCfg)
{
	int i;

	enet_dev_if_t *enetIfPtr = (enet_dev_if_t *)&enetDevIf[BOARD_DEBUG_ENET_INSTANCE];
	
	// Allocate TX descriptors
    if(TX_DESC_BUF_BASE!=NULL){
    	free(TX_DESC_BUF_BASE);
    	TX_DESC_BUF_BASE=NULL;
    } 	
	TX_DESC_BUF_BASE = (void*)calloc(1, enet_hal_get_bd_size() * enetIfPtr->macCfgPtr->txBdNumber + ENET_BD_ALIGNMENT);
	if(TX_DESC_BUF_BASE==NULL){
		return NyLPC_TBool_FALSE;
	}
	
	_driver.tx_desc_start_addr = (uint8_t *)ENET_ALIGN((uint32_t)TX_DESC_BUF_BASE, ENET_BD_ALIGNMENT);
	
	txbdCfg->txBdPtrAlign = _driver.tx_desc_start_addr;
	txbdCfg->txBufferNum = enetIfPtr->macCfgPtr->txBdNumber;
	txbdCfg->txBufferSizeAlign = ENET_ALIGN(enetIfPtr->maxFrameSize, ENET_TX_BUFFER_ALIGNMENT);
	
	// Make the TX descriptor ring circular
	for(i=0;i<NUM_OF_TX_RING;i++){
		setTxDesc(i);
	}
	return NyLPC_TBool_TRUE;
}


//--------------------------------------------------------------------------------
// ISR
//--------------------------------------------------------------------------------
extern IRQn_Type enet_irq_ids[HW_ENET_INSTANCE_COUNT][FSL_FEATURE_ENET_INTERRUPT_COUNT];
extern uint8_t enetIntMap[kEnetIntNum];
extern void *enetIfHandle;

static void eth_arch_enable_interrupts(void)
{
	enet_hal_config_interrupt(BOARD_DEBUG_ENET_INSTANCE, (kEnetTxFrameInterrupt | kEnetRxFrameInterrupt), true);  
	interrupt_enable(enet_irq_ids[BOARD_DEBUG_ENET_INSTANCE][enetIntMap[kEnetRxfInt]]);
	interrupt_enable(enet_irq_ids[BOARD_DEBUG_ENET_INSTANCE][enetIntMap[kEnetTxfInt]]); 
}

static void eth_arch_disable_interrupts(void)
{
	interrupt_disable(enet_irq_ids[BOARD_DEBUG_ENET_INSTANCE][enetIntMap[kEnetRxfInt]]);
	interrupt_disable(enet_irq_ids[BOARD_DEBUG_ENET_INSTANCE][enetIntMap[kEnetTxfInt]]);  
}
void ENET_Transmit_IRQHandler(void)
{
//	led(0,-1);
	enet_hal_clear_interrupt(((enet_dev_if_t *)enetIfHandle)->deviceNumber, kEnetTxFrameInterrupt);
	_event_handler(_event_param,NyLPC_TiEthernetDevice_EVENT_ON_TX);
}

void ENET_Receive_IRQHandler(void)
{
	enet_hal_clear_interrupt(((enet_dev_if_t *)enetIfHandle)->deviceNumber, kEnetRxFrameInterrupt);
	_event_handler(_event_param,NyLPC_TiEthernetDevice_EVENT_ON_RX);
}


#endif



